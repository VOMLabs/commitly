#!/usr/bin/env python3
"""
tools/compile.py — build Commitly to a runnable executable.

Usage:
    python tools/compile.py                  # release build
    python tools/compile.py --debug          # debug build
    python tools/compile.py --clean          # wipe build dir first
    python tools/compile.py --target-dir D   # custom build dir (default: native/build)
    python tools/compile.py --skip-go        # skip commitlyc compiler build
    python tools/compile.py --skip-rust      # skip Rust crate check
"""

from __future__ import annotations

import argparse
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path

# ── Paths ─────────────────────────────────────────────────────────────────────
REPO_ROOT = Path(__file__).resolve().parent.parent
NATIVE_DIR = REPO_ROOT / "native"
COMPILER_DIR = REPO_ROOT / "compiler"
DEFAULT_BUILD_DIR = NATIVE_DIR / "build"

EXE_SUFFIX = ".exe" if platform.system() == "Windows" else ""
COMMITLY_BIN = f"commitly{EXE_SUFFIX}"
COMMITLYC_BIN = f"commitlyc{EXE_SUFFIX}"


def run(cmd: list[str], cwd: Path, *, label: str) -> None:
    print(f"\n{'─'*60}")
    print(f"  {label}")
    print(f"  {' '.join(str(c) for c in cmd)}")
    print(f"{'─'*60}")
    result = subprocess.run(cmd, cwd=cwd)
    if result.returncode != 0:
        print(f"\n[ERROR] {label} failed (exit {result.returncode})", file=sys.stderr)
        sys.exit(result.returncode)


def find_cmake() -> str:
    cmake = shutil.which("cmake")
    if not cmake:
        print("[ERROR] cmake not found on PATH. Install CMake >= 3.25.", file=sys.stderr)
        sys.exit(1)
    return cmake


def find_go() -> str | None:
    return shutil.which("go")


def find_cargo() -> str:
    cargo = shutil.which("cargo")
    if not cargo:
        print("[ERROR] cargo not found on PATH. Install Rust via https://rustup.rs", file=sys.stderr)
        sys.exit(1)
    return cargo


def build_go_compiler(build_dir: Path, skip: bool) -> None:
    if skip:
        print("[SKIP] commitlyc (--skip-go)")
        return
    go = find_go()
    if not go:
        print("[WARN] go not found on PATH — skipping commitlyc build.", file=sys.stderr)
        return
    out = build_dir / COMMITLYC_BIN
    run(
        [go, "build", "-o", str(out), "."],
        cwd=COMPILER_DIR,
        label="Building commitlyc (Go)",
    )
    print(f"[OK] commitlyc → {out}")


def build_native(
    build_dir: Path,
    build_type: str,
    skip_rust: bool,
) -> Path:
    cmake = find_cmake()

    # If skipping Rust we still need a dummy lib or the link will fail;
    # accept the flag but warn the user.
    extra_defs: list[str] = []
    if skip_rust:
        print("[WARN] --skip-rust passed; the Rust FFI symbols will be missing at link time.")

    # Configure
    run(
        [cmake, str(NATIVE_DIR), f"-DCMAKE_BUILD_TYPE={build_type}", *extra_defs],
        cwd=build_dir,
        label=f"CMake configure ({build_type})",
    )

    # Build
    cpu_count = os.cpu_count() or 2
    run(
        [cmake, "--build", ".", "--parallel", str(cpu_count)],
        cwd=build_dir,
        label="CMake build",
    )

    exe = build_dir / COMMITLY_BIN
    if not exe.exists():
        # MSVC puts the binary inside a config subfolder
        exe = build_dir / build_type / COMMITLY_BIN

    if not exe.exists():
        print(f"[ERROR] Expected executable not found at {exe}", file=sys.stderr)
        sys.exit(1)

    return exe


def main() -> None:
    parser = argparse.ArgumentParser(description="Build Commitly native app")
    parser.add_argument("--debug", action="store_true", help="Debug build (default: Release)")
    parser.add_argument("--clean", action="store_true", help="Delete build dir before building")
    parser.add_argument("--target-dir", type=Path, default=DEFAULT_BUILD_DIR, metavar="DIR")
    parser.add_argument("--skip-go", action="store_true", help="Skip commitlyc Go compiler build")
    parser.add_argument("--skip-rust", action="store_true", help="Skip Rust crate check (link may fail)")
    args = parser.parse_args()

    build_type = "Debug" if args.debug else "Release"
    build_dir: Path = args.target_dir.resolve()

    print(f"[commitly build]  type={build_type}  dir={build_dir}")

    if args.clean and build_dir.exists():
        print(f"[CLEAN] Removing {build_dir}")
        shutil.rmtree(build_dir)

    build_dir.mkdir(parents=True, exist_ok=True)

    # 1. Go compiler
    build_go_compiler(build_dir, args.skip_go)

    # 2. Native app (CMake → Qt6 + Rust)
    exe = build_native(build_dir, build_type, args.skip_rust)

    print(f"\n{'═'*60}")
    print(f"  BUILD COMPLETE")
    print(f"  Executable : {exe}")
    commitlyc = build_dir / COMMITLYC_BIN
    if commitlyc.exists():
        print(f"  Compiler   : {commitlyc}")
    print(f"{'═'*60}\n")


if __name__ == "__main__":
    main()
