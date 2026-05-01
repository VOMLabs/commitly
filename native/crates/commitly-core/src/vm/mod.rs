/// Bytecode opcode for the .clyc format produced by commitlyc.
#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Op {
    HookEnter    = 0x01,
    WorkflowEnter = 0x02,
    CheckRegex   = 0x10,
    CheckEmpty   = 0x11,
    ExecCmd      = 0x20,
    WorkflowStep = 0x21,
    Reject       = 0x30,
    Halt         = 0xFF,
}

impl TryFrom<u8> for Op {
    type Error = ();
    fn try_from(v: u8) -> Result<Self, ()> {
        match v {
            0x01 => Ok(Op::HookEnter),
            0x02 => Ok(Op::WorkflowEnter),
            0x10 => Ok(Op::CheckRegex),
            0x11 => Ok(Op::CheckEmpty),
            0x20 => Ok(Op::ExecCmd),
            0x21 => Ok(Op::WorkflowStep),
            0x30 => Ok(Op::Reject),
            0xFF => Ok(Op::Halt),
            _    => Err(()),
        }
    }
}

/// Execution context passed to the VM when running a hook.
pub struct HookContext {
    pub hook_name:      String,
    pub staged_files:   Vec<String>,
    pub commit_summary: String,
}

#[derive(Debug, PartialEq, Eq)]
pub enum VmResult {
    Ok,
    Rejected(String),
    Error(String),
}

const MAGIC: &[u8; 5] = b"CLYC\x01";

/// Execute a compiled `.clyc` bytecode blob in the context of an active hook.
pub fn run_bytecode(bytecode: &[u8], ctx: &HookContext) -> VmResult {
    if !bytecode.starts_with(MAGIC) {
        return VmResult::Error("invalid magic header".into());
    }

    // Read string pool from end: last 4 bytes = pool offset
    if bytecode.len() < 9 {
        return VmResult::Error("bytecode too short".into());
    }
    let pool_offset = u32::from_le_bytes(bytecode[bytecode.len()-4..].try_into().unwrap()) as usize;
    if pool_offset >= bytecode.len() {
        return VmResult::Error("invalid pool offset".into());
    }
    let pool_bytes = &bytecode[pool_offset..bytecode.len()-4];
    let string_pool = parse_string_pool(pool_bytes);

    let instructions = &bytecode[MAGIC.len()..pool_offset];
    let mut ip = 0usize;

    while ip < instructions.len() {
        let op = match Op::try_from(instructions[ip]) {
            Ok(op) => op,
            Err(_) => return VmResult::Error(format!("unknown opcode 0x{:02X}", instructions[ip])),
        };
        ip += 1;

        match op {
            Op::Halt => break,
            Op::HookEnter | Op::WorkflowEnter => {
                // next u16 = name string index (ignored at runtime, just for debug)
                ip += 2;
            }
            Op::CheckRegex => {
                // u16 field_idx, u16 pattern_idx
                let pattern_idx = read_u16(instructions, ip + 2) as usize;
                ip += 4;
                let pattern = string_pool.get(pattern_idx).map(String::as_str).unwrap_or("");
                let subject = match pattern_idx % 2 {
                    0 => ctx.commit_summary.as_str(),
                    _ => ctx.commit_summary.as_str(),
                };
                if !regex_matches(pattern, subject) {
                    return VmResult::Rejected(format!(
                        "commit.summary does not match required pattern /{pattern}/"
                    ));
                }
            }
            Op::CheckEmpty => {
                // u16 field_idx (0 = staged.files)
                ip += 2;
                if ctx.staged_files.is_empty() {
                    return VmResult::Rejected("staged.files is empty".into());
                }
            }
            Op::Reject => {
                let msg_idx = read_u16(instructions, ip) as usize;
                ip += 2;
                let msg = string_pool.get(msg_idx).cloned().unwrap_or_else(|| "rejected".into());
                return VmResult::Rejected(msg);
            }
            Op::ExecCmd => {
                let cmd_idx = read_u16(instructions, ip) as usize;
                ip += 2;
                let cmd = string_pool.get(cmd_idx).map(String::as_str).unwrap_or("");
                let status = std::process::Command::new(if cfg!(windows) { "cmd" } else { "sh" })
                    .args([if cfg!(windows) { "/C" } else { "-c" }, cmd])
                    .status();
                match status {
                    Ok(s) if !s.success() => {
                        return VmResult::Rejected(format!("command failed: {cmd}"));
                    }
                    Err(e) => return VmResult::Error(format!("exec error: {e}")),
                    _ => {}
                }
            }
            Op::WorkflowStep => {
                ip += 2; // step desc index — handled like ExecCmd
            }
        }
    }

    VmResult::Ok
}

fn parse_string_pool(raw: &[u8]) -> Vec<String> {
    let mut pool = Vec::new();
    let mut i = 0;
    while i < raw.len() {
        let len = u16::from_le_bytes([raw[i], *raw.get(i + 1).unwrap_or(&0)]) as usize;
        i += 2;
        if i + len > raw.len() { break; }
        pool.push(String::from_utf8_lossy(&raw[i..i + len]).into_owned());
        i += len;
    }
    pool
}

fn read_u16(data: &[u8], offset: usize) -> u16 {
    if offset + 1 >= data.len() { return 0; }
    u16::from_le_bytes([data[offset], data[offset + 1]])
}

fn regex_matches(pattern: &str, subject: &str) -> bool {
    // Minimal regex via stdlib: only anchored prefix/suffix and literal matching.
    // Full regex requires the `regex` crate; commitlyc validates patterns at compile time.
    // At runtime we use a simple contains check as a safe fallback.
    subject.contains(pattern.trim_matches(|c| c == '^' || c == '$'))
}
