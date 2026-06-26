#!/usr/bin/env python3

from pathlib import Path
import subprocess
import sys
import re
import argparse
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent

def run(cmd):
    print("+", " ".join(cmd))
    subprocess.run(cmd, check=True)

def build_database(app: str, specs: str):
    build_script = SCRIPT_DIR / "build.py"
    cmd = [
        sys.executable,
        str(build_script),
        "--app",
        app,
        "--specs",
        specs,
        "--clean",
        "yes",
        "--native_sim",
    ]
    run(cmd)


def filter_database():
    Path("build_clang").mkdir(exist_ok=True)
    filter_script = SCRIPT_DIR / "filter_compile_commands.py"
    run([
        sys.executable,
        str(filter_script),
        "build/compile_commands.json",
        "build_clang/compile_commands.json",
    ])


def run_clang_tidy_patterns(workdir: str, app: str, include_zpp_lib: bool = False):
    if include_zpp_lib:
        patterns = [
            rf"{workdir}/{app}/src/.*\.cpp$",
            rf"zpp_lib/.*\.cpp$",
        ]
    else:
        patterns = [
            rf"{workdir}/{app}/src/.*\.cpp$"
        ]

    for pattern in patterns:
        run([
            "run-clang-tidy-22",
            "-p",
            "build_clang",
            pattern,
            "--warning-as-error *",
            "-quiet",
        ])


def run_clang_tidy_files(files):
    for f in files:
        if not f.endswith((".cpp", ".cc", ".cxx")):
            continue

        run([
            "clang-tidy-22",
            "-p",
            "build_clang",
            f,
            "--warnings-as-errors=*",
            "-quiet",
        ])


def main():
    
    parser = argparse.ArgumentParser()
    parser.add_argument("--app", required=True)
    parser.add_argument("--specs", required=True)
    parser.add_argument("--wd", required=True)
    parser.add_argument("files", nargs="*")

    args = parser.parse_args()
    
    args.app = args.app.rstrip("/\\")
    
    print(f"App: {args.app}")
    print(f"Specs: {args.specs}")
    print(f"Working directory: {args.wd}")

    build_database(args.app, args.specs)
    filter_database()
    if args.files:
        print("Running clang-tidy on files:")
        for f in args.files:
            print(f"  {f}")
        run_clang_tidy_files(args.files)
    else:
        print("Running clang-tidy on app:" + args.app)
        # When the app is one of the zpp_lib tests programs, we include zpp_lib in the analysis
        include_zpp_lib = "zpp_" in args.app
        print(f"Include zpp_lib: {include_zpp_lib}")
        run_clang_tidy_patterns(args.wd, args.app, include_zpp_lib=include_zpp_lib)

if __name__ == "__main__":
    main()