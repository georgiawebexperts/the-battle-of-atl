#include "BattleRoadTrafficDirector.h"
#include "Camera/CameraActor.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
void TickBattleRoadTrafficAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ABattleRoadTrafficDirector> Director;TWeakObjectPtr<ABattleRoadCar> First;TWeakObjectPtr<ACameraActor> Camera;float Clock=0;int Phase=0;bool Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Why){S.Done=true;UE_LOG(LogTemp,Display,TEXT("RoadTrafficAudit: {\"passed\":%s,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Why);PC->ConsoleCommand(TEXT("quit"));};
#define CHECK_TRAFFIC(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 CHECK_TRAFFIC(S.Clock<15,"Director audit timed out");
 auto View=[&](FVector Position,FVector Facing){S.Camera->SetActorLocation(Position);S.Camera->SetActorRotation(Facing.Rotation());PC->SetViewTarget(S.Camera.Get());PC->PlayerCameraManager->UpdateCamera(0);};
 const FVector Start(-19800,13075,350);
 if(S.Phase==0){
  auto* D=PC->GetWorld()->SpawnActor<ABattleRoadTrafficDirector>();D->SetActorTickEnabled(false);D->MaxCars=2;D->MaxCarsPerLane=2;S.Director=D;S.Camera=PC->GetWorld()->SpawnActor<ACameraActor>();
  FBattleRoadTrafficLane Lane;Lane.Points={Start,FVector(-15800,13015,300)};D->Lanes={Lane,Lane};
  CHECK_TRAFFIC(!D->TrySpawnLane(-1),"Invalid lane accepted");
  View(Start+FVector(0,1000,200),FVector(0,1,0));CHECK_TRAFFIC(!D->TrySpawnLane(0),"Spawned near camera");
  const FVector Eye=Start+FVector(0,6000,2000);View(Eye,Start-Eye);CHECK_TRAFFIC(!D->TrySpawnLane(0),"Spawned in view");
  View(Eye,FVector(0,1,0));S.First=D->TrySpawnLane(0);CHECK_TRAFFIC(S.First.IsValid()&&D->LiveCars==1,"Hidden spawn failed");CHECK_TRAFFIC(!D->TrySpawnLane(0)&&D->TotalSpawned==1,"Occupied spawn accepted");S.Phase=1;S.Clock=0;return;
 }
 if(S.Phase==1&&S.Clock>3){
  auto* D=S.Director.Get();CHECK_TRAFFIC(S.First.IsValid()&&S.First->DistanceTravelled>500,"First car failed to move");auto* Second=D->TrySpawnLane(0);CHECK_TRAFFIC(Second&&D->LiveCars==2,"Second spaced car failed");CHECK_TRAFFIC(!D->TrySpawnLane(1)&&D->PeakCars==2,"Population cap failed");
  S.First->bRouteFinished=true;S.First->Speed=0;const FVector Position=S.First->GetActorLocation();View(Position+FVector(0,1000,200),FVector(0,-1,0));D->Tick(.1f);CHECK_TRAFFIC(S.First.IsValid()&&D->TotalRemoved==0,"Visible completed car removed");
  View(Position+FVector(0,6000,2000),FVector(0,1,0));D->Tick(.1f);CHECK_TRAFFIC(D->TotalRemoved==1&&D->LiveCars==1,"Hidden completed car not removed");D->Destroy();CHECK_TRAFFIC(Second->IsActorBeingDestroyed(),"Director teardown left car alive");Finish(true,TEXT("Visibility, proximity, overlap rejection, spacing, cap, retention, removal and teardown passed"));
 }
#undef CHECK_TRAFFIC
#endif
}

#include "EngineUtils.h"
void TickBattleTrafficPopulationAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ABattleRoadTrafficDirector> Director;TWeakObjectPtr<ACameraActor> Camera;float Clock=0,ProgressClock=0;bool SawWait=false,Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Why){S.Done=true;auto* D=S.Director.Get();UE_LOG(LogTemp,Display,TEXT("TrafficPopulationAudit: {\"passed\":%s,\"reason\":\"%s\",\"spawned\":%d,\"removed\":%d,\"peak\":%d,\"saw_signal_wait\":%s}"),Pass?TEXT("true"):TEXT("false"),Why,D?D->TotalSpawned:0,D?D->TotalRemoved:0,D?D->PeakCars:0,S.SawWait?TEXT("true"):TEXT("false"));PC->ConsoleCommand(TEXT("quit"));};
 if(!S.Director.IsValid()){
  for(TActorIterator<ABattleRoadTrafficDirector> I(PC->GetWorld());I;++I)if(I->ActorHasTag(TEXT("TenthRoadTrafficReview")))S.Director=*I;
  if(!S.Director.IsValid()){Finish(false,TEXT("Review director missing"));return;}
  S.Camera=PC->GetWorld()->SpawnActor<ACameraActor>();S.Camera->SetActorLocation(FVector(-9000,21000,1800));S.Camera->SetActorRotation(FRotator(0,90,0));
  if(S.Director->ActorHasTag(TEXT("IrwinTrafficReview"))){S.Camera->SetActorLocation(FVector(13000,100850,2500));S.Camera->SetActorRotation(FRotator(0,180,0));}
 }
 // Keep the controlled observation direction through the game's respawn flow.
 PC->SetViewTarget(S.Camera.Get());PC->PlayerCameraManager->UpdateCamera(0);
 auto* D=S.Director.Get();if(D->PeakCars>6||D->LiveCars>6){Finish(false,TEXT("Traffic cap exceeded"));return;}
 for(TActorIterator<ABattleRoadCar> I(PC->GetWorld());I;++I)if(I->ActorHasTag(TEXT("AmbientRoadCar"))&&!I->IsActorBeingDestroyed()){
  if(!I->bGrounded){Finish(false,TEXT("Traffic lost wheel support"));return;}
  S.SawWait|=I->bWaitingForCrossing&&I->Speed<1.f;
 }
 S.ProgressClock+=Dt;if(S.ProgressClock>20){S.ProgressClock=0;UE_LOG(LogTemp,Display,TEXT("TrafficPopulationProgress: spawned=%d removed=%d live=%d peak=%d waited=%d"),D->TotalSpawned,D->TotalRemoved,D->LiveCars,D->PeakCars,S.SawWait);}
 if(D->TotalSpawned>=8&&D->TotalRemoved>=2&&(S.SawWait||D->ActorHasTag(TEXT("IrwinTrafficReview")))){Finish(true,TEXT("Automatic opposing traffic, required crossing checks, bounded population and endpoint recycling passed"));return;}
 if(S.Clock>140)Finish(false,TEXT("Traffic population/recycling timed out"));
#endif
}
