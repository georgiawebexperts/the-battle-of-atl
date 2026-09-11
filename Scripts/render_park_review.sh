#!/bin/zsh
set -eu
PROJECT_ROOT="${0:A:h:h}"
mkdir -p "$PROJECT_ROOT/work/scene-review"
'/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd' \
 "$PROJECT_ROOT/AuraPlayground.uproject" -run=pythonscript \
 "-script=$PROJECT_ROOT/Scripts/render_park_review.py" \
 -AllowCommandletRendering -NoTextureStreaming -RCWebControlDisable \
 -unattended -nosound -stdout > "$PROJECT_ROOT/work/scene-review/render.log" 2>&1
