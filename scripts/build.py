# scripts/build.py
import re
import subprocess
import sys
from tkinter import ON
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--app", required=True)
parser.add_argument("--specs", required=True)
parser.add_argument("--clean", required=True)
parser.add_argument("--board", required=True)
args = parser.parse_args()
  
app = args.app
specs = args.specs
clean = args.clean=="yes"
board = args.board
qemu = args.board=="qemu_x86"
native_sim = args.board=="native_sim"

if qemu or native_sim:
    print(f"Building {app} for qemu_x86 or native_sim with spec '{specs}' and clean='{clean}'")
    cmd = [
        "west",
        "build",
        app,
        "-b",         
    ]
    if qemu:
        cmd.append("qemu_x86")
    else:
        cmd.append("native_sim")
else:
    print(f"Building {app} for board '{board}' with spec '{specs}' and clean='{clean}'")
    cmd = [
        "west",
        "build",
        app,
        "-b",
        board,
        "--shield",
        "adafruit_2_8_tft_touch_v2",
    ]

if clean != "no":
    cmd.append("--pristine")

for s in filter(None, re.split(r"[+,]", specs)):
    cmd.extend(["--extra-conf", f"prj_{s}.conf"])
    if qemu or native_sim:
        if s == "gpio":
            cmd.extend(["--extra-conf", "prj_gpio_emul.conf"])        
        elif s == "sensor":
            cmd.extend(["--extra-conf", "prj_sensor_emul.conf"])
        
if native_sim:
    cmd.append("--")
    cmd.append("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")

print(" ".join(cmd))
subprocess.run(cmd, check=True)