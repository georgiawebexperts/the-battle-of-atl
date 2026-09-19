#!/bin/zsh
# BattleRoadLaneAudit drives two authored opposing car lanes to their endpoints.
#
# It is not in the 29-audit sweep and it cannot be, for two reasons that cost an
# afternoon to pin down on 2026-09-19:
#
#   1. The actors it looks for are tagged TenthCarLaneReview (or
#      MonroeCarLaneReview with -BattleMonroeLanes) and they live in a review
#      map. Run it against PiedmontWorld and it always answers "Expected
#      opposing lane candidates", because the main world has no lane-review
#      actors at all - a red that says nothing about the game.
#   2. That review map is deliberately not cooked into the share (only
#      PiedmontWorld is), so a packaged build cannot load it: the run dies with
#      "Failed to load package ... PiedmontCarLaneReview".
#
# So it runs from the editor, against the review map:
#
#     Scripts/run_road_lane_audit.sh
#
# Last known good: 2 cars, total_distance_cm 81399.055, "Both opposing road
# lanes traversed with wheel support and endpoint stops".
set -u
PROJECT="/Volumes/Adam Assets/Unreal/Projects/AuraPlayground"
EDITOR="/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor"
MAP="${1:-/Game/PiedmontRide/Maps/PiedmontCarLaneReview}"
LOG="$PROJECT/work/road-lane-audit.log"

"$EDITOR" "$PROJECT/AuraPlayground.uproject" "$MAP?Difficulty=Easy?AutoStart=1" \
  -game -RenderOffscreen -windowed -ResX=640 -ResY=360 -ForceRes \
  -BattleSkipTutorial -unattended -nosound -stdout -BattleRoadLaneAudit > "$LOG" 2>&1

VERDICT=$(grep -h "RoadLaneAudit: " "$LOG" | tail -1)
if [[ "$VERDICT" == *'"passed":true'* ]]; then
  printf '%s\n' "road lanes: pass  $VERDICT"
  exit 0
fi
printf '%s\n' "road lanes: FAIL ${VERDICT:-<no verdict>}  (log: $LOG)"
exit 1
