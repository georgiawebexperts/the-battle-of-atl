#!/bin/zsh
# Render the spectral bear review and collect the pictures.
#
#   Scripts/review_spirit_bear.sh <executable> [output dir]
#
#   Scripts/review_spirit_bear.sh \
#     "$PWD/Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground"
#   Scripts/review_spirit_bear.sh \
#     "/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd"
#
# Why this is a script and not two lines of shell. It is the only way to see what
# a *packaged* build looks like, and until 2026-09-19 nobody had ever seen one:
# the review wrote two PNGs from the editor for a whole day and nothing at all
# from a cooked build. Two different reasons, both invisible if you only ever run
# one of the two.
#
#   1. The editor's pictures were not written by the game. The editor opens with
#      "Requested channels: 'cpu,gpu,frame,log,bookmark,screenshot,region'", so
#      the trace screenshot channel serviced FScreenshotRequest. A cooked build
#      has no trace server ("UTS: The Unreal Trace Server binary is not
#      available") and silently wrote nothing.
#   2. A packaged Mac build runs sandboxed, so even a direct render-target export
#      cannot write into this project - it fails with "FileWrite failed to
#      create" and lands in the container instead.
#
# The game now captures in-process and falls back to Saved/SpiritReview inside
# its container, and this script copies the results back out.
set -u
ROOT="${0:A:h:h}"
BIN="${1:-}"
OUT="${2:-$ROOT/work/spirit-review}"

if [[ -z "$BIN" || ! -e "$BIN" ]]; then
  print -u2 "usage: Scripts/review_spirit_bear.sh <executable> [output dir]"
  exit 2
fi

MAP="/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1"
CONTAINER="$HOME/Library/Containers/com.webexperts.battleofatl/Data/Library/Application Support/Epic/AuraPlayground/Saved/SpiritReview"
LOG="$ROOT/work/$(basename "$OUT").log"
mkdir -p "$OUT"

PREFIX=("$MAP")
SUFFIX=(-installed)
if [[ "$BIN" == *UnrealEditor* ]]; then
  PREFIX=("$ROOT/AuraPlayground.uproject" "$MAP")
  SUFFIX=()
fi

"$BIN" "${PREFIX[@]}" -game -RenderOffscreen -windowed -ResX=1280 -ResY=720 -ForceRes \
  -BattleSkipTutorial -BattleSpiritReview "-BattleSpiritReviewDir=$OUT" \
  "${SUFFIX[@]}" -unattended -nosound -stdout > "$LOG" 2>&1

# The sandbox copy, and it is only needed for the packaged build.
if [[ -d "$CONTAINER" ]]; then
  for F in "$CONTAINER"/*.png; do
    [[ -e "$F" ]] && cp -f "$F" "$OUT/"
  done
fi

grep -h "BattleSpiritReview: capture\|BattleSpiritReview: state" "$LOG" | tail -4
COUNT=$(ls "$OUT"/*.png 2>/dev/null | wc -l | tr -d ' ')
print "REVIEW: $COUNT picture(s) in $OUT"
[[ "$COUNT" -ge 2 ]]
