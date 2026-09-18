#!/bin/zsh
# Runs the quick audit sweep for The Battle of ATL and prints one verdict line
# per audit. Logs land in work/ next to the project.
#
#   Scripts/run_quick_audits.sh                       # editor build
#   Scripts/run_quick_audits.sh <packaged-executable> # cooked build
#
# The two long route audits (BattleRoadContactAudit, BattleKrogRiderAudit) are
# deliberately not in here: they drive the whole course and want a quiet machine.
set -u
PROJECT="/Volumes/Adam Assets/Unreal/Projects/AuraPlayground"
EDITOR="/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor"
MAP="/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1"
COMMON=(-game -RenderOffscreen -windowed -ResX=640 -ResY=360 -ForceRes -BattleSkipTutorial -unattended -nosound -stdout)

if [[ $# -ge 1 ]]; then
  BIN="$1"
  PREFIX=("$MAP")
  SUFFIX=(-installed)
else
  BIN="$EDITOR"
  PREFIX=("$PROJECT/AuraPlayground.uproject" "$MAP")
  SUFFIX=()
fi

# name|log label|extra flags
AUDITS=(
  "BattleCurseAudit||"
  "BattleTreeRideAudit||"
  "BattleTreeRideAudit|Realistic|BattleRealTreeAudit"
  "BattleTroubleAudit||"
  "BattleSteeringAudit||"
  "BattleSleeperAudit||"
  "BattleGrassAudit||"
  "BattleBikeCarAudit||"
  "BattleMurderKAudit||"
  "BattlePickupAudit||"
  "BattleSpiritAudit||"
  "BattleKrogCrashAudit||"
  "BattleDuckAudit||"
)

FAILED=0
for ENTRY in "${AUDITS[@]}"; do
  NAME="${ENTRY%%|*}"
  REST="${ENTRY#*|}"
  TAG="${REST%%|*}"
  FLAGS="${REST#*|}"
  LABEL="${TAG:+$TAG-}$NAME"
  EXTRA=()
  [[ -n "$FLAGS" ]] && EXTRA=("-$FLAGS")
  LOG="$PROJECT/work/$LABEL.log"
  "$BIN" "${PREFIX[@]}" "${COMMON[@]}" "-$NAME" "${EXTRA[@]}" "${SUFFIX[@]}" > "$LOG" 2>&1
  VERDICT=$(grep -h "\"passed\":" "$LOG" | tail -1)
  if [[ "$VERDICT" == *'"passed":true'* ]]; then
    printf '%s\n' "-- $LABEL: pass"
  else
    printf '%s\n' "-- $LABEL: FAIL ${VERDICT:-<no verdict>}"
    FAILED=1
  fi
done
exit $FAILED
