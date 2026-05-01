package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/vomlabs/commitly/compiler/codegen"
	"github.com/vomlabs/commitly/compiler/lexer"
	"github.com/vomlabs/commitly/compiler/parser"
	"github.com/vomlabs/commitly/compiler/sema"
	"github.com/vomlabs/commitly/compiler/vm"
)

func main() {
	checkFlag   := flag.Bool("check", false, "lint only, do not emit bytecode")
	emitAST     := flag.Bool("emit-ast", false, "print AST as JSON and exit")
	disasm      := flag.Bool("disassemble", false, "disassemble .clyc file")
	runFlag     := flag.String("run", "", "run .clyc file with given hook name (for local testing)")
	outputFlag  := flag.String("output", "", "output path for .clyc file (default: <input>.clyc)")
	flag.Parse()

	args := flag.Args()
	if len(args) != 1 {
		fmt.Fprintf(os.Stderr, "usage: commitlyc [flags] <source.cly | source.clyc>\n")
		flag.PrintDefaults()
		os.Exit(1)
	}
	inputPath := args[0]

	// ── Disassemble mode ──────────────────────────────────────────────────
	if *disasm {
		data, err := os.ReadFile(inputPath)
		if err != nil {
			fatalf("cannot read %s: %v", inputPath, err)
		}
		fmt.Print(codegen.Disassemble(data))
		return
	}

	// ── Compile ───────────────────────────────────────────────────────────
	src, err := os.ReadFile(inputPath)
	if err != nil {
		fatalf("cannot read %s: %v", inputPath, err)
	}

	// Lex
	l := lexer.New(string(src))
	tokens, err := l.Tokenise()
	if err != nil {
		fatalf("%s: %v", inputPath, err)
	}

	// Parse
	p := parser.New(tokens)
	prog, err := p.Parse()
	if err != nil {
		fatalf("%s: %v", inputPath, err)
	}

	// Emit AST
	if *emitAST {
		enc := json.NewEncoder(os.Stdout)
		enc.SetIndent("", "  ")
		type astView struct {
			Hooks     []*parser.HookDecl
			Workflows []*parser.WorkflowDecl
		}
		_ = enc.Encode(astView{Hooks: prog.Hooks, Workflows: prog.Workflows})
		return
	}

	// Semantic analysis
	errs := sema.Check(prog)
	if len(errs) > 0 {
		for _, e := range errs {
			fmt.Fprintf(os.Stderr, "%s: %v\n", inputPath, e)
		}
		os.Exit(1)
	}

	if *checkFlag {
		fmt.Printf("%s: OK (no errors)\n", inputPath)
		return
	}

	// Emit bytecode
	emitter  := codegen.NewEmitter()
	bytecode := emitter.Emit(prog)

	outPath := *outputFlag
	if outPath == "" {
		ext := filepath.Ext(inputPath)
		outPath = strings.TrimSuffix(inputPath, ext) + ".clyc"
	}

	if err := os.WriteFile(outPath, bytecode, 0o644); err != nil {
		fatalf("cannot write %s: %v", outPath, err)
	}
	fmt.Printf("compiled %s → %s (%d bytes)\n", inputPath, outPath, len(bytecode))

	// ── Run mode (local test) ─────────────────────────────────────────────
	if *runFlag != "" {
		ctx := &vm.Context{
			HookName:      *runFlag,
			StagedFiles:   []string{"example.txt"}, // placeholder for local test
			CommitSummary: "feat: example commit",
		}
		result := vm.Run(bytecode, ctx)
		if result.OK {
			fmt.Printf("hook %q: OK\n", *runFlag)
		} else {
			fmt.Fprintf(os.Stderr, "hook %q: REJECTED — %s\n", *runFlag, result.Message)
			os.Exit(1)
		}
	}
}

func fatalf(format string, args ...any) {
	fmt.Fprintf(os.Stderr, "commitlyc: "+format+"\n", args...)
	os.Exit(1)
}
