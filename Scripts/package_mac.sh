#!/bin/zsh
set -eu
PROJECT_ROOT="${0:A:h:h}"
ENGINE_ROOT="/Volumes/Adam Assets/Unreal/UE_5.8"
ARCHIVE_ROOT="/Volumes/Adam Assets/Unreal/Builds/BattleForTheA"
BUILD_CONFIG="${BATTLE_MAC_CONFIG:-Development}"
mkdir -p "$PROJECT_ROOT/work" "$ARCHIVE_ROOT"
"$ENGINE_ROOT/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun \
 -project="$PROJECT_ROOT/AuraPlayground.uproject" -noP4 \
 -platform=Mac -clientconfig="$BUILD_CONFIG" -build -cook \
 -map=/Game/PiedmontRide/Maps/PiedmontWorld \
 -CookDir="$PROJECT_ROOT/Content/CitySampleCrowd+$PROJECT_ROOT/Content/BattleRetarget" \
 -stage -pak -archive -archivedirectory="$ARCHIVE_ROOT" \
 -utf8output -unattended > "$PROJECT_ROOT/work/mac-package.log" 2>&1
