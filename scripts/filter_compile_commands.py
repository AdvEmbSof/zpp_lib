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

# Exact flags to remove
STRIP_FLAGS = {
    '-fno-reorder-functions',
    '-mpreferred-stack-boundary=2',
    '-fno-freestanding',
    '-fno-defer-pop',
    '-fsanitize=bounds-strict'
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
    '-Wno-unknown-warning-option',
    '-Wno-unused-command-line-argument',
]


def filter_command(command: str) -> str:
    parts  = command.split()
    result = []
    skip   = False

    for i, part in enumerate(parts):
        if skip:
            skip = False
            continue

        # Strip exact flags
        if part in STRIP_FLAGS:
            continue

        # Strip pattern flags
        if any(re.match(p, part) for p in STRIP_PATTERNS):
            # Also skip the next token if it is the value
            if not '=' in part:
                skip = True
            continue

        result.append(part)

    # Replace compiler with clang++ for C++ files
    if result and ('gcc' in result[0] or 'g++' in result[0]):
        result[0] = 'clang++'

    result.extend(EXTRA_ARGS)
    return ' '.join(result)


def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} input.json output.json")
        sys.exit(1)

    with open(sys.argv[1]) as f:
        db = json.load(f)

    filtered = []
    for entry in db:
        # Only process C++ files — skip C files and assembly
        file = entry.get('file', '')
        if not file.endswith(('.cpp', '.cxx', '.cc')):
            continue

        new_entry = dict(entry)
        if 'command' in new_entry:
            new_entry['command'] = filter_command(new_entry['command'])
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
    