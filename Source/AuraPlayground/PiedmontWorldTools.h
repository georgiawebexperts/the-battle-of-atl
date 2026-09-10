#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PiedmontWorldTools.generated.h"
class ALandscape;
UCLASS()
class AURAPLAYGROUND_API UPiedmontWorldTools : public UBlueprintFunctionLibrary {
 GENERATED_BODY()
public:
 /** Import measured, little-endian R16 samples into a real Unreal Landscape. Editor only. */
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|World")
 static ALandscape* ImportMeasuredLandscape(const FString& Filename,int32 Width,int32 Height,FVector Location,FVector Scale);
};
