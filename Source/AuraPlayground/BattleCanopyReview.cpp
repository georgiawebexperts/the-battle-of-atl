#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "UnrealClient.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

#if !UE_BUILD_SHIPPING
static bool InstallRuntimeCanopy(UWorld* World){
 auto Read=[](const TCHAR* File,TSharedPtr<FJsonObject>& Result){FString Text;return FFileHelper::LoadFileToString(Text,*(FPaths::ProjectDir()/File))&&FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text),Result)&&Result.IsValid();};
 TSharedPtr<FJsonObject> Placement,Stations;
 if(!Read(FParse::Param(FCommandLine::Get(),TEXT("BattleMixedCanopy"))?TEXT("SourceAssets/Terrain/park-mixed-canopy.json"):TEXT("Tests/Results/2026-09-12-detailed-canopy-low-clearance.json"),Placement)||!Read(TEXT("SourceAssets/Terrain/park-tree-stations.json"),Stations))return false;
 const auto& Trees=Stations->GetArrayField(TEXT("trees"));TMap<FString,UHierarchicalInstancedStaticMeshComponent*> Groups;int Count=0;
 for(const auto& Value:Placement->GetArrayField(TEXT("accepted"))){
  const auto Row=Value->AsObject();const int Index=Row->GetIntegerField(TEXT("index"));if(!Trees.IsValidIndex(Index))return false;const FString Path=Row->GetStringField(TEXT("mesh"));
  UHierarchicalInstancedStaticMeshComponent* Instances=Groups.FindRef(Path);
  if(!Instances){auto* Mesh=LoadObject<UStaticMesh>(nullptr,*Path);if(!Mesh)return false;auto* Actor=World->SpawnActor<AActor>();Instances=NewObject<UHierarchicalInstancedStaticMeshComponent>(Actor);Actor->SetRootComponent(Instances);Actor->AddInstanceComponent(Instances);Instances->SetStaticMesh(Mesh);Instances->SetCollisionEnabled(ECollisionEnabled::NoCollision);Instances->SetCanEverAffectNavigation(false);Instances->SetCullDistances(28000,35000);Instances->RegisterComponent();Groups.Add(Path,Instances);}
  const auto Station=Trees[Index]->AsObject();const auto& XY=Station->GetArrayField(TEXT("xy_cm"));const FVector Point(XY[0]->AsNumber(),XY[1]->AsNumber(),0);FHitResult Hit;FCollisionQueryParams Q;Q.bTraceComplex=true;
  if(!World->LineTraceSingleByChannel(Hit,Point+FVector(0,0,7000),Point-FVector(0,0,7000),ECC_WorldStatic,Q))return false;
  const auto Bounds=Instances->GetStaticMesh()->GetBounds();const float Scale=Station->GetNumberField(TEXT("height_game_cm"))/(2*Bounds.BoxExtent.Z);
  const FVector Location=Hit.ImpactPoint-FVector(0,0,(Bounds.Origin.Z-Bounds.BoxExtent.Z)*Scale);
  Instances->AddInstance(FTransform(FRotator(0,Station->GetNumberField(TEXT("yaw")),0),Location,FVector(Scale)),true);++Count;
 }
 if(Count!=Placement->GetIntegerField(TEXT("instances")))return false;
 for(auto& Pair:Groups)Pair.Value->BuildTreeIfOutdated(false,true);
 for(TActorIterator<AActor> It(World);It;++It)if(It->ActorHasTag(TEXT("BattleParkCanopy")))It->SetActorHiddenInGame(true);
 UE_LOG(LogTemp,Display,TEXT("CanopySwap: %d instances"),Count);return true;
}
#endif

void TickBattleCanopyReview(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ACameraActor> Camera;int Phase=0;double Start=0,Previous=0;TArray<double> Frames;bool Prepared=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Phase==4||PC->GetWorld()->GetTimeSeconds()<5)return;
 if(!S.Prepared){S.Prepared=true;if(FParse::Param(FCommandLine::Get(),TEXT("BattleDetailedCanopy"))&&!InstallRuntimeCanopy(PC->GetWorld())){S.Phase=4;UE_LOG(LogTemp,Error,TEXT("CanopyRuntime: replacement failed"));PC->ConsoleCommand(TEXT("quit"));return;}}
 const double Now=FPlatformTime::Seconds();
 if(S.Phase==3){if(Now-S.Start>2){S.Phase=4;UE_LOG(LogTemp,Display,TEXT("CanopyRuntime: complete"));PC->ConsoleCommand(TEXT("quit"));}return;}
 if(!S.Camera.IsValid())S.Camera=PC->GetWorld()->SpawnActor<ACameraActor>();
 if(S.Start==0){
  const FVector XY[]={FVector(-17400,-5200,0),FVector(-6000,2000,0),FVector(0,-4000,0)};
  const FVector Target[]={FVector(-14000,-5200,0),FVector(0,-4000,0),FVector(-6000,2000,0)};
  FHitResult Hit;FCollisionQueryParams Q;Q.bTraceComplex=true;if(PC->GetPawn())Q.AddIgnoredActor(PC->GetPawn());
  if(!PC->GetWorld()->LineTraceSingleByChannel(Hit,XY[S.Phase]+FVector(0,0,7000),XY[S.Phase]-FVector(0,0,7000),ECC_WorldStatic,Q)){S.Phase=4;UE_LOG(LogTemp,Error,TEXT("CanopyRuntime: missing floor"));PC->ConsoleCommand(TEXT("quit"));return;}
  FVector Position=Hit.ImpactPoint+FVector(0,0,170);FVector Look=Target[S.Phase];Look.Z=Position.Z;
  S.Camera->SetActorLocationAndRotation(Position,(Look-Position).Rotation());S.Camera->GetCameraComponent()->SetFieldOfView(85);
  if(APawn* Pawn=PC->GetPawn()){Pawn->SetActorLocation(Hit.ImpactPoint+FVector(0,0,98),false,nullptr,ETeleportType::TeleportPhysics);Pawn->SetActorHiddenInGame(true);if(auto* Character=Cast<ACharacter>(Pawn))Character->GetCharacterMovement()->DisableMovement();}
  S.Start=Now;S.Previous=Now;S.Frames.Reset();
 }
 PC->SetViewTarget(S.Camera.Get());if(PC->GetHUD())PC->GetHUD()->bShowHUD=false;PC->PlayerCameraManager->UpdateCamera(0);
 if(Now-S.Start>5&&S.Previous>0)S.Frames.Add((Now-S.Previous)*1000);S.Previous=Now;
 if(Now-S.Start>9){
  S.Frames.Sort();if(S.Frames.IsEmpty())return;const double P50=S.Frames[S.Frames.Num()/2],P95=S.Frames[FMath::Min(S.Frames.Num()-1,FMath::FloorToInt(S.Frames.Num()*.95))];
  UE_LOG(LogTemp,Display,TEXT("CanopyFrame: {\"view\":%d,\"samples\":%d,\"p50_ms\":%.3f,\"p95_ms\":%.3f}"),S.Phase+1,S.Frames.Num(),P50,P95);
  FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);IFileManager::Get().MakeDirectory(*Folder,true);FScreenshotRequest::RequestScreenshot(Folder/FString::Printf(TEXT("canopy-%d.png"),S.Phase+1),false,false);
  ++S.Phase;S.Start=S.Phase==3?Now:0;
 }
#endif
}
