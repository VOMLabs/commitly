package sema

import (
	"fmt"
	"regexp"
	"strings"

	"github.com/vomlabs/commitly/compiler/parser"
)

var validHookNames = map[string]bool{
	"pre-commit":         true,
	"commit-msg":         true,
	"post-commit":        true,
	"pre-push":           true,
	"post-merge":         true,
	"pre-rebase":         true,
	"post-checkout":      true,
	"post-rewrite":       true,
	"prepare-commit-msg": true,
}

var validFields = map[string]bool{
	"commit.summary":  true,
	"commit.body":     true,
	"staged.files":    true,
	"commit.author":   true,
}

// Check performs semantic analysis on a parsed program.
// Returns a slice of errors (may be multiple).
func Check(prog *parser.Program) []error {
	var errs []error

	hookNames := map[string]int{}
	for _, h := range prog.Hooks {
		// Duplicate hook check
		if prev, ok := hookNames[h.Name]; ok {
			errs = append(errs, fmt.Errorf("line %d: duplicate hook %q (first declared at line %d)", h.Line, h.Name, prev))
		} else {
			hookNames[h.Name] = h.Line
		}
		// Valid hook name check
		if !validHookNames[h.Name] {
			errs = append(errs, fmt.Errorf("line %d: unknown hook name %q (valid: %s)", h.Line, h.Name, knownHooks()))
		}
		// Check body statements
		for _, stmt := range h.Body {
			errs = append(errs, checkStmt(stmt)...)
		}
	}

	workflowNames := map[string]int{}
	for _, w := range prog.Workflows {
		if prev, ok := workflowNames[w.Name]; ok {
			errs = append(errs, fmt.Errorf("line %d: duplicate workflow %q (first declared at line %d)", w.Line, w.Name, prev))
		} else {
			workflowNames[w.Name] = w.Line
		}
		for _, step := range w.Steps {
			if step.Inner != nil {
				errs = append(errs, checkStmt(step.Inner)...)
			}
		}
	}

	return errs
}

func checkStmt(stmt parser.Stmt) []error {
	var errs []error
	switch s := stmt.(type) {
	case *parser.RequireStmt:
		errs = append(errs, checkPredicate(s.Predicate)...)
	case *parser.RejectStmt:
		errs = append(errs, checkPredicate(s.Predicate)...)
	case *parser.RunStmt:
		if strings.TrimSpace(s.Command) == "" {
			errs = append(errs, fmt.Errorf("line %d: run command is empty", s.Line))
		}
	case *parser.BranchEnsureStmt:
		if s.Pattern == "" {
			errs = append(errs, fmt.Errorf("line %d: branch.ensure pattern is empty", s.Line))
		}
	}
	return errs
}

func checkPredicate(pred *parser.Predicate) []error {
	var errs []error
	field := strings.Join(pred.Field, ".")
	if !validFields[field] {
		errs = append(errs, fmt.Errorf("line %d:%d: unknown field %q", pred.Tok.Line, pred.Tok.Col, field))
	}
	if pred.Kind == parser.PredMatchesRegex {
		if _, err := regexp.Compile(pred.Pattern); err != nil {
			errs = append(errs, fmt.Errorf("line %d:%d: invalid regex /%s/: %v", pred.Tok.Line, pred.Tok.Col, pred.Pattern, err))
		}
	}
	return errs
}

func knownHooks() string {
	ks := make([]string, 0, len(validHookNames))
	for k := range validHookNames {
		ks = append(ks, k)
	}
	return strings.Join(ks, ", ")
}
