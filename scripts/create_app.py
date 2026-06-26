from pathlib import Path
import shutil
import subprocess
import sys

app = sys.argv[1]

print(app)

if Path(app).is_dir():
    print(f'Directory {app} already exists!')
    sys.exit(1)

subprocess.run([
    'git', 'clone',
    '--depth', '1',
    'https://github.com/AdvEmbSof/template_application.git',
    app,
], check=True)

shutil.rmtree(Path(app) / '.git')