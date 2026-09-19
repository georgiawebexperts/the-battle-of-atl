#!/bin/sh
# How close is this chat to the wall that kills long sessions?
#
# Codex re-sends the whole *live window* to the provider on every request. The
# rollout .jsonl under ~/.codex/sessions is an append-only local log, so the
# file grows forever - but the request only carries the messages since the last
# compaction. File size does not kill a chat; window size does.
#
# Measured on this Mac 2026-09-19 (api.deepseek.com):
#   * two sessions died with "413 Payload Too Large" at live windows of
#     50.7 MB and 53.9 MB, both windows 47-58 MB of base64 screenshots
#   * healthy windows ran 0.8-28 MB (about 1.5 hours of real work)
#   * auto-compaction fires when the window reaches model_auto_compact_token_limit
#     (measured: 404375 tokens with the limit at 400000), which is far too late:
#     a screenshot-heavy 320k-token window is already 50 MB
# The fix is a lower limit in ~/.codex/config.toml:
#     model_auto_compact_token_limit = 120000   # compacts at 120k tokens, ~15 MB
#
#   Scripts/session_size.sh              # sessions touched in the last 3 hours
#   Scripts/session_size.sh 01a0b727      # one thread id, any age
#   Scripts/session_size.sh "" 24         # everything touched in the last 24h
#
set -u
ROOT="${HOME}/.codex/sessions"
WALL_MB=50      # 413 observed at 50.7 MB
TIGHT_MB=25     # start banking state here

THREAD="${1:-}"
HOURS="${2:-3}"

if [ -n "$THREAD" ]; then
  FILES=$(find "$ROOT" -name "*${THREAD}*.jsonl" 2>/dev/null)
else
  FILES=$(find "$ROOT" -name "*.jsonl" -newermt "-${HOURS} hours" 2>/dev/null)
fi

if [ -z "$FILES" ]; then
  echo "no rollout files found under $ROOT"
  exit 0
fi

printf "%7s %8s %8s %7s %7s  %-7s %s\n" "FILE" "WINDOW" "IMAGES" "CTX" "IDLE" "VERDICT" "ROLLOUT"

echo "$FILES" | while IFS= read -r f; do
  [ -f "$f" ] || continue
  # one pass: total file size, plus the size of everything after the last
  # compaction marker (that tail is what the provider actually receives)
  stats=$(awk '
    {
      len = length($0) + 1
      total += len
      if ($0 ~ /"type": ?"compacted"/) { win = 0; img = 0; seen = 1 }
      else {
        win += len
        if ($0 ~ /"(input_image|image_url)"/) img += len
      }
    }
    END {
      printf "%.1f %.1f %.1f %d", total/1048576, win/1048576, img/1048576, (seen ? 1 : 0)
    }
  ' "$f")
  total_mb=$(echo "$stats" | cut -d' ' -f1)
  window_mb=$(echo "$stats" | cut -d' ' -f2)
  image_mb=$(echo "$stats" | cut -d' ' -f3)
  compacted=$(echo "$stats" | cut -d' ' -f4)

  # live context tokens: the last token_count event's last_token_usage
  ctx=$(tail -400 "$f" | grep '"type":"token_count"' | tail -1 \
        | sed -n 's/.*"last_token_usage":{"input_tokens":\([0-9]*\).*/\1/p')
  [ -n "${ctx:-}" ] || ctx="?"
  idle=$(( ( $(date +%s) - $(stat -f%m "$f") ) / 60 ))
  if [ "$compacted" = "1" ]; then state="compacted"; else state="never-compacted"; fi

  verdict="ok"
  if [ "$(echo "$window_mb >= $TIGHT_MB" | bc -l)" = "1" ]; then verdict="TIGHT"; fi
  if [ "$(echo "$window_mb >= $WALL_MB" | bc -l)" = "1" ]; then verdict="WALL"; fi

  printf "%6.1fM %7.1fM %7.1fM %7s %6smin  %-7s %s (%s)\n" \
    "$total_mb" "$window_mb" "$image_mb" "$ctx" "$idle" "$verdict" "$(basename "$f")" "$state"
done

echo
echo "WINDOW is what the provider receives - the number that matters (wall at ~${WALL_MB} MB)."
echo "IMAGES is the share of the window that is base64 screenshots; that is what makes it climb fast."
echo "TIGHT: bank state into Brain/History now. WALL: the next request will likely be refused."
