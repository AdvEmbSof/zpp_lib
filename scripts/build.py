# scripts/build.py
import re
import subprocess
from tkinter import ON
import argparse
import subprocess

WORKSPACE_DIR = subprocess.check_output(
    ["west", "topdir"],
    text=True,
).strip()

parser = argparse.ArgumentParser()
parser.add_argument("--app", required=True)
parser.add_argument("--spec", required=True)
parser.add_argument("--board", required=True)
parser.add_argument("--config_dir", required=False, default=WORKSPACE_DIR + "/deps/zpp_lib/configs")
parser.add_argument("--shield", required=False)
parser.add_argument("--pristine", action="store_true")

args = parser.parse_args()

app = args.app
spec = args.spec   
pristine = args.pristine
board = args.board
config_dir = args.config_dir
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

conf_files = [config_dir + "/prj.conf"]
for s in filter(None, re.split(r"[+,]", spec)):
    if qemu or native_sim:
        # for gpio/sensor/display on qemu or native_sim, we need to use the emulated config only            
        if s == "gpio" or s == "sensor" or s == "display":
            conf_file = config_dir + f"/prj_{s}_emul.conf"
            conf_files.append(conf_file)
        else:
            conf_file = config_dir + f"/prj_{s}.conf"
            conf_files.append(conf_file)
    else:
       conf_file = config_dir + f"/prj_{s}.conf"
       conf_files.append(conf_file)

print(f"Using conf files: {conf_files}")
conf_files_param = f'-DCONF_FILE="{";".join(conf_files)}"'
cmd.extend([
    "--",
    f"-DCONF_FILE={';'.join(conf_files)}"
])

if native_sim:
    cmd.append("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")

print(" ".join(cmd))
subprocess.run(cmd, check=True)