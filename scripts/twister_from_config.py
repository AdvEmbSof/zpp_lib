import subprocess
import yaml
import argparse
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
twister_script = SCRIPT_DIR / "twister.py"

parser = argparse.ArgumentParser()
parser.add_argument("--config", required=True)
parser.add_argument("--board", required=False, help="If specified, only test this board from the config file")
parser.add_argument("--map-file", required=False, help="If specified, only build this spec from the config file")

args = parser.parse_args()

config_file = args.config
board= args.board
map_file= args.map_file

with open(config_file, "r") as f:
    config = yaml.safe_load(f)
    
for test in config["tests"]:
    tags = ",".join(test.get("tags", []))

    print(f"Running twister with root '{test["root"]}'")
    for board in test["boards"]:
        board_name = board["board"]
        if args.board and board["board"] != args.board:
            continue
        
        cmd = [
            "python",
            str(twister_script),
            "--root", 
            test["root"],
            "--tags", 
            tags,
            "--board", 
            board_name
        ]

        config_map_file = board.get("map_file", "")
        if config_map_file != "" and map_file is None:
            cmd.extend(["--map-file", config_map_file])
        elif map_file:
            cmd.extend(["--map-file", map_file]) 
            
        print(" ".join(cmd))
        subprocess.run(cmd, check=True)

