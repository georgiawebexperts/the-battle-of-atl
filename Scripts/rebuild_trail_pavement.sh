#!/bin/zsh
# Re-bake and reinstall the BeltLine and connector pavement at every authored
# width.
#
# These commands used to be typed by hand, which is how a build note ended up
# saying "re-run the two bake_park_pavement.py passes" without recording what the
# passes were. Each pass below reproduces the committed OBJ files byte for byte,
# so a re-bake that changes anything at all is a real change.
#
# Widths live in Scripts/arcade_trail.py. Run this, then the audit sweep, then
# package. It edits the saved map, so commit the map deliberately.
#
# The installers go through -ExecutePythonScript rather than -run=pythonscript.
# Both load the map, but the python commandlet's navigation rebuild comes back
# unroutable - the same map that answers a 103188 cm Monroe-to-Irwin route
# answers -1 after a commandlet rebuild - so the installers' reachability assert
# fires on a change that is perfectly fine. Cost a diagnosis on 2026-09-18:
# work/probe-nav-commandlet.log versus work/probe-nav-editor.log.
set -eu
PROJECT_ROOT="${0:A:h:h}"
PY="$PROJECT_ROOT/Tools/terrain-venv/bin/python"
EDITOR="/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd"
cd "$PROJECT_ROOT"

echo "== authoring the source networks"
"$PY" Scripts/prepare_eastside_trail.py
"$PY" Scripts/prepare_beltline_connector.py

echo "== baking pavement, wide arcade ribbon"
"$PY" Scripts/bake_park_pavement.py --network eastside-trail-network.json \
  --output-dir EastsideTrail --prefix EastsideTrail \
  --height-profiles eastside-height-profiles.json --include-bridges \
  --subtract-network park-path-network.json
"$PY" Scripts/bake_park_pavement.py --network beltline-connector-network.json \
  --output-dir BeltlineConnector --prefix BeltlineConnector

echo "== baking pavement, narrow realistic ribbon"
"$PY" Scripts/bake_park_pavement.py --network eastside-trail-network-realistic.json \
  --output-dir EastsideTrailRealistic --prefix EastsideTrailRealistic \
  --height-profiles eastside-height-profiles.json --include-bridges \
  --subtract-network park-path-network.json
"$PY" Scripts/bake_park_pavement.py --network beltline-connector-network-realistic.json \
  --output-dir BeltlineConnectorRealistic --prefix BeltlineConnectorRealistic

echo "== baking bridge rails for both offsets"
"$PY" Scripts/bake_eastside_rails.py

echo "== installing into PiedmontWorld"
"$EDITOR" "$PROJECT_ROOT/AuraPlayground.uproject" \
  "-ExecutePythonScript=$PROJECT_ROOT/Scripts/install_eastside_trail.py" \
  -unattended -nosplash -stdout -BattleQuitAfterScript > "$PROJECT_ROOT/work/install-eastside.log" 2>&1
"$EDITOR" "$PROJECT_ROOT/AuraPlayground.uproject" \
  "-ExecutePythonScript=$PROJECT_ROOT/Scripts/install_beltline_connector.py" \
  -unattended -nosplash -stdout -BattleQuitAfterScript > "$PROJECT_ROOT/work/install-connector.log" 2>&1

echo "== done; check work/install-eastside.log and work/install-connector.log"
