import subprocess
import os
import yaml
import argparse
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
build_script = SCRIPT_DIR / "build.py"

parser = argparse.ArgumentParser()
parser.add_argument("--config", required=True)
parser.add_argument("--app", required=False, help="If specified, only build this app from the config file")
parser.add_argument("--board", required=False, help="If specified, only build this board from the config file")
parser.add_argument("--spec", required=False, help="If specified, only build this spec from the config file")
parser.add_argument("--pristine", action="store_true", help="If specified, build with --pristine")

args = parser.parse_args()

config_file = args.config
app= args.app
board= args.board
spec= args.spec
pristine= args.pristine

with open(config_file, "r") as f:
    config = yaml.safe_load(f)
    
for app in config["applications"]:
    if args.app and app["app"] != args.app:
        continue

    config_dir = app.get("config_dir", None)
    print(f"Building app '{app['app']}' with config_dir='{config_dir}'")
    for board in app["boards"]:
        board_name = board["board"]
        if args.board and board["board"] != args.board:
            continue
    
        shield = board.get("shield")
        
        for spec in app["specs"]:
            if args.spec and spec != args.spec:
                continue

            cmd = [
                "python",
                str(build_script),
                "--app", app["app"],
                "--spec", spec,
                "--board", board_name
            ]

            if pristine:
                cmd.append("--pristine")

            if config_dir:
                cmd.extend(["--config_dir", config_dir])

            if shield:
                cmd.extend(["--shield", shield])

            print(" ".join(cmd))
            subprocess.run(cmd, check=True)

