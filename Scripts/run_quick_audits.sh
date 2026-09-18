#!/bin/zsh
# Runs the quick audit sweep for The Battle of ATL and prints one verdict line
# per audit. Logs land in work/ next to the project.
#
#   Scripts/run_quick_audits.sh                       # editor build
#   Scripts/run_quick_audits.sh <packaged-executable> # cooked build
#
# The two long route audits (BattleRoadContactAudit, BattleKrogRiderAudit) are
# deliberately not in here: they drive the whole course and want a quiet machine.
#
# The three fixtures at the end are slow - each drives a fixture for 30 to 50
# seconds - but they are in here now because being outside the sweep is exactly
# how BattleSkaterAudit, BattleDroneAudit and BattleFrisbeeAudit stayed red
# without anyone noticing. They cover promises the player can see: a body hit in
# arcade leaves you up and in realistic throws you off, a drone that dives
# connects, and the park lawn does not cost you your speed.
#
# One more trap, found 2026-09-18: COMMON passes -BattleSkipTutorial, so any
# audit that covers tutorial-time behaviour was being run with the feature it
# tests switched off. BattleTutorialAudit failed on "Practice timer ran before
# gateway" and BattleMarketImpactAudit hung forever - its fence destroys itself
# the moment bTutorialActive is false - purely because of that flag. An entry
# whose flags include the token NoSkipTutorial drops it for that audit only.
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
  "BattleSpeedAudit||"
  "BattleTimeAudit||"
  "BattleDiscAudit||"
  "BattleMeleeAudit||"
  # The two-keys-at-once complaint: holds W, then W+D, then W+A on foot and
  # measures the direction actually travelled against where the camera looks.
  "BattleDiagonalAudit||"
  # End-to-end guard on the ending: walks the whole authored course and requires
  # the win to commit at the patio. This is the audit that would have caught a
  # rider reaching the party with no result screen.
  "BattlePatioAudit||"
  "BattleTrailModeAudit||"
  "BattleSkaterAudit||"
  "BattleDroneAudit||"
  "BattleFrisbeeAudit||"
  "BattlePanicAudit||"
  "BattleSpareBikeAudit||"
  "BattleTutorialAudit||NoSkipTutorial"
  "BattleMarketImpactAudit||NoSkipTutorial"
  "BattleScooterTrafficAudit||BattleFurnitureAudit"
)

FAILED=0

# Launch one audit with a hard timeout. An audit that returns its verdict and
# then fails to exit used to hang the whole sweep with no output at all:
# BattleTutorialAudit did exactly that on build 123 - it logged "passed":true,
# printed "PreExit Game" and then sat at 2% CPU for eighteen minutes while the
# sweep waited on nothing. The verdict is still read after the kill, so an audit
# that finished and lingered is scored on its own result rather than on the exit.
AUDIT_TIMEOUT="${BATTLE_AUDIT_TIMEOUT:-420}"
LaunchAudit() {
  "$BIN" "${PREFIX[@]}" "${RUN[@]}" "-$NAME" "${EXTRA[@]}" "${SUFFIX[@]}" > "$LOG" 2>&1 &
  local pid=$! waited=0
  while kill -0 $pid 2>/dev/null && (( waited < AUDIT_TIMEOUT )); do sleep 1; (( waited++ )); done
  if kill -0 $pid 2>/dev/null; then kill -9 $pid 2>/dev/null; AUDIT_TIMED_OUT=1; else AUDIT_TIMED_OUT=0; fi
}

for ENTRY in "${AUDITS[@]}"; do
  NAME="${ENTRY%%|*}"
  REST="${ENTRY#*|}"
  TAG="${REST%%|*}"
  FLAGS="${REST#*|}"
  LABEL="${TAG:+$TAG-}$NAME"
  EXTRA=()
  RUN=("${COMMON[@]}")
  if [[ -n "$FLAGS" ]]; then
    for TOKEN in ${(s:,:)FLAGS}; do
      if [[ "$TOKEN" == "NoSkipTutorial" ]]; then
        RUN=("${RUN[@]:#-BattleSkipTutorial}")
      else
        EXTRA+=("-$TOKEN")
      fi
    done
  fi
  LOG="$PROJECT/work/$LABEL.log"
  AUDIT_TIMED_OUT=0
  LaunchAudit
  VERDICT=$(grep -h "\"passed\":" "$LOG" | tail -1)
  # An empty log means the app never got going; retry once before calling it a
  # failure so a launch hiccup is not reported as a broken audit.
  if [[ -z "$VERDICT" ]]; then
    [[ $AUDIT_TIMED_OUT -eq 1 ]] && printf '%s\n' "-- $LABEL: no verdict inside ${AUDIT_TIMEOUT}s; retrying once"
    AUDIT_TIMED_OUT=0
    LaunchAudit
    VERDICT=$(grep -h "\"passed\":" "$LOG" | tail -1)
  fi
  if [[ "$VERDICT" == *'"passed":true'* ]]; then
    printf '%s\n' "-- $LABEL: pass"
  else
    printf '%s\n' "-- $LABEL: FAIL ${VERDICT:-<no verdict>}"
    FAILED=1
  fi
done
exit $FAILED
