package lexer

// TokenKind enumerates all lexer token types.
type TokenKind int

const (
	EOF TokenKind = iota

	// Keywords
	HOOK
	WORKFLOW
	STEP
	RUN
	REQUIRE
	REJECT
	IF
	MATCHES
	IS
	EMPTY
	NOT

	// Literals
	IDENT
	STRING
	REGEX // /pattern/

	// Punctuation
	LBRACE
	RBRACE
	ASSIGN // =
	DOT
	NEWLINE

	// Misc
	COMMENT
	ILLEGAL
)

var keywords = map[string]TokenKind{
	"hook":     HOOK,
	"workflow": WORKFLOW,
	"step":     STEP,
	"run":      RUN,
	"require":  REQUIRE,
	"reject":   REJECT,
	"if":       IF,
	"matches":  MATCHES,
	"is":       IS,
	"empty":    EMPTY,
	"not":      NOT,
}

// Token is a single lexical unit.
type Token struct {
	Kind    TokenKind
	Literal string
	Line    int
	Col     int
}

func (t Token) String() string {
	return t.Literal
}

// LookupIdent returns the keyword kind for s, or IDENT if not a keyword.
func LookupIdent(s string) TokenKind {
	if k, ok := keywords[s]; ok {
		return k
	}
	return IDENT
}
