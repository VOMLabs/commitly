package codegen

import (
	"encoding/binary"
	"strings"

	"github.com/vomlabs/commitly/compiler/parser"
)

// Opcode constants matching vm/mod.rs in the Rust runtime.
const (
	OpHookEnter     byte = 0x01
	OpWorkflowEnter byte = 0x02
	OpCheckRegex    byte = 0x10
	OpCheckEmpty    byte = 0x11
	OpExecCmd       byte = 0x20
	OpWorkflowStep  byte = 0x21
	OpReject        byte = 0x30
	OpHalt          byte = 0xFF
)

var magic = []byte{'C', 'L', 'Y', 'C', 0x01}

// Emitter compiles an AST to .clyc bytecode.
type Emitter struct {
	code   []byte
	pool   []string
	poolIdx map[string]uint16
}

// NewEmitter creates a ready Emitter.
func NewEmitter() *Emitter {
	return &Emitter{poolIdx: map[string]uint16{}}
}

// Emit compiles the program and returns the complete .clyc blob.
func (e *Emitter) Emit(prog *parser.Program) []byte {
	e.code = append(e.code, magic...)

	for _, h := range prog.Hooks {
		e.emitU8(OpHookEnter)
		e.emitU16(e.intern(h.Name))
		for _, stmt := range h.Body {
			e.emitStmt(stmt)
		}
	}

	for _, w := range prog.Workflows {
		e.emitU8(OpWorkflowEnter)
		e.emitU16(e.intern(w.Name))
		for _, step := range w.Steps {
			e.emitU8(OpWorkflowStep)
			if step.Inner != nil {
				e.emitStmt(step.Inner)
			}
		}
	}

	e.emitU8(OpHalt)

	// Append string pool then its offset as 4-byte LE uint32.
	poolStart := uint32(len(e.code))
	for _, s := range e.pool {
		raw := []byte(s)
		e.emitU16(uint16(len(raw)))
		e.code = append(e.code, raw...)
	}
	var buf [4]byte
	binary.LittleEndian.PutUint32(buf[:], poolStart)
	e.code = append(e.code, buf[:]...)

	return e.code
}

func (e *Emitter) emitStmt(stmt parser.Stmt) {
	switch s := stmt.(type) {
	case *parser.RequireStmt:
		e.emitPredicate(s.Predicate)
	case *parser.RejectStmt:
		e.emitU8(OpReject)
		e.emitU16(e.intern("reject: " + strings.Join(s.Predicate.Field, ".")))
	case *parser.RunStmt:
		e.emitU8(OpExecCmd)
		e.emitU16(e.intern(s.Command))
	case *parser.BranchEnsureStmt:
		e.emitU8(OpCheckRegex)
		e.emitU16(e.intern("branch.name"))
		e.emitU16(e.intern(s.Pattern))
	case *parser.TagCreateStmt:
		e.emitU8(OpExecCmd)
		e.emitU16(e.intern("tag.create prefix=" + s.Prefix))
	}
}

func (e *Emitter) emitPredicate(pred *parser.Predicate) {
	switch pred.Kind {
	case parser.PredMatchesRegex:
		e.emitU8(OpCheckRegex)
		e.emitU16(e.intern(strings.Join(pred.Field, ".")))
		e.emitU16(e.intern(pred.Pattern))
	case parser.PredIsEmpty:
		e.emitU8(OpCheckEmpty)
		e.emitU16(e.intern(strings.Join(pred.Field, ".")))
	}
}

func (e *Emitter) emitU8(b byte) {
	e.code = append(e.code, b)
}

func (e *Emitter) emitU16(v uint16) {
	e.code = append(e.code, byte(v), byte(v>>8))
}

// intern adds s to the string pool if not already present and returns its index.
func (e *Emitter) intern(s string) uint16 {
	if idx, ok := e.poolIdx[s]; ok {
		return idx
	}
	idx := uint16(len(e.pool))
	e.pool = append(e.pool, s)
	e.poolIdx[s] = idx
	return idx
}

// Disassemble returns a human-readable dump of the bytecode (for --emit-ast / debug).
func Disassemble(code []byte) string {
	if len(code) < 5 {
		return "<too short>"
	}
	if string(code[:4]) != "CLYC" {
		return "<missing magic>"
	}

	poolOffset := binary.LittleEndian.Uint32(code[len(code)-4:])
	pool := parsePool(code[poolOffset : len(code)-4])
	body := code[5:poolOffset]

	var sb strings.Builder
	ip := 0
	for ip < len(body) {
		op := body[ip]
		ip++
		switch op {
		case OpHookEnter:
			idx := readU16(body, ip); ip += 2
			sb.WriteString("HOOK_ENTER    " + poolStr(pool, idx) + "\n")
		case OpWorkflowEnter:
			idx := readU16(body, ip); ip += 2
			sb.WriteString("WORKFLOW_ENTER " + poolStr(pool, idx) + "\n")
		case OpCheckRegex:
			f := readU16(body, ip); ip += 2
			pat := readU16(body, ip); ip += 2
			sb.WriteString("CHECK_REGEX   field=" + poolStr(pool, f) + " pattern=/" + poolStr(pool, pat) + "/\n")
		case OpCheckEmpty:
			f := readU16(body, ip); ip += 2
			sb.WriteString("CHECK_EMPTY   field=" + poolStr(pool, f) + "\n")
		case OpExecCmd:
			idx := readU16(body, ip); ip += 2
			sb.WriteString("EXEC_CMD      " + poolStr(pool, idx) + "\n")
		case OpWorkflowStep:
			sb.WriteString("WORKFLOW_STEP\n")
		case OpReject:
			idx := readU16(body, ip); ip += 2
			sb.WriteString("REJECT        " + poolStr(pool, idx) + "\n")
		case OpHalt:
			sb.WriteString("HALT\n")
			return sb.String()
		default:
			sb.WriteString("UNKNOWN 0x" + string(rune('0'+op>>4)) + string(rune('0'+op&0xf)) + "\n")
		}
	}
	return sb.String()
}

func parsePool(raw []byte) []string {
	var pool []string
	i := 0
	for i+1 < len(raw) {
		l := int(binary.LittleEndian.Uint16(raw[i : i+2]))
		i += 2
		if i+l > len(raw) { break }
		pool = append(pool, string(raw[i:i+l]))
		i += l
	}
	return pool
}

func readU16(data []byte, offset int) uint16 {
	if offset+1 >= len(data) { return 0 }
	return binary.LittleEndian.Uint16(data[offset : offset+2])
}

func poolStr(pool []string, idx uint16) string {
	if int(idx) >= len(pool) { return "<invalid>" }
	return pool[idx]
}
