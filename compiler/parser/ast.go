package parser

import "github.com/vomlabs/commitly/compiler/lexer"

// ── AST Node types ────────────────────────────────────────────────────────

type Node interface{ nodeKind() string }

// Program is the root node.
type Program struct {
	Hooks     []*HookDecl
	Workflows []*WorkflowDecl
}

func (*Program) nodeKind() string { return "Program" }

// HookDecl: hook <name> { <body> }
type HookDecl struct {
	Name  string // e.g. "pre-commit"
	Line  int
	Body  []Stmt
}

func (*HookDecl) nodeKind() string { return "HookDecl" }

// WorkflowDecl: workflow <name> { <steps> }
type WorkflowDecl struct {
	Name  string
	Line  int
	Steps []*StepStmt
}

func (*WorkflowDecl) nodeKind() string { return "WorkflowDecl" }

// Stmt interface
type Stmt interface {
	Node
	stmtNode()
}

// StepStmt: step { <inner> }
type StepStmt struct {
	Line  int
	Inner Stmt
}

func (*StepStmt) nodeKind() string { return "StepStmt" }
func (*StepStmt) stmtNode()        {}

// RunStmt: run "<cmd>"
type RunStmt struct {
	Line    int
	Command string
}

func (*RunStmt) nodeKind() string { return "RunStmt" }
func (*RunStmt) stmtNode()        {}

// RequireStmt: require <predicate>
type RequireStmt struct {
	Line      int
	Predicate *Predicate
}

func (*RequireStmt) nodeKind() string { return "RequireStmt" }
func (*RequireStmt) stmtNode()        {}

// RejectStmt: reject if <predicate>
type RejectStmt struct {
	Line      int
	Predicate *Predicate
}

func (*RejectStmt) nodeKind() string { return "RejectStmt" }
func (*RejectStmt) stmtNode()        {}

// BranchEnsureStmt: branch.ensure name = "<pattern>"
type BranchEnsureStmt struct {
	Line    int
	Pattern string
}

func (*BranchEnsureStmt) nodeKind() string { return "BranchEnsureStmt" }
func (*BranchEnsureStmt) stmtNode()        {}

// TagCreateStmt: tag.create prefix = "<p>" auto-increment = true
type TagCreateStmt struct {
	Line          int
	Prefix        string
	AutoIncrement bool
}

func (*TagCreateStmt) nodeKind() string { return "TagCreateStmt" }
func (*TagCreateStmt) stmtNode()        {}

// ── Predicate ─────────────────────────────────────────────────────────────

// PredicateKind describes what the predicate tests.
type PredicateKind int

const (
	PredMatchesRegex PredicateKind = iota // <field> matches /<pattern>/
	PredIsEmpty                           // <field> is [not] empty
)

type Predicate struct {
	Kind    PredicateKind
	Field   []string // e.g. ["commit","summary"] or ["staged","files"]
	Pattern string   // for PredMatchesRegex
	Negate  bool     // for PredIsEmpty: "is not empty"
	Tok     lexer.Token
}
