"""Reapply review art after geometry validation, then render the result."""
import json,runpy
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir())
assert json.loads((root/'Tests/Results/2026-09-12-krog-road-support.json').read_text())['passed']
for script in ['survey_krog_floor_width.py','import_krog_markings.py','create_krog_street_art.py','light_krog_tunnel.py','render_krog_roads.py']:
 runpy.run_path(str(root/'Scripts'/script),run_name='__main__')
