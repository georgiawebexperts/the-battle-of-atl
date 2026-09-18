#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BattleTrailMode.generated.h"

/**
 * The BeltLine and the park-to-BeltLine connector are authored twice.
 *
 * Elliott, 2026-09-18: "i wonder if you are doing the arcade mode if the beltlien
 * is wide and if you are doing the pro mode or realistic mode it was like it was
 * before?" The widened 4.2 m trail stays the arcade one; realistic rides the
 * original 3.2 m. The surface is baked geometry rather than a runtime parameter,
 * so both ribbons live in the level and exactly one of them is visible and
 * colliding at a time. The splines stay single: WidthCm is the number the
 * BeltLine grass rule reads to find the pavement edge, so it is rewritten on the
 * same toggle rather than being duplicated.
 *
 * The widths are authored in Scripts/arcade_trail.py and baked by that pipeline.
 * BattleTrailModeAudit measures the actual pavement edge in both modes, so a
 * drift between this file and the bake shows up as a failure rather than as a
 * rider on grass that counts as pavement.
 */
UCLASS()
class AURAPLAYGROUND_API UBattleTrailMode : public UBlueprintFunctionLibrary {
 GENERATED_BODY()
public:
 /** Game centimetres of pavement either side of the centreline, per mode. */
 static constexpr float ArcadeWidthCm=420.f;
 static constexpr float RealisticWidthCm=320.f;

 /** Show the ribbon and spline width that belong to the given handling mode. */
 UFUNCTION(BlueprintCallable,Category="Battle|Trail")
 static void Apply(UObject* WorldContextObject,bool bRealistic);
 static void ApplyToWorld(UWorld* World,bool bRealistic);
};
