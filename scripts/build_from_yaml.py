import subprocess
import os
import yaml
import argparse
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
build_script = SCRIPT_DIR / "build.py"

parser = argparse.ArgumentParser()
parser.add_argument("--yaml", required=True)
parser.add_argument("--app", required=False, help="If specified, only build this app from the config file")
parser.add_argument("--board", required=False, help="If specified, only build this board from the config file")
parser.add_argument("--configs", required=False, help="If specified, only build this configs from the config file")

args = parser.parse_args()

yaml_file = args.yaml
argsapp = args.app
if argsapp:
    argsapp = argsapp.rstrip("/\\")
board = args.board
configs = args.configs

with open(yaml_file, "r") as f:
    yaml_data = yaml.safe_load(f)
    
for app in yaml_data["applications"]:
    if argsapp and app["app"] != argsapp:
        continue

    configs_dir = app.get("configs_dir", None)
    print(f"Building app '{app['app']}' with configs_dir='{configs_dir}'")
    for board in app["boards"]:
        board_name = board["board"]
        if args.board and board["board"] != args.board:
            continue
    
        shield = board.get("shield")
        
        for configs in app["configs"]:
            if args.configs and configs != args.configs:
                continue

            cmd = [
                "python",
                str(build_script),
                "--app", app["app"],
                "--configs", configs,
                "--board", board_name
            ]

            cmd.append("--pristine")

            if configs_dir:
                cmd.extend(["--configs_dir", configs_dir])

            if shield:
                cmd.extend(["--shield", shield])
            
            print(" ".join(cmd))
            subprocess.run(cmd, check=True)

