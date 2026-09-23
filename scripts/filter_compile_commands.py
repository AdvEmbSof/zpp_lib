#!/usr/bin/env python3
# filter_compile_commands.py
# Generate a clang-tidy-compatible compile_commands.json
# by stripping GCC-specific flags from the Zephyr build output.
#
# Usage:
#   python3 filter_compile_commands.py \
#       build/compile_commands.json \
#       build/compile_commands_clang.json

import json
import sys
import re
import os
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

# Flags to replace — GCC specific flags that have a Clang equivalent
REPLACE_FLAGS = {
    # Keep -mcpu for Clang but strip GCC sub-options it does not understand
}

# Add system C++ headers so <chrono> etc. are found
EXTRA_ARGS = [
    '-DZPP_CLANG_TIDY',
    '--target=arm-none-eabi',
    '-Wno-unknown-warning-option',
    '-Wno-unused-command-line-argument',
]

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

            # Exclude GCC's private compiler headers.
            if "/lib/gcc/" in str(path):
                continue

            if path.exists():
                paths.append(str(path))

    return paths

def find_gcc_compiler(db: list[dict]) -> str:
    """Find the GCC C/C++ compiler used by the compilation database."""
    for entry in db:
        if "arguments" in entry and entry["arguments"]:
            compiler = entry["arguments"][0]
        elif "command" in entry:
            match = re.match(r'\s*(?:"([^"]+)"|(\S+))', entry["command"])
            if not match:
                continue
            compiler = match.group(1) or match.group(2)
        else:
            continue

        compiler_name = Path(compiler).name.lower()

        if compiler_name.endswith(("gcc", "g++")):
            return compiler

    raise RuntimeError(
        "Could not find a GCC compiler in compile_commands.json"
    )

def filter_command(command: str, gcc_includes: list[str]) -> str:
    parts = split_command(command)
    result = []
    skip = False

    for part in parts:
        if skip:
            skip = False
            continue

        if part in STRIP_FLAGS:
            continue

        if any(re.match(p, part) for p in STRIP_PATTERNS):
            if "=" not in part:
                skip = True
            continue

        result.append(part)

    # Replace GCC compiler with Clang.
    if result:
        compiler_name = Path(result[0]).name.lower()
        
        if compiler_name in {"gcc", "g++"} or compiler_name.endswith(("-gcc", "-g++")):
            result[0] = "clang++"
            result.insert(1, "--target=arm-none-eabi")

            # Tell Clang where GCC's C++ standard library headers are
            for path in gcc_includes:
                result.extend(["-isystem", path])

    result.extend(EXTRA_ARGS)

    return " ".join(result)


def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} input.json output.json")
        sys.exit(1)

    with open(sys.argv[1]) as f:
        db = json.load(f)

    gcc_compiler = find_gcc_compiler(db)
    print(f"GCC compiler: {gcc_compiler}")

    gcc_includes = gcc_include_paths(gcc_compiler)

    print("GCC C++ include paths:")
    for path in gcc_includes:
        print(f"  {path}")

    filtered = []
    for entry in db:
        # Only process C++ files — skip C files and assembly
        file = entry.get('file', '')
        if not file.endswith(('.cpp', '.cxx', '.cc')):
            continue

        new_entry = dict(entry)
        if 'command' in new_entry:
            new_entry["command"] = filter_command(
                new_entry["command"],
                gcc_includes,
            )
        if 'arguments' in new_entry:
            # arguments is a list — filter each element
            args   = new_entry['arguments']
            result = []
            skip   = False
            for i, arg in enumerate(args):
                if skip:
                    skip = False
                    continue
                if arg in STRIP_FLAGS:
                    continue
                if any(re.match(p, arg) for p in STRIP_PATTERNS):
                    if '=' not in arg:
                        skip = True
                    continue
                result.append(arg)
            if result and ('gcc' in result[0] or 'g++' in result[0]):
                result[0] = 'clang++'
            result.extend(EXTRA_ARGS)
            new_entry['arguments'] = result

        filtered.append(new_entry)

    with open(sys.argv[2], 'w') as f:
        json.dump(filtered, f, indent=2)

    print(f"Filtered {len(db)} entries -> {len(filtered)} C++ entries")
    print(f"Compilation database written to {sys.argv[2]}")


if __name__ == '__main__':
    main()
    