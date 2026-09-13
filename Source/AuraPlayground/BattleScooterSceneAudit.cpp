#include "BattleScooterScene.h"
#include "BattleBike.h"
#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"
#include "PiedmontPedestrian.h"
#include "Camera/CameraActor.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/OverlapResult.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
void TickBattleScooterSceneAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 static int Stage=0;static float Age=0;static TWeakObjectPtr<ABattleScooterScene> Selected,Skipped;static TWeakObjectPtr<ACameraActor> Camera;static bool HiddenWait=false;static FVector Initial[3];
 if(!PC||!PC->GetPawn()||PC->GetWorld()->GetTimeSeconds()<5||Stage==99)return;Age+=Dt;
 const FVector Site(30349.800013,114057.877225,1110.508188);
 auto Finish=[&](bool Passed,const TCHAR* Reason){
  // Snapshot before destruction, then prove owned people do not survive a removed scene.
  TArray<TWeakObjectPtr<APiedmontPedestrian>> Owned;
  if(Selected.IsValid())for(auto P:Selected->Participants)Owned.Add(P.Get());
  const int ParticipantCount=Owned.Num();const bool Ready=Selected.IsValid()&&Selected->bSceneReady;
  if(Selected.IsValid())Selected->Destroy();bool Clean=true;
  for(auto P:Owned)Clean&=!P.IsValid()||P->IsActorBeingDestroyed();Passed&=Clean;
  UE_LOG(LogTemp,Display,TEXT("ScooterCleanupAudit: {\"passed\":%s,\"participants_checked\":%d}"),Clean?TEXT("true"):TEXT("false"),ParticipantCount);
 UE_LOG(LogTemp,Display,TEXT("ScooterSceneAudit: {\"passed\":%s,\"reason\":\"%s\",\"offscreen_wait\":%s,\"participants\":%d,\"ready\":%s,\"skipped_empty\":%s}"),Passed?TEXT("true"):TEXT("false"),Reason,HiddenWait?TEXT("true"):TEXT("false"),ParticipantCount,Ready?TEXT("true"):TEXT("false"),Skipped.IsValid()&&!Skipped->bSelected&&Skipped->Participants.IsEmpty()?TEXT("true"):TEXT("false"));Stage=99;PC->ConsoleCommand(TEXT("quit"));};
 if(Stage==0){for(TActorIterator<ABattleScooterScene> Existing(PC->GetWorld());Existing;++Existing)Existing->Destroy();auto* C=PC->GetWorld()->SpawnActor<ACameraActor>(Site+FVector(550,0,300),FRotator(0,180,0));Camera=C;
  auto* A=PC->GetWorld()->SpawnActor<ABattleScooterScene>(Site,FRotator::ZeroRotator);A->AppearanceChance=1;Selected=A;
  auto* B=PC->GetWorld()->SpawnActor<ABattleScooterScene>(Site,FRotator::ZeroRotator);B->AppearanceChance=0;Skipped=B;Stage=1;Age=0;
 }
 if(Camera.IsValid()){PC->SetViewTarget(Camera.Get());if(PC->PlayerCameraManager)PC->PlayerCameraManager->UpdateCamera(0);}
 if(Stage==1){
  FHitResult Obstruction;FCollisionQueryParams Q;Q.AddIgnoredActor(PC->GetPawn());
  if(PC->GetWorld()->LineTraceSingleByObjectType(Obstruction,Camera->GetActorLocation(),Site+FVector(0,0,100),FCollisionObjectQueryParams(ECC_WorldStatic),Q)){Finish(false,TEXT("Visibility fixture is occluded; cannot prove on-screen exclusion"));return;}
 }

 if(!Selected.IsValid()||!Skipped.IsValid()){Finish(false,TEXT("Scene aborted during mapped placement"));return;}
 if(Stage==1&&Age>1){HiddenWait=Selected->bSelected&&!Selected->bSceneReady&&Selected->Participants.IsEmpty();Camera->SetActorRotation(FRotator::ZeroRotator);Stage=2;Age=0;return;}
 if(Stage==2&&Age>5){TArray<UStaticMeshComponent*> Parts;Selected->GetComponents(Parts);
  const bool Pass=HiddenWait&&Selected->bSceneReady&&Selected->Participants.Num()==3&&Selected->Participants[0]->bIncidentPosing&&Selected->Participants[1]->bIncidentPosing&&!Selected->Participants[2]->bIncidentPosing&&Parts.Num()==11&&!Skipped->bSelected&&Skipped->Participants.IsEmpty();
  if(Pass&&FParse::Param(FCommandLine::Get(),TEXT("BattleScooterVisitAudit"))){
   const FVector At=Site+FVector(-650,0,0);FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(PC->GetPawn());
   if(!PC->GetWorld()->LineTraceSingleByChannel(Hit,At+FVector(0,0,300),At-FVector(0,0,300),ECC_WorldStatic,Q)){Finish(false,TEXT("Missing visitor fixture ground"));return;}
   PC->GetPawn()->SetActorLocation(Hit.ImpactPoint+FVector(0,0,98),false,nullptr,ETeleportType::TeleportPhysics);
   if(auto* Bike=Cast<ABattleBike>(PC->GetPawn())){Bike->Ride->Speed=0;Bike->Ride->StopMovementImmediately();Bike->Ride->SetMovementMode(MOVE_Walking);Bike->Ride->bForceNextFloorCheck=true;}
   for(int I=0;I<3;I++)Initial[I]=Selected->Participants[I]->GetActorLocation();Stage=10;Age=0;return;
  }
  FString RenderPath;if(Pass&&FParse::Value(FCommandLine::Get(),TEXT("BattleScooterSceneRender="),RenderPath)){Camera->SetActorLocation(Site+FVector(550,0,300));Camera->SetActorRotation((Site+FVector(0,0,60)-Camera->GetActorLocation()).Rotation());Stage=3;Age=0;return;}
  Finish(Pass,TEXT("Actual Krog site: visible wait then offscreen assembly, two interactive poses, bystander and eleven scooter parts"));}
 if(Stage==3&&Age>.8f){FString Path;FParse::Value(FCommandLine::Get(),TEXT("BattleScooterSceneRender="),Path);FScreenshotRequest::RequestScreenshot(Path,false,false);Stage=4;Age=0;}
 if(Stage==4&&Age>1)Finish(true,TEXT("Runtime scene captured; visual review pending"));
 if(Stage==10&&Age>1){if(!Selected->bVisitStarted){Finish(false,TEXT("Nearby player did not begin visit"));return;}Stage=11;Age=0;}
 if(Stage==11&&Age>36){bool Released=true;int Moving=0;float MinDistance=100000;
  for(int I=0;I<3;I++){auto* P=Selected->Participants[I].Get();Released&=IsValid(P)&&!P->bIncidentPosing&&!P->bDead;if(IsValid(P)){float D=FVector::Dist2D(Initial[I],P->GetActorLocation());MinDistance=FMath::Min(MinDistance,D);if(D>100)Moving++;}}
  for(int I=0;I<3;I++){auto* P=Selected->Participants[I].Get();if(!IsValid(P))continue;
   FCollisionQueryParams Q(SCENE_QUERY_STAT(VisitStanding),false,P);TArray<FOverlapResult> Hits;PC->GetWorld()->OverlapMultiByChannel(Hits,P->GetActorLocation(),FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,86),Q);
   FString Names;for(const auto& H:Hits)if(H.bBlockingHit)Names+=GetNameSafe(H.GetActor())+TEXT(";");
   FNavLocation NavPoint;auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(PC->GetWorld());bool Projected=Nav&&Nav->ProjectPointToNavigation(P->GetActorLocation(),NavPoint,FVector(1000,1000,1000));
   UE_LOG(LogTemp,Display,TEXT("ScooterVisitPerson: index=%d posed=%d dead=%d pause=%.2f destination=%d nav=%d nav_distance=%.2f blockers=%s"),I,P->bIncidentPosing,P->bDead,P->PauseRemaining,P->bHasDestination,Projected,Projected?FVector::Dist2D(P->GetActorLocation(),NavPoint.Location):-1.f,*Names);
  }
  UE_LOG(LogTemp,Display,TEXT("ScooterVisitAudit: {\"visit_started\":%s,\"released\":%s,\"moving_participants\":%d,\"minimum_travel_cm\":%.2f}"),Selected->bVisitStarted?TEXT("true"):TEXT("false"),Released?TEXT("true"):TEXT("false"),Moving,MinDistance);
  Finish(Released&&Moving==3,TEXT("Visit timer releases both poses and all participants resume walking"));}
#endif
}
