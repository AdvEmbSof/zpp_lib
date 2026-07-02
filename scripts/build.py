# scripts/build.py
import re
import subprocess
from tkinter import ON
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--app", required=True)
parser.add_argument("--spec", required=True)
parser.add_argument("--board", required=True)
parser.add_argument("--shield", required=False)
parser.add_argument("--pristine", action="store_true")

args = parser.parse_args()

app = args.app
spec = args.spec   
pristine = args.pristine
board = args.board
shield = args.shield
qemu = args.board=="qemu_x86"
native_sim = args.board=="native_sim"

if qemu or native_sim:
    print(f"Building {app} for qemu_x86 or native_sim with spec '{spec}' and pristine='{pristine}'")
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
    print(f"Building {app} for board '{board}' with spec '{spec}' and pristine='{pristine}'")
    cmd = [
        "west",
        "build",
        app,
        "-b",
        board
    ]
    if shield:
      cmd.extend(["--shield", shield])
    
if pristine:
    cmd.append("--pristine")

for s in filter(None, re.split(r"[+,]", spec)):
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