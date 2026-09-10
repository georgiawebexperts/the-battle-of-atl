#include "PiedmontWorldTools.h"
#include "Landscape.h"
#include "Misc/FileHelper.h"
#if WITH_EDITOR
#include "Editor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "WaterBodyActor.h"
#include "WaterBodyComponent.h"
#include "Engine/World.h"
#endif
ALandscape* UPiedmontWorldTools::ImportMeasuredLandscape(const FString& Filename,int32 Width,int32 Height,FVector Location,FVector Scale){
#if WITH_EDITOR
 if(!GEditor||Width<127||Height<127||Width>4097||Height>4097||(Width-1)%126||(Height-1)%126)return nullptr;
 TArray<uint8> Bytes;
 if(!FFileHelper::LoadFileToArray(Bytes,*Filename)||Bytes.Num()!=Width*Height*2)return nullptr;
 TArray<uint16> Samples;Samples.SetNumUninitialized(Width*Height);
 for(int32 I=0;I<Samples.Num();++I)Samples[I]=uint16(Bytes[I*2])|(uint16(Bytes[I*2+1])<<8);
 UWorld* World=GEditor->GetEditorWorldContext().World();if(!World)return nullptr;
 ALandscape* Land=World->SpawnActor<ALandscape>(Location,FRotator::ZeroRotator);if(!Land)return nullptr;
 Land->SetActorLabel(TEXT("USGS measured Atlanta terrain — 1 to 3 scale"));Land->SetActorScale3D(Scale);Land->Tags.Add(TEXT("RideGrass"));
 TMap<FGuid,TArray<uint16>> Heights;Heights.Add(FGuid(),MoveTemp(Samples));
 TMap<FGuid,TArray<FLandscapeImportLayerInfo>> Layers;Layers.Add(FGuid(),TArray<FLandscapeImportLayerInfo>());
 Land->Import(FGuid::NewGuid(),0,0,Width-1,Height-1,2,63,Heights,*Filename,Layers,ELandscapeImportAlphamapType::Additive,TArrayView<const FLandscapeLayer>());
 Land->RegisterAllComponents();Land->PostEditChange();Land->MarkPackageDirty();return Land;
#else
 return nullptr;
#endif
}

bool UPiedmontWorldTools::TraceWorldSurface(FVector Start,FVector End,FVector& ImpactPoint,AActor*& HitActor,float SweepRadius){
 ImpactPoint=FVector::ZeroVector;HitActor=nullptr;
#if WITH_EDITOR
 UWorld* World=GEditor?GEditor->GetEditorWorldContext().World():nullptr;
 if(!World)return false;
 FHitResult Hit;FCollisionQueryParams Params(SCENE_QUERY_STAT(PiedmontSurfaceValidation),true);
 const bool HitSurface=SweepRadius>0?World->SweepSingleByChannel(Hit,Start,End,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(SweepRadius),Params):World->LineTraceSingleByChannel(Hit,Start,End,ECC_Visibility,Params);
 if(!HitSurface)return false;
 ImpactPoint=Hit.ImpactPoint;HitActor=Hit.GetActor();return true;
#else
 return false;
#endif
}

bool UPiedmontWorldTools::RefreshWaterBody(AActor* WaterActor){
#if WITH_EDITOR
 AWaterBody* Water=Cast<AWaterBody>(WaterActor);
 if(!Water||!Water->GetWaterBodyComponent())return false;
 FOnWaterBodyChangedParams Params;
 Params.bShapeOrPositionChanged=true;
 Params.bUserTriggered=true;
 Water->GetWaterBodyComponent()->OnWaterBodyChanged(Params);
 Water->MarkPackageDirty();return true;
#else
 return false;
#endif
}

AActor* UPiedmontWorldTools::SpawnValidationObstacle(UObject* WorldContext,FVector Location,FVector Scale){
#if WITH_EDITOR
 UWorld* World=WorldContext?WorldContext->GetWorld():nullptr;if(!World||World->WorldType!=EWorldType::PIE)return nullptr;
 FActorSpawnParameters Params;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
 auto* Actor=World->SpawnActor<AStaticMeshActor>(Location,FRotator::ZeroRotator,Params);
 if(!Actor)return nullptr;
 Actor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
 Actor->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
 Actor->SetActorScale3D(Scale);Actor->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));return Actor;
#else
 return nullptr;
#endif
}
