package lexer

import (
	"fmt"
	"strings"
	"unicode"
)

// Lexer tokenises .cly source.
type Lexer struct {
	input  []rune
	pos    int
	line   int
	col    int
}

// New creates a Lexer for the given source string.
func New(src string) *Lexer {
	return &Lexer{input: []rune(src), line: 1, col: 1}
}

// Tokenise returns all tokens including EOF.
func (l *Lexer) Tokenise() ([]Token, error) {
	var tokens []Token
	for {
		tok, err := l.next()
		if err != nil {
			return nil, err
		}
		if tok.Kind == COMMENT {
			continue
		}
		tokens = append(tokens, tok)
		if tok.Kind == EOF {
			break
		}
	}
	return tokens, nil
}

func (l *Lexer) peek() rune {
	if l.pos >= len(l.input) {
		return 0
	}
	return l.input[l.pos]
}

func (l *Lexer) advance() rune {
	ch := l.input[l.pos]
	l.pos++
	if ch == '\n' {
		l.line++
		l.col = 1
	} else {
		l.col++
	}
	return ch
}

func (l *Lexer) next() (Token, error) {
	// Skip spaces and tabs (not newlines — they are NEWLINE tokens)
	for l.pos < len(l.input) && (l.peek() == ' ' || l.peek() == '\t' || l.peek() == '\r') {
		l.advance()
	}
	if l.pos >= len(l.input) {
		return Token{Kind: EOF, Line: l.line, Col: l.col}, nil
	}

	startLine, startCol := l.line, l.col
	ch := l.advance()

	switch {
	case ch == '\n':
		return Token{Kind: NEWLINE, Literal: "\n", Line: startLine, Col: startCol}, nil

	case ch == '#':
		// Comment: consume to end of line
		var sb strings.Builder
		for l.pos < len(l.input) && l.peek() != '\n' {
			sb.WriteRune(l.advance())
		}
		return Token{Kind: COMMENT, Literal: sb.String(), Line: startLine, Col: startCol}, nil

	case ch == '{':
		return Token{Kind: LBRACE, Literal: "{", Line: startLine, Col: startCol}, nil
	case ch == '}':
		return Token{Kind: RBRACE, Literal: "}", Line: startLine, Col: startCol}, nil
	case ch == '=':
		return Token{Kind: ASSIGN, Literal: "=", Line: startLine, Col: startCol}, nil
	case ch == '.':
		return Token{Kind: DOT, Literal: ".", Line: startLine, Col: startCol}, nil

	case ch == '"':
		// String literal
		var sb strings.Builder
		for l.pos < len(l.input) && l.peek() != '"' {
			c := l.advance()
			if c == '\\' && l.pos < len(l.input) {
				escaped := l.advance()
				switch escaped {
				case 'n':
					sb.WriteRune('\n')
				case 't':
					sb.WriteRune('\t')
				default:
					sb.WriteRune(escaped)
				}
			} else {
				sb.WriteRune(c)
			}
		}
		if l.pos >= len(l.input) {
			return Token{}, fmt.Errorf("line %d:%d: unterminated string literal", startLine, startCol)
		}
		l.advance() // closing "
		return Token{Kind: STRING, Literal: sb.String(), Line: startLine, Col: startCol}, nil

	case ch == '/':
		// Regex literal /pattern/
		var sb strings.Builder
		for l.pos < len(l.input) && l.peek() != '/' {
			if l.peek() == '\n' {
				return Token{}, fmt.Errorf("line %d:%d: unterminated regex literal", startLine, startCol)
			}
			c := l.advance()
			if c == '\\' && l.pos < len(l.input) {
				sb.WriteRune(c)
				sb.WriteRune(l.advance())
			} else {
				sb.WriteRune(c)
			}
		}
		if l.pos >= len(l.input) {
			return Token{}, fmt.Errorf("line %d:%d: unterminated regex literal", startLine, startCol)
		}
		l.advance() // closing /
		return Token{Kind: REGEX, Literal: sb.String(), Line: startLine, Col: startCol}, nil

	case unicode.IsLetter(ch) || ch == '_' || ch == '-':
		// Identifier or keyword (allow hyphens in names like "pre-commit")
		var sb strings.Builder
		sb.WriteRune(ch)
		for l.pos < len(l.input) && (unicode.IsLetter(l.peek()) || unicode.IsDigit(l.peek()) || l.peek() == '_' || l.peek() == '-') {
			sb.WriteRune(l.advance())
		}
		lit  := sb.String()
		kind := LookupIdent(lit)
		return Token{Kind: kind, Literal: lit, Line: startLine, Col: startCol}, nil

	case unicode.IsDigit(ch):
		var sb strings.Builder
		sb.WriteRune(ch)
		for l.pos < len(l.input) && unicode.IsDigit(l.peek()) {
			sb.WriteRune(l.advance())
		}
		return Token{Kind: IDENT, Literal: sb.String(), Line: startLine, Col: startCol}, nil

	default:
		return Token{Kind: ILLEGAL, Literal: string(ch), Line: startLine, Col: startCol},
			fmt.Errorf("line %d:%d: illegal character %q", startLine, startCol, ch)
	}
}
