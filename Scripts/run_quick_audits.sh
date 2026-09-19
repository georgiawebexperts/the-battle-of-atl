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
  # The Krog Street Tunnel: the bore's length, that its hazard stations landed
  # on the tunnel floor rather than on the deck above, and that a pothole can
  # actually throw the rider. Outside the sweep is how three audits stayed red
  # for several builds, so this one goes in with the work it covers.
  "BattleTunnelHazardAudit||"
  "BattleTrailModeAudit||"
  "BattleSkaterAudit||"
  "BattleDroneAudit||"
  "BattleFrisbeeAudit||"
  "BattlePanicAudit||"
  "BattleSpareBikeAudit||"
  "BattleTutorialAudit||NoSkipTutorial"
  "BattleMarketImpactAudit||NoSkipTutorial"
  "BattleScooterTrafficAudit||BattleFurnitureAudit"
  # The music audit needs the audio device: with the sweep's -nosound the wave
  # never reports Playing, so it fails on the flag rather than on the game. It
  # passes with audio (tracks 2, cycle off-song1-song2-off, preference restored).
  "BattleMusicAudit||WithAudio"
)

FAILED=0
FLAKY=0

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
  # BATTLE_SWEEP_ONLY=Name,Name runs just those, for verifying one entry without
  # paying for twenty-nine launches.
  if [[ -n "${BATTLE_SWEEP_ONLY:-}" && ",${BATTLE_SWEEP_ONLY}," != *",$NAME,"* ]]; then continue; fi
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
      elif [[ "$TOKEN" == "WithAudio" ]]; then
        # Same trap as NoSkipTutorial, one flag over: an audit that tests audio
        # cannot be run with the sweep's -nosound.
        RUN=("${RUN[@]:#-nosound}")
      else
        EXTRA+=("-$TOKEN")
      fi
    done
  fi
  LOG="$PROJECT/work/$LABEL.log"
  AUDIT_TIMED_OUT=0
  # The next process starts while the last one is still tearing down, and a
  # sweep of twenty-nine driven audits has now produced four false reds on this
  # machine that pass standalone on the same binary on the same day
  # (BattleTunnelHazardAudit, BattleSkaterAudit, BattleFrisbeeAudit,
  # BattleSleeperAudit and BattleSpareBikeAudit). Letting the machine settle is
  # the cheap half of the fix.
  sleep "${BATTLE_SWEEP_SETTLE:-3}"
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
    # The honest half: re-run the audit once on its own, and report both runs
    # rather than only the worse one. A gate that fails in a sweep and passes
    # alone is telling us about the sweep, and saying so is the point.
    printf '%s\n' "-- $LABEL: FAIL ${VERDICT:-<no verdict>}; re-running alone after ${BATTLE_SWEEP_RETRY_SETTLE:-8}s"
    sleep "${BATTLE_SWEEP_RETRY_SETTLE:-8}"
    AUDIT_TIMED_OUT=0
    LOG="$PROJECT/work/$LABEL-retry.log"
    LaunchAudit
    RETRY=$(grep -h "\"passed\":" "$LOG" | tail -1)
    if [[ "$RETRY" == *'"passed":true'* ]]; then
      printf '%s\n' "-- $LABEL: fail in sweep, PASS alone (see $LABEL.log and $LABEL-retry.log)"
      FLAKY=1
    else
      printf '%s\n' "-- $LABEL: FAIL twice ${RETRY:-<no verdict>}"
      FAILED=1
    fi
  fi
done
[[ ${FLAKY:-0} -eq 1 ]] && printf '%s\n' "-- note: at least one audit failed in the sweep and passed alone; both logs kept"
exit $FAILED
