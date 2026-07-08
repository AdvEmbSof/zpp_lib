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
parser.add_argument("--configs", required=True)
parser.add_argument("--board", required=True)
parser.add_argument("--configs-dir", required=False, default=WORKSPACE_DIR + "/deps/zpp_lib/configs")
parser.add_argument("--shield", required=False)
parser.add_argument("--app-config", required=False)
parser.add_argument("--pristine", action="store_true")

args = parser.parse_args()

app = args.app
configs = args.configs   
pristine = args.pristine
board = args.board
configs_dir = args.configs_dir
shield = args.shield
app_config = args.app_config
qemu = args.board=="qemu_x86"
native_sim = args.board=="native_sim"

# Build the initial command based on the board type
if qemu or native_sim:
    print(f"Building {app} for qemu_x86 or native_sim with configs '{configs}' and pristine='{pristine}'")
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
    print(f"Building {app} for board '{board}' with configs '{configs}' and pristine='{pristine}'")
    cmd = [
        "west",
        "build",
        app,
        "-b",
        board
    ]
    if shield:
      cmd.extend(["--shield", shield])
    
# Add pristine argument if configured
if pristine:
    cmd.append("--pristine")

# Add extra overlay file for qemu, native_sim, or nrf5340dk_nrf5340_cpuapp
extra_overlay_file = ""
if qemu:
    extra_overlay_file = configs_dir + "/boards/qemu_x86.overlay"
elif native_sim:
    extra_overlay_file = configs_dir + "/boards/native_sim.overlay"
elif board == "nrf5340dk/nrf5340/cpuapp":
    extra_overlay_file = configs_dir + "/boards/nrf5340dk_nrf5340_cpuapp.overlay"
cmd.extend(["--extra-dtc-overlay",
            extra_overlay_file])

# Build and add configuration files
conf_files = [configs_dir + "/prj.conf"]
for c in filter(None, re.split(r"[+,]", configs)):
    if qemu or native_sim:
        # for gpio/sensor/display on qemu or native_sim, we need to use the emulated config only            
        if c == "gpio" or c == "sensor" or c == "display":
            conf_file = configs_dir + f"/prj_{c}_emul.conf"
            conf_files.append(conf_file)
        else:
            conf_file = configs_dir + f"/prj_{c}.conf"
            conf_files.append(conf_file)
    else:
       conf_file = configs_dir + f"/prj_{c}.conf"
       conf_files.append(conf_file)

if app_config:
    conf_files.append(app_config)
    
print(f"Using conf files: {conf_files}")
conf_files_param = f'-DCONF_FILE="{";".join(conf_files)}"'
cmd.extend([
    "--",
    f"-DCONF_FILE={';'.join(conf_files)}"
])

# For native_sim, export compile commands for clang-tidy
if native_sim:
    cmd.append("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")

print(" ".join(cmd))
subprocess.run(cmd, check=True)