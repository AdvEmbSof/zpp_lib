#!/usr/bin/env python3

from pathlib import Path
import argparse
import json
import subprocess
import sys


SCRIPT_DIR = Path(__file__).resolve().parent
BUILD_DIR = Path("build")
CLANG_BUILD_DIR = Path("build_clang")


def run(cmd: list[str]) -> None:
    """Run a command and print it first."""
    print("+", " ".join(cmd))
    subprocess.run(cmd, check=True)


def normalize_path(path: str | Path) -> str:
    """Return a path using forward slashes."""
    return str(path).replace("\\", "/")


def build_database(
    app: str,
    configs: str,
    board: str,
    shield: str | None = None,
    app_config: str | None = None,
) -> None:
    """Build the application and generate compile_commands.json."""
    build_script = SCRIPT_DIR / "build.py"

    cmd = [
        sys.executable,
        str(build_script),
        "--app",
        app,
        "--configs",
        configs,
        "--board",
        board,
        "--pristine",
    ]

    if shield:
        cmd.extend(["--shield", shield])

    if app_config:
        cmd.extend(["--app-config", app_config])

    run(cmd)


def filter_database() -> None:
    """Generate the clang-tidy-compatible compilation database."""
    CLANG_BUILD_DIR.mkdir(exist_ok=True)

    filter_script = SCRIPT_DIR / "filter_compile_commands.py"

    run([
        sys.executable,
        str(filter_script),
        str(BUILD_DIR / "compile_commands.json"),
        str(CLANG_BUILD_DIR / "compile_commands.json"),
    ])


def read_compilation_database() -> list[dict]:
    """Read the filtered compilation database."""
    database_path = CLANG_BUILD_DIR / "compile_commands.json"

    if not database_path.exists():
        raise RuntimeError(
            f"Compilation database not found: {database_path}"
        )

    with database_path.open(encoding="utf-8") as f:
        return json.load(f)


def is_cpp_file(path: str) -> bool:
    """Return whether the path refers to a C++ source file."""
    return path.lower().endswith((".cpp", ".cc", ".cxx"))


def application_source_marker(app: str) -> str:
    """
    Return the path fragment identifying the application's source directory.

    For example:
        blinky
            -> /blinky/src/

        zpp_rtos/tests/mutex
            -> /zpp_rtos/tests/mutex/src/
    """
    app = normalize_path(app).strip("/")
    return f"/{app}/src/"


def select_application_files(
    database: list[dict],
    app: str,
    include_zpp_lib: bool,
) -> list[str]:
    """Select only source files belonging to the application."""

    marker = application_source_marker(app)

    files: list[str] = []

    for entry in database:
        file = normalize_path(entry["file"])

        if not is_cpp_file(file):
            continue

        # Application sources.
        if marker in file:
            files.append(file)
            continue

        # zpp_lib sources for zpp_* test applications.
        if include_zpp_lib and "/deps/zpp_lib/" in file:
            files.append(file)

    return files


def run_clang_tidy_files(files: list[str]) -> None:
    """Run clang-tidy on the selected source files."""
    if not files:
        raise RuntimeError("No C++ files selected for clang-tidy.")

    print(f"Running clang-tidy on {len(files)} file(s):")
    run(["clang-tidy", "--version",])
    run(["clang-tidy", "--dump-config",])

    with open("build_clang/compile_commands.json") as f:
        db = json.load(f)

    for entry in db:
        if entry["file"].endswith("zpp_rtos/tests/mutex/src/main.cpp"):
            print(entry["command"])
            break
        
    for file in files:
        print(f"  {file}")

        run([
            "clang-tidy",
            "-p",
            str(CLANG_BUILD_DIR),
            file,
            "--warnings-as-errors=*",
            "-quiet",
        ])


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Build a Zephyr application and run clang-tidy "
                    "on its C++ sources."
    )

    parser.add_argument(
        "--app",
        required=True,
        help="Application directory relative to the workspace.",
    )

    parser.add_argument(
        "--board",
        default="native_sim",
        help="Zephyr board.",
    )

    parser.add_argument(
        "--configs",
        required=True,
        help="Build configuration files.",
    )

    parser.add_argument(
        "--wd",
        required=False,
        help="Workspace directory. Kept for compatibility; "
             "application paths are obtained from the compilation database.",
    )

    parser.add_argument(
        "--shield",
        required=False,
    )

    parser.add_argument(
        "--app-config",
        required=False,
    )

    parser.add_argument(
        "files",
        nargs="*",
        help="Optional source files to analyze instead of the whole application.",
    )

    args = parser.parse_args()

    # Normalize application path once.
    args.app = normalize_path(args.app).rstrip("/")

    print(f"App: {args.app}")
    print(f"Configs: {args.configs}")

    if args.app_config:
        print(f"App Config: {args.app_config}")

    if args.wd:
        print(f"Working directory: {normalize_path(args.wd)}")

    print(f"Board: {args.board}")

    if args.shield:
        print(f"Shield: {args.shield}")

    # ------------------------------------------------------------------
    # Step 1: Build with GCC and generate compile_commands.json
    # ------------------------------------------------------------------
    build_database(
        args.app,
        args.configs,
        args.board,
        args.shield,
        args.app_config,
    )

    # ------------------------------------------------------------------
    # Step 2: Generate the clang-tidy-compatible database
    # ------------------------------------------------------------------
    filter_database()

    # ------------------------------------------------------------------
    # Step 3: Determine which files to analyze
    # ------------------------------------------------------------------
    if args.files:
        files = [
            normalize_path(file)
            for file in args.files
            if is_cpp_file(file)
        ]

        print("Running clang-tidy on explicitly specified files:")

    else:
        database = read_compilation_database()

        # zpp_* test applications also analyze the zpp_lib sources.
        include_zpp_lib = "zpp_" in args.app

        print(f"Running clang-tidy on app: {args.app}")
        print(f"Include zpp_lib: {include_zpp_lib}")

        files = select_application_files(
            database,
            args.app,
            include_zpp_lib,
        )

    if not files:
        raise RuntimeError(
            f"No C++ source files found for application '{args.app}'."
        )

    # ------------------------------------------------------------------
    # Step 4: Run clang-tidy
    # ------------------------------------------------------------------
    run_clang_tidy_files(files)


if __name__ == "__main__":
    main()
