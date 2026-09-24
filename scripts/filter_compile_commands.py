#!/usr/bin/env python3
# filter_compile_commands.py
# Generate a clang-tidy-compatible compile_commands.json
# by stripping GCC-specific flags from the Zephyr build output.
#
# In addition to normal C++ compilation entries, generate one
# translation unit (.cpp) for every application header below
# <header-root>. This allows clang-tidy to lint application headers
# without enabling linting of system headers.
#
# Usage:
#   python3 filter_compile_commands.py \
#       build/compile_commands.json \
#       build_clang/compile_commands.json \
#       deps/zpp_lib/zpp_include

import argparse
import json
import os
import re
import subprocess
from pathlib import Path


# Exact flags to remove
STRIP_FLAGS = {
    '-fno-reorder-functions',
    '-mpreferred-stack-boundary=2',
    '-fno-freestanding',
    '-fno-defer-pop',
    '-fsanitize=bounds-strict',
    '-specs=picolibc.specs',
    '--param=min-pagesize=0',
    '-mfp16-format=ieee',
}

# Regex patterns for flags with values (flag and its argument)
STRIP_PATTERNS = [
    r'^--sysroot$',
    r'^--sysroot=.*$',
]

# Flags to replace — GCC-specific flags that have a Clang equivalent
REPLACE_FLAGS = {
    # Keep -mcpu for Clang but strip GCC sub-options it does not understand.
}

# Add system C++ headers so <chrono> etc. are found
EXTRA_ARGS = [
    '-DZPP_CLANG_TIDY',
    '--target=arm-none-eabi',
    '-Wno-unknown-warning-option',
    '-Wno-unused-command-line-argument',
]

CXX_SUFFIXES = ('.cpp', '.cxx', '.cc')
HEADER_SUFFIXES = ('.h', '.hh', '.hpp', '.hxx')


def gcc_include_paths(compiler: str) -> list[str]:
    """Return GCC C++/target include paths suitable for Clang."""
    result = subprocess.run(
        [
            compiler,
            "-v",
            "-E",
            "-x",
            "c++",
            os.devnull,
        ],
        input="",
        text=True,
        capture_output=True,
        check=True,
    )

    paths = []
    in_search_list = False

    for line in result.stderr.splitlines():
        if line == "#include <...> search starts here:":
            in_search_list = True
            continue

        if in_search_list:
            if line == "End of search list.":
                break

            path = Path(line.strip()).resolve()

            # Normalize before testing GCC private headers.
            if "/lib/gcc/" in path.as_posix():
                continue

            if path.exists():
                paths.append(path.as_posix())

    return paths


def find_gcc_compiler(db: list[dict]) -> str:
    """Find the GCC C/C++ compiler used by the compilation database."""
    for entry in db:
        if "arguments" in entry and entry["arguments"]:
            compiler = entry["arguments"][0]

        elif "command" in entry:
            match = re.match(
                r'\s*(?:"([^"]+)"|(\S+))',
                entry["command"],
            )
            if not match:
                continue

            compiler = match.group(1) or match.group(2)

        else:
            continue

        compiler_name = Path(compiler).stem.lower()

        if compiler_name.endswith(("gcc", "g++")):
            return compiler

    raise RuntimeError(
        "Could not find a GCC compiler in compile_commands.json"
    )


def filter_command(
    command: str,
    gcc_includes: list[str],
) -> str:
    """Convert a GCC command string to a Clang command string."""
    if os.name == "nt":
        command = command.replace("\\", "/")

    parts = command.split()
    result = []
    skip = False

    for part in parts:
        if skip:
            skip = False
            continue

        if part in STRIP_FLAGS:
            continue

        if any(re.match(pattern, part) for pattern in STRIP_PATTERNS):
            if "=" not in part:
                skip = True
            continue

        result.append(part)

    if result:
        compiler_name = Path(result[0]).stem.lower()

        if compiler_name.endswith(("gcc", "g++")):
            result[0] = "clang++"

            # Tell Clang where GCC's C++ standard library headers are.
            for path in gcc_includes:
                result.extend(["-isystem", path])

        result.extend(EXTRA_ARGS)

    return " ".join(result)


def filter_arguments(
    arguments: list[str],
    gcc_includes: list[str],
) -> list[str]:
    """Convert an arguments-style GCC command to Clang arguments."""
    result = []
    skip = False

    for arg in arguments:
        if skip:
            skip = False
            continue

        if arg in STRIP_FLAGS:
            continue

        if any(re.match(pattern, arg) for pattern in STRIP_PATTERNS):
            if "=" not in arg:
                skip = True
            continue

        result.append(arg)

    if result:
        compiler_name = Path(result[0]).stem.lower()

        if compiler_name.endswith(("gcc", "g++")):
            result[0] = "clang++"

            # Tell Clang where GCC's C++ standard library headers are.
            for path in gcc_includes:
                result.extend(["-isystem", path])

        result.extend(EXTRA_ARGS)

    return result


def is_source_argument(
    argument: str,
    source_file: Path,
) -> bool:
    """Return True if argument refers to the given source file."""
    try:
        return Path(argument).resolve() == source_file.resolve()
    except (OSError, RuntimeError):
        return False


def replace_source_argument(
    arguments: list[str],
    source_file: Path,
    new_source: Path,
) -> list[str]:
    """Replace the source file argument in a compilation command."""
    result = list(arguments)

    for i, argument in enumerate(result):
        if is_source_argument(argument, source_file):
            result[i] = new_source.resolve().as_posix()
            return result

    # Compilation databases can contain paths in a different form
    # from Path.resolve(), so fall back to matching the filename.
    source_name = source_file.name

    for i, argument in enumerate(result):
        if Path(argument).name == source_name:
            result[i] = new_source.resolve().as_posix()
            return result

    raise RuntimeError(
        f"Could not find source file '{source_file}' "
        "in compilation command"
    )


def add_include_path(
    arguments: list[str],
    include_path: Path,
) -> list[str]:
    """Add an application include directory to a compilation command."""
    result = list(arguments)

    # Add after the compiler. This is valid for Clang and keeps the
    # generated command easy to inspect.
    result[1:1] = [
        "-I",
        include_path.resolve().as_posix(),
    ]

    return result


def generate_header_tus(
    filtered_entries: list[dict],
    header_root: Path,
    output_path: Path,
) -> list[dict]:
    """
    Generate one .cpp translation unit for every application header.

    For example:

        deps/zpp_lib/zpp_include/zpp_assert.hpp

    becomes:

        build_clang/header_tus/zpp_assert.hpp.cpp

    containing:

        #include "zpp_assert.hpp"

    The generated compilation command is based on an existing
    transformed C++ application command and gets header_root added
    as an include directory.
    """
    header_root = header_root.resolve()
    output_path = output_path.resolve()

    if not header_root.is_dir():
        raise RuntimeError(
            f"Application header directory does not exist: "
            f"{header_root}"
        )

    headers = sorted(
        path
        for path in header_root.rglob("*")
        if path.is_file()
        and path.suffix.lower() in HEADER_SUFFIXES
    )

    if not headers:
        print(f"No application headers found under {header_root}")
        return []

    # Use an existing transformed C++ compilation command as the
    # template. It provides the correct Zephyr target, defines,
    # include paths, C++ standard, etc.
    template = next(
        (
            entry
            for entry in filtered_entries
            if Path(entry.get("file", "")).suffix.lower()
            in CXX_SUFFIXES
        ),
        None,
    )

    if template is None:
        raise RuntimeError(
            "Cannot generate header TUs: "
            "no transformed C++ compilation entry exists"
        )

    template_file = Path(template["file"])

    header_tu_dir = output_path.parent / "header_tus"
    header_tu_dir.mkdir(parents=True, exist_ok=True)

    generated = []

    for header in headers:
        relative_header = header.relative_to(header_root)

        # Preserve the header directory structure.
        #
        # Example:
        #
        #   zpp_include/zpp_rtos/thread.hpp
        #
        # becomes:
        #
        #   header_tus/zpp_rtos/thread.hpp.cpp
        #
        tu_path = (
            header_tu_dir
            / relative_header.parent
            / f"{relative_header.name}.cpp"
        )

        tu_path.parent.mkdir(parents=True, exist_ok=True)

        # Include relative to header_root.
        include_name = relative_header.as_posix()

        tu_path.write_text(
            f'#include "{include_name}"\n',
            encoding="utf-8",
        )

        new_entry = {
            "directory": template["directory"],
            "file": tu_path.resolve().as_posix(),
        }

        if "arguments" in template:
            arguments = replace_source_argument(
                template["arguments"],
                template_file,
                tu_path,
            )

            arguments = add_include_path(
                arguments,
                header_root,
            )

            new_entry["arguments"] = arguments

        elif "command" in template:
            command = template["command"]

            # Replace the original source file with the generated TU.
            original_file = template_file.resolve().as_posix()
            generated_file = tu_path.resolve().as_posix()

            if original_file in command:
                command = command.replace(
                    original_file,
                    generated_file,
                    1,
                )
            elif template["file"] in command:
                command = command.replace(
                    template["file"],
                    generated_file,
                    1,
                )
            else:
                raise RuntimeError(
                    f"Could not find source file '{template_file}' "
                    f"in compilation command"
                )

            # The generated TU is outside the application include tree,
            # so explicitly add the header root.
            command = (
                f'{command.split(" ", 1)[0]} '
                f'-I"{header_root.as_posix()}"'
                + command[len(command.split(" ", 1)[0]):]
            )

            new_entry["command"] = command

        else:
            raise RuntimeError(
                "Compilation entry contains neither "
                "'command' nor 'arguments'"
            )

        generated.append(new_entry)

    print(
        f"Generated {len(generated)} header translation units "
        f"under {header_tu_dir}"
    )

    return generated


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Generate a clang-tidy-compatible compilation database "
            "from a Zephyr GCC compilation database."
        )
    )

    parser.add_argument(
        "input",
        type=Path,
        help="Input Zephyr compile_commands.json",
    )

    parser.add_argument(
        "output",
        type=Path,
        help="Output Clang/clang-tidy compile_commands.json",
    )

    parser.add_argument(
        "header_root",
        type=Path,
        help=(
            "Root directory containing application headers to lint "
            "(.h, .hh, .hpp, .hxx)"
        ),
    )

    args = parser.parse_args()

    input_path = args.input
    output_path = args.output
    header_root = args.header_root

    with input_path.open() as f:
        db = json.load(f)

    gcc_compiler = find_gcc_compiler(db)
    print(f"GCC compiler: {gcc_compiler}")

    gcc_includes = gcc_include_paths(gcc_compiler)

    print("GCC C++ include paths:")
    for path in gcc_includes:
        print(f"  {path}")

    filtered = []

    for entry in db:
        # Only process C++ files — skip C files and assembly.
        file = entry.get("file", "")

        if not file.endswith(CXX_SUFFIXES):
            continue

        new_entry = dict(entry)

        if "command" in new_entry:
            new_entry["command"] = filter_command(
                new_entry["command"],
                gcc_includes,
            )

        if "arguments" in new_entry:
            new_entry["arguments"] = filter_arguments(
                new_entry["arguments"],
                gcc_includes,
            )

        filtered.append(new_entry)

    # Generate application-header translation units.
    header_entries = generate_header_tus(
        filtered,
        header_root,
        output_path,
    )

    filtered.extend(header_entries)

    output_path.parent.mkdir(parents=True, exist_ok=True)

    with output_path.open("w") as f:
        json.dump(filtered, f, indent=2)

    print(
        f"Filtered {len(db)} entries -> "
        f"{len(filtered)} C++ entries "
        f"({len(header_entries)} header TUs)"
    )
    print(f"Compilation database written to {output_path}")


if __name__ == "__main__":
    main()