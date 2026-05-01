package vm

import (
	"encoding/binary"
	"fmt"
	"os/exec"
	"regexp"
	"strings"
)

// Opcode values must match codegen/emitter.go and Rust vm/mod.rs.
const (
	opHookEnter     byte = 0x01
	opWorkflowEnter byte = 0x02
	opCheckRegex    byte = 0x10
	opCheckEmpty    byte = 0x11
	opExecCmd       byte = 0x20
	opWorkflowStep  byte = 0x21
	opReject        byte = 0x30
	opHalt          byte = 0xFF
)

var magic = []byte{'C', 'L', 'Y', 'C', 0x01}

// Context is the runtime state passed when executing a hook.
type Context struct {
	HookName      string
	StagedFiles   []string
	CommitSummary string
}

// Result of running bytecode.
type Result struct {
	OK      bool
	Message string // rejection or error message when !OK
}

// Run executes .clyc bytecode in the given context.
func Run(bytecode []byte, ctx *Context) Result {
	if !strings.HasPrefix(string(bytecode[:min(5, len(bytecode))]), string(magic)) {
		return Result{Message: "invalid magic header"}
	}
	if len(bytecode) < 9 {
		return Result{Message: "bytecode too short"}
	}

	poolOffset := binary.LittleEndian.Uint32(bytecode[len(bytecode)-4:])
	if int(poolOffset) >= len(bytecode) {
		return Result{Message: "invalid pool offset"}
	}
	pool := parsePool(bytecode[poolOffset : len(bytecode)-4])
	body := bytecode[5:poolOffset]

	ip := 0
	for ip < len(body) {
		op := body[ip]
		ip++
		switch op {
		case opHalt:
			return Result{OK: true}

		case opHookEnter, opWorkflowEnter:
			ip += 2

		case opCheckRegex:
			_ = readU16(body, ip); ip += 2
			patIdx := readU16(body, ip); ip += 2
			pattern := poolGet(pool, patIdx)
			if matched, err := regexp.MatchString(pattern, ctx.CommitSummary); err != nil {
				return Result{Message: fmt.Sprintf("invalid regex /%s/: %v", pattern, err)}
			} else if !matched {
				return Result{Message: fmt.Sprintf("commit.summary does not match /%s/", pattern)}
			}

		case opCheckEmpty:
			ip += 2
			if len(ctx.StagedFiles) == 0 {
				return Result{Message: "staged.files is empty"}
			}

		case opReject:
			msgIdx := readU16(body, ip); ip += 2
			return Result{Message: poolGet(pool, msgIdx)}

		case opExecCmd:
			cmdIdx := readU16(body, ip); ip += 2
			cmd := poolGet(pool, cmdIdx)
			var c *exec.Cmd
			if isWindows() {
				c = exec.Command("cmd", "/C", cmd)
			} else {
				c = exec.Command("sh", "-c", cmd)
			}
			if out, err := c.CombinedOutput(); err != nil {
				return Result{Message: fmt.Sprintf("command %q failed: %s", cmd, strings.TrimSpace(string(out)))}
			}

		case opWorkflowStep:
			ip += 2
		default:
			return Result{Message: fmt.Sprintf("unknown opcode 0x%02X at ip=%d", op, ip-1)}
		}
	}

	return Result{OK: true}
}

func parsePool(raw []byte) []string {
	var pool []string
	i := 0
	for i+1 < len(raw) {
		l := int(binary.LittleEndian.Uint16(raw[i : i+2]))
		i += 2
		if i+l > len(raw) {
			break
		}
		pool = append(pool, string(raw[i:i+l]))
		i += l
	}
	return pool
}

func readU16(data []byte, offset int) uint16 {
	if offset+1 >= len(data) {
		return 0
	}
	return binary.LittleEndian.Uint16(data[offset : offset+2])
}

func poolGet(pool []string, idx uint16) string {
	if int(idx) >= len(pool) {
		return ""
	}
	return pool[idx]
}

func min(a, b int) int {
	if a < b {
		return a
	}
	return b
}

func isWindows() bool {
	// runtime.GOOS approach to avoid importing "runtime" here.
	return strings.Contains(strings.ToLower(
		func() string {
			cmd := exec.Command("go", "env", "GOOS")
			out, _ := cmd.Output()
			return strings.TrimSpace(string(out))
		}()), "windows")
}
