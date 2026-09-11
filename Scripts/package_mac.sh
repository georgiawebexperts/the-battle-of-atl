#!/bin/zsh
set -eu
PROJECT_ROOT="${0:A:h:h}"
ENGINE_ROOT="/Volumes/Adam Assets/Unreal/UE_5.8"
ARCHIVE_ROOT="/Volumes/Adam Assets/Unreal/Builds/BattleForTheA"
mkdir -p "$PROJECT_ROOT/work" "$ARCHIVE_ROOT"
"$ENGINE_ROOT/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun \
 -project="$PROJECT_ROOT/AuraPlayground.uproject" -noP4 \
 -platform=Mac -clientconfig=Development -build -cook \
 -map=/Game/PiedmontRide/Maps/PiedmontWorld+/Game/BattleForTheA/Maps/ArcadeBikeLab \
 -stage -pak -archive -archivedirectory="$ARCHIVE_ROOT" \
 -utf8output -unattended > "$PROJECT_ROOT/work/mac-package.log" 2>&1
