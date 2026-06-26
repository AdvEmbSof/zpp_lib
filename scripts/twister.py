# scripts/build.py
import argparse
import re
import subprocess
import sys

parser = argparse.ArgumentParser()
parser.add_argument("--root", required=True)
parser.add_argument("--tags", required=True)
parser.add_argument("--map-file", required=False)
parser.add_argument("--qemu", required=False, action="store_true")

args = parser.parse_args()

test_suite_root = args.root
tags = args.tags
map_file = args.map_file
qemu = args.qemu

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
        "--device-testing",
        "--hardware-map",
        map_file
    ]

for t in filter(None, re.split(r"[+,]", tags)):
    cmd.extend(["--tag", f"{t}"])

print(" ".join(cmd))
subprocess.run(cmd, check=True)