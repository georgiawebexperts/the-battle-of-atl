#include "PiedmontWorldTools.h"
#include "PiedmontDarkZone.h"
#include "Landscape.h"
#include "Misc/FileHelper.h"
#if WITH_EDITOR
#include "Editor.h"
#include "Components/SkinnedMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/SkinWeightVertexBuffer.h"
#include "AssetCompilingManager.h"
#include "Containers/Ticker.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "NavMesh/RecastNavMesh.h"
#include "Builders/CubeBuilder.h"
#include "Model.h"
#include "Components/BrushComponent.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "WaterBodyActor.h"
#include "WaterBodyComponent.h"
#include "Commandlets/Commandlet.h"
#include "WaterSubsystem.h"
#include "WaterZoneActor.h"
#include "WaterMeshComponent.h"
#include "Engine/World.h"
#endif
void UPiedmontWorldTools::FinishEditorAssetLoading(){
#if WITH_EDITOR
 FlushAsyncLoading();FAssetCompilingManager::Get().FinishAllCompilation();
 if(IsRunningCommandlet())for(int I=0;I<20;I++)FTSTicker::GetCoreTicker().Tick(.15f);
#endif
}
bool UPiedmontWorldTools::RefreshLandscapeCollision(ALandscape* Landscape){
#if WITH_EDITOR
 if(!Landscape)return false;
 Landscape->RecreateCollisionComponents();Landscape->RecreateComponentsState();Landscape->MarkPackageDirty();return true;
#else
 return false;
#endif
}
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

AActor* UPiedmontWorldTools::SpawnValidationDarkZone(UObject* WorldContext,FVector Location){
#if WITH_EDITOR
 UWorld* World=WorldContext?WorldContext->GetWorld():nullptr;if(!World||World->WorldType!=EWorldType::PIE)return nullptr;
 return World->SpawnActor<APiedmontDarkZone>(Location,FRotator::ZeroRotator);
#else
 return nullptr;
#endif
}

bool UPiedmontWorldTools::BuildParkNavigation(FVector Center,FVector Extent){
#if WITH_EDITOR
 UWorld* World=GEditor?GEditor->GetEditorWorldContext().World():nullptr;
 if(!World||(World->GetName()!=TEXT("PiedmontWorld")&&World->GetName()!=TEXT("ArcadeBikeLab")&&World->GetName()!=TEXT("PiedmontKrogWorldReview")&&World->GetName()!=TEXT("PiedmontScooterReview")&&World->GetName()!=TEXT("PiedmontKrogApproachReview")))return false;
 for(TActorIterator<AActor> It(World);It;++It){
  const bool Relevant=It->ActorHasTag(TEXT("RideNavOnly"))||It->ActorHasTag(TEXT("RideDirt"))||It->ActorHasTag(TEXT("RidePath"))||It->ActorHasTag(TEXT("RideBridge"))||It->ActorHasTag(TEXT("RideBarrier"));
  TArray<UPrimitiveComponent*> Components;It->GetComponents(Components);
  for(auto* Component:Components){
   if(It->ActorHasTag(TEXT("RideNavOnly"))){
    // Refresh cached navigation registration after loading hidden, noncolliding tiles.
    Component->SetCanEverAffectNavigation(false);
    Component->SetCustomNavigableGeometry(EHasCustomNavigableGeometry::EvenIfNotCollidable);
    if(auto* Instances=Cast<UInstancedStaticMeshComponent>(Component)){
     UStaticMesh* Mesh=Instances->GetStaticMesh();
     if(Mesh)Mesh->CreateNavCollision();
    }
   }
   Component->SetCanEverAffectNavigation(Relevant);
   if(It->ActorHasTag(TEXT("RideNavOnly")))FNavigationSystem::UpdateComponentData(*Component);
  }
 }
 ANavMeshBoundsVolume* Bounds=nullptr;
 for(TActorIterator<ANavMeshBoundsVolume> It(World);It;++It)if(It->ActorHasTag(TEXT("PiedmontNavBounds")))Bounds=*It;
 if(!Bounds){Bounds=World->SpawnActor<ANavMeshBoundsVolume>();Bounds->Tags.Add(TEXT("PiedmontNavBounds"));Bounds->SetActorLabel(TEXT("Park path navigation bounds"));}
 Bounds->SetActorLocation(Center);Bounds->Brush=NewObject<UModel>(Bounds,NAME_None,RF_Transactional);Bounds->Brush->Initialize(Bounds,true);Bounds->GetBrushComponent()->Brush=Bounds->Brush;
 auto* Builder=NewObject<UCubeBuilder>(Bounds);Bounds->BrushBuilder=Builder;Builder->X=Extent.X*2;Builder->Y=Extent.Y*2;Builder->Z=Extent.Z*2;
 if(!Builder->Build(World,Bounds))return false;Bounds->SetActorLocation(Center);Bounds->GetBrushComponent()->BuildSimpleBrushCollision();Bounds->PostEditChange();
 auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
 if(!Nav){FNavigationSystem::AddNavigationSystemToWorld(*World,FNavigationSystemRunMode::EditorMode);Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);}
 if(!Nav)return false;Nav->OnNavigationBoundsUpdated(Bounds);
 if(auto* Recast=Cast<ARecastNavMesh>(Nav->GetDefaultNavDataInstance(FNavigationSystem::Create))){
  Recast->AgentRadius=36;Recast->AgentHeight=180;Recast->AgentMaxSlope=40;
  // Long park routes exhausted Recast's default search budget during the dense audit.
  Recast->DefaultMaxSearchNodes=32768;Recast->RecreateDefaultFilter();
  for(uint8 I=0;I<(uint8)ENavigationDataResolution::MAX;++I){Recast->SetCellSize((ENavigationDataResolution)I,10);Recast->SetCellHeight((ENavigationDataResolution)I,2);Recast->NavMeshResolutionParams[I].AgentMaxStepHeight=24;}
 }
 // Headless imports do not pump the editor's delayed asset-loading unlock.
 // Finish assets, then let the engine's own ticker release that specific lock.
 if(IsRunningCommandlet()){
  FlushAsyncLoading();FAssetCompilingManager::Get().FinishAllCompilation();
  for(int I=0;I<20;I++)FTSTicker::GetCoreTicker().Tick(.15f);
 }
 Nav->Build();World->MarkPackageDirty();return true;
#else
 return false;
#endif
}
bool UPiedmontWorldTools::ProjectParkNavigation(FVector Point,FVector& Projected){
#if WITH_EDITOR
 UWorld* World=GEditor?GEditor->GetEditorWorldContext().World():nullptr;auto* Nav=World?FNavigationSystem::GetCurrent<UNavigationSystemV1>(World):nullptr;FNavLocation Location;
 if(Nav&&Nav->ProjectPointToNavigation(Point,Location,FVector(120,120,180))){Projected=Location.Location;return true;}
#endif
 return false;
}
float UPiedmontWorldTools::ParkRouteLength(FVector Start,FVector End){
#if WITH_EDITOR
 UWorld* World=GEditor?GEditor->GetEditorWorldContext().World():nullptr;if(!World)return -1;
 auto* Path=UNavigationSystemV1::FindPathToLocationSynchronously(World,Start,End);
 if(Path&&Path->GetPath().IsValid()&&Path->GetPath()->DidSearchReachedLimit())return -2;
 if(Path&&Path->IsValid()&&!Path->IsPartial())return Path->GetPathLength();
#endif
 return -1;
}

bool UPiedmontWorldTools::IsParkNavigationBuilding(){
#if WITH_EDITOR
 UWorld* World=GEditor?GEditor->GetEditorWorldContext().World():nullptr;auto* Nav=World?FNavigationSystem::GetCurrent<UNavigationSystemV1>(World):nullptr;return Nav&&Nav->IsNavigationBuildInProgress();
#else
 return false;
#endif
}

bool UPiedmontWorldTools::FinishParkNavigationBuild(){
#if WITH_EDITOR
 UWorld* World=GEditor?GEditor->GetEditorWorldContext().World():nullptr;
 if(!World||(World->GetName()!=TEXT("PiedmontWorld")&&World->GetName()!=TEXT("PiedmontKrogWorldReview")&&World->GetName()!=TEXT("PiedmontScooterReview")&&World->GetName()!=TEXT("PiedmontKrogApproachReview")))return false;
 for(TActorIterator<ANavigationData> It(World);It;++It)It->EnsureBuildCompletion();
 World->MarkPackageDirty();return !IsParkNavigationBuilding();
#else
 return false;
#endif
}

void UPiedmontWorldTools::TickSceneReview(){
#if WITH_EDITOR
 if(IsRunningCommandlet() && GEditor){
  if(UWorld* World=GEditor->GetEditorWorldContext().World()){
   if(UWaterSubsystem* Water=World->GetSubsystem<UWaterSubsystem>())Water->Tick(1.f/60.f);
   CommandletHelpers::TickEngine(World,1.0/60.0);
  }
 }
#endif
}

void UPiedmontWorldTools::RebuildWaterZones(){
#if WITH_EDITOR
 if(UWorld* World=GEditor?GEditor->GetEditorWorldContext().World():nullptr){
  for(TActorIterator<AWaterZone> It(World);It;++It){
   It->MarkForRebuild(EWaterZoneRebuildFlags::All);It->Update();
   if(auto* Mesh=It->FindComponentByClass<UWaterMeshComponent>()){Mesh->MarkWaterMeshGridDirty();Mesh->Update();}
   It->MarkPackageDirty();
  }
 }
#endif
}

bool UPiedmontWorldTools::ReviewSkinGroundClearance(USkinnedMeshComponent* Mesh,float& MinimumClearance,int32& Samples,FVector& ClosestVertex,FVector& GroundPoint,FString& GroundActor){
 MinimumClearance=0;Samples=0;ClosestVertex=GroundPoint=FVector::ZeroVector;GroundActor.Reset();
#if WITH_EDITOR
 if(!Mesh||!Mesh->GetWorld())return false;
 // A commandlet may not evaluate an offscreen leader before the first capture.
 if(auto* Pose=Cast<USkeletalMeshComponent>(Mesh->LeaderPoseComponent.IsValid()?Mesh->LeaderPoseComponent.Get():Mesh)){Pose->TickAnimation(0.f,false);Pose->RefreshBoneTransforms();}
 const auto* Data=Mesh->GetSkeletalMeshRenderData();
 if(!Data||Data->LODRenderData.Num()==0)return false;
 const auto& LOD=Data->LODRenderData[0];auto* Weights=Mesh->GetSkinWeightBuffer(0);
 if(!Weights)return false;
 float Min=TNumericLimits<float>::Max();FCollisionQueryParams Q;Q.AddIgnoredActor(Mesh->GetOwner());
 for(uint32 I=0;I<LOD.GetNumVertices();I++){
  const FVector Point=Mesh->GetComponentTransform().TransformPosition(FVector(USkinnedMeshComponent::GetSkinnedVertexPosition(Mesh,I,LOD,*Weights)));
  FHitResult Hit;
  if(!Mesh->GetWorld()->LineTraceSingleByChannel(Hit,Point+FVector(0,0,300),Point-FVector(0,0,500),ECC_WorldStatic,Q))return false;
  const float Gap=float(Point.Z-Hit.ImpactPoint.Z);
  if(Gap<Min){Min=Gap;ClosestVertex=Point;GroundPoint=Hit.ImpactPoint;GroundActor=Hit.GetActor()?Hit.GetActor()->GetActorLabel():TEXT("None");}Samples++;
 }
 if(!Samples)return false;MinimumClearance=Min;return true;
#else
 return false;
#endif
}
