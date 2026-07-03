# scripts/build.py
import argparse
import re
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument("--root", required=True)
parser.add_argument("--tags", required=True)
parser.add_argument("--board", required=True)
parser.add_argument("--map-file", required=False)

args = parser.parse_args()

test_suite_root = args.root
tags = args.tags
map_file = args.map_file
board = args.board
qemu = args.board=="qemu_x86"

if qemu:
    print(f"Testing {test_suite_root} for QEMU with tags '{tags}'")
    cmd = [
        "west",
        "twister",
        "-T",
        f"{test_suite_root}",
        "-p",
        "qemu_x86"
    ]
else:
    print(f"Testing {test_suite_root} with tags '{tags}'")
    cmd = [
        "west",
        "twister",
        "-T",
        f"{test_suite_root}",
        "-p",
        f"{board}",
        "--device-testing",
        "--hardware-map",
        map_file
    ]

for t in filter(None, re.split(r"[+,]", tags)):
    cmd.extend(["--tag", f"{t}"])

print(" ".join(cmd))
subprocess.run(cmd, check=True)