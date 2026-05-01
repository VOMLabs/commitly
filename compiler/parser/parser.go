package parser

import (
	"fmt"

	"github.com/vomlabs/commitly/compiler/lexer"
)

// Parser builds an AST from a token slice.
type Parser struct {
	tokens []lexer.Token
	pos    int
}

// New creates a Parser from a token list (produced by lexer.Tokenise).
func New(tokens []lexer.Token) *Parser {
	return &Parser{tokens: tokens}
}

// Parse returns the root Program or the first error encountered.
func (p *Parser) Parse() (*Program, error) {
	prog := &Program{}
	for !p.atEnd() {
		p.skipNewlines()
		if p.atEnd() {
			break
		}
		tok := p.peek()
		switch tok.Kind {
		case lexer.HOOK:
			h, err := p.parseHook()
			if err != nil {
				return nil, err
			}
			prog.Hooks = append(prog.Hooks, h)
		case lexer.WORKFLOW:
			w, err := p.parseWorkflow()
			if err != nil {
				return nil, err
			}
			prog.Workflows = append(prog.Workflows, w)
		default:
			return nil, fmt.Errorf("line %d:%d: unexpected token %q", tok.Line, tok.Col, tok.Literal)
		}
	}
	return prog, nil
}

// ── Helpers ───────────────────────────────────────────────────────────────

func (p *Parser) peek() lexer.Token { return p.tokens[p.pos] }
func (p *Parser) atEnd() bool       { return p.pos >= len(p.tokens) || p.tokens[p.pos].Kind == lexer.EOF }

func (p *Parser) advance() lexer.Token {
	t := p.tokens[p.pos]
	p.pos++
	return t
}

func (p *Parser) expect(kind lexer.TokenKind) (lexer.Token, error) {
	if p.atEnd() || p.peek().Kind != kind {
		got := p.peek()
		return lexer.Token{}, fmt.Errorf("line %d:%d: expected %v, got %q", got.Line, got.Col, kind, got.Literal)
	}
	return p.advance(), nil
}

func (p *Parser) skipNewlines() {
	for !p.atEnd() && p.peek().Kind == lexer.NEWLINE {
		p.advance()
	}
}

// ── Top-level declarations ────────────────────────────────────────────────

func (p *Parser) parseHook() (*HookDecl, error) {
	kwTok := p.advance() // consume "hook"
	nameTok, err := p.expect(lexer.IDENT)
	if err != nil {
		return nil, err
	}
	if _, err := p.expect(lexer.LBRACE); err != nil {
		return nil, err
	}
	body, err := p.parseBody()
	if err != nil {
		return nil, err
	}
	if _, err := p.expect(lexer.RBRACE); err != nil {
		return nil, err
	}
	return &HookDecl{Name: nameTok.Literal, Line: kwTok.Line, Body: body}, nil
}

func (p *Parser) parseWorkflow() (*WorkflowDecl, error) {
	kwTok := p.advance() // consume "workflow"
	nameTok, err := p.expect(lexer.IDENT)
	if err != nil {
		return nil, err
	}
	if _, err := p.expect(lexer.LBRACE); err != nil {
		return nil, err
	}
	var steps []*StepStmt
	for {
		p.skipNewlines()
		if p.atEnd() || p.peek().Kind == lexer.RBRACE {
			break
		}
		s, err := p.parseStep()
		if err != nil {
			return nil, err
		}
		steps = append(steps, s)
	}
	if _, err := p.expect(lexer.RBRACE); err != nil {
		return nil, err
	}
	return &WorkflowDecl{Name: nameTok.Literal, Line: kwTok.Line, Steps: steps}, nil
}

// ── Body / statements ─────────────────────────────────────────────────────

func (p *Parser) parseBody() ([]Stmt, error) {
	var stmts []Stmt
	for {
		p.skipNewlines()
		if p.atEnd() || p.peek().Kind == lexer.RBRACE {
			break
		}
		stmt, err := p.parseStmt()
		if err != nil {
			return nil, err
		}
		if stmt != nil {
			stmts = append(stmts, stmt)
		}
	}
	return stmts, nil
}

func (p *Parser) parseStmt() (Stmt, error) {
	tok := p.peek()
	switch tok.Kind {
	case lexer.REQUIRE:
		return p.parseRequire()
	case lexer.REJECT:
		return p.parseReject()
	case lexer.RUN:
		return p.parseRun()
	case lexer.IDENT:
		return p.parseDotStmt()
	default:
		return nil, fmt.Errorf("line %d:%d: unexpected token %q in body", tok.Line, tok.Col, tok.Literal)
	}
}

func (p *Parser) parseStep() (*StepStmt, error) {
	kwTok, err := p.expect(lexer.STEP)
	if err != nil {
		return nil, err
	}
	if _, err := p.expect(lexer.LBRACE); err != nil {
		return nil, err
	}
	p.skipNewlines()
	inner, err := p.parseStmt()
	if err != nil {
		return nil, err
	}
	p.skipNewlines()
	if _, err := p.expect(lexer.RBRACE); err != nil {
		return nil, err
	}
	return &StepStmt{Line: kwTok.Line, Inner: inner}, nil
}

func (p *Parser) parseRequire() (*RequireStmt, error) {
	tok := p.advance() // consume "require"
	pred, err := p.parsePredicate()
	if err != nil {
		return nil, err
	}
	return &RequireStmt{Line: tok.Line, Predicate: pred}, nil
}

func (p *Parser) parseReject() (*RejectStmt, error) {
	tok := p.advance() // consume "reject"
	if _, err := p.expect(lexer.IF); err != nil {
		return nil, err
	}
	pred, err := p.parsePredicate()
	if err != nil {
		return nil, err
	}
	return &RejectStmt{Line: tok.Line, Predicate: pred}, nil
}

func (p *Parser) parseRun() (*RunStmt, error) {
	tok := p.advance() // consume "run"
	cmdTok, err := p.expect(lexer.STRING)
	if err != nil {
		return nil, err
	}
	return &RunStmt{Line: tok.Line, Command: cmdTok.Literal}, nil
}

// parseDotStmt handles "branch.ensure" / "tag.create" / etc.
func (p *Parser) parseDotStmt() (Stmt, error) {
	objTok := p.advance()
	if p.atEnd() || p.peek().Kind != lexer.DOT {
		return nil, fmt.Errorf("line %d:%d: expected '.' after %q", objTok.Line, objTok.Col, objTok.Literal)
	}
	p.advance() // consume '.'
	methodTok, err := p.expect(lexer.IDENT)
	if err != nil {
		return nil, err
	}

	switch objTok.Literal + "." + methodTok.Literal {
	case "branch.ensure":
		return p.parseBranchEnsure(objTok.Line)
	case "tag.create":
		return p.parseTagCreate(objTok.Line)
	default:
		return nil, fmt.Errorf("line %d:%d: unknown method %q.%q", objTok.Line, objTok.Col, objTok.Literal, methodTok.Literal)
	}
}

func (p *Parser) parseBranchEnsure(line int) (*BranchEnsureStmt, error) {
	// name = "<pattern>"
	if _, err := p.expect(lexer.IDENT); err != nil { // "name"
		return nil, err
	}
	if _, err := p.expect(lexer.ASSIGN); err != nil {
		return nil, err
	}
	patTok, err := p.expect(lexer.STRING)
	if err != nil {
		return nil, err
	}
	return &BranchEnsureStmt{Line: line, Pattern: patTok.Literal}, nil
}

func (p *Parser) parseTagCreate(line int) (*TagCreateStmt, error) {
	stmt := &TagCreateStmt{Line: line}
	// prefix = "<p>" auto-increment = true
	for !p.atEnd() && p.peek().Kind == lexer.IDENT {
		keyTok := p.advance()
		if _, err := p.expect(lexer.ASSIGN); err != nil {
			return nil, err
		}
		valTok := p.advance()
		switch keyTok.Literal {
		case "prefix":
			stmt.Prefix = valTok.Literal
		case "auto-increment":
			stmt.AutoIncrement = valTok.Literal == "true"
		}
	}
	return stmt, nil
}

// parsePredicate: <field> matches /<pattern>/ | <field> is [not] empty
func (p *Parser) parsePredicate() (*Predicate, error) {
	// field: one or more IDENT separated by DOT
	firstTok := p.peek()
	field, err := p.parseDotField()
	if err != nil {
		return nil, err
	}

	opTok := p.peek()
	switch opTok.Kind {
	case lexer.MATCHES:
		p.advance()
		regexTok, err := p.expect(lexer.REGEX)
		if err != nil {
			return nil, err
		}
		return &Predicate{Kind: PredMatchesRegex, Field: field, Pattern: regexTok.Literal, Tok: firstTok}, nil

	case lexer.IS:
		p.advance()
		negate := false
		if !p.atEnd() && p.peek().Kind == lexer.NOT {
			p.advance()
			negate = true
		}
		if _, err := p.expect(lexer.EMPTY); err != nil {
			return nil, err
		}
		return &Predicate{Kind: PredIsEmpty, Field: field, Negate: negate, Tok: firstTok}, nil

	default:
		return nil, fmt.Errorf("line %d:%d: expected 'matches' or 'is', got %q", opTok.Line, opTok.Col, opTok.Literal)
	}
}

func (p *Parser) parseDotField() ([]string, error) {
	tok, err := p.expect(lexer.IDENT)
	if err != nil {
		return nil, err
	}
	field := []string{tok.Literal}
	for !p.atEnd() && p.peek().Kind == lexer.DOT {
		p.advance() // consume '.'
		next, err := p.expect(lexer.IDENT)
		if err != nil {
			return nil, err
		}
		field = append(field, next.Literal)
	}
	return field, nil
}
