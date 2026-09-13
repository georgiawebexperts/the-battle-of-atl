#include "BattleRoadTrafficDirector.h"
#include "BattleRoadCar.h"
#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
void TickBattleCarLoopAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ABattleRoadCar> Car;TWeakObjectPtr<ACameraActor> Camera;FVector Previous;float Age=0,Length=0,MaxStep=0;bool Started=false,Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}
 if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;S.Age+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Why){auto* C=S.Car.Get();UE_LOG(LogTemp,Display,TEXT("CarLoopAudit: {\"passed\":%s,\"reason\":\"%s\",\"loops\":%d,\"travel_cm\":%.2f,\"loop_length_cm\":%.2f,\"max_frame_step_cm\":%.2f,\"obstacle\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Why,C?C->CompletedLoops:0,C?C->DistanceTravelled:0,S.Length,S.MaxStep,C?*C->LastObstacle:TEXT("missing"));S.Done=true;PC->ConsoleCommand(TEXT("quit"));};
 if(!S.Started){
  FBattleRoadTrafficLane Lane;bool Found=false;
  for(TActorIterator<ABattleRoadTrafficDirector> It(PC->GetWorld());It;++It){It->SetActorTickEnabled(false);for(const auto& L:It->Lanes)if(L.bLoopRoute){Lane=L;Found=true;}}
  if(!Found){Finish(false,TEXT("No closed review lane"));return;}
  for(TActorIterator<APiedmontTrafficDirector> It(PC->GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(PC->GetWorld());It;++It)It->Destroy();
  for(TActorIterator<ABattleRoadCar> It(PC->GetWorld());It;++It)It->Destroy();
  auto* C=PC->GetWorld()->SpawnActor<ABattleRoadCar>();S.Car=C;C->Route=Lane.Points;C->Crossings=Lane.Crossings;C->CruiseSpeed=Lane.CruiseSpeed;C->bLoopRoute=true;
  if(!C->StartRoute()){Finish(false,TEXT("Closed route rejected"));return;}
  for(int I=1;I<C->Route.Num();I++)S.Length+=FVector::Dist2D(C->Route[I-1],C->Route[I]);
  S.Camera=PC->GetWorld()->SpawnActor<ACameraActor>();S.Previous=C->GetActorLocation();S.Age=0;S.Started=true;return;
 }
 auto* C=S.Car.Get();if(!C){Finish(false,TEXT("Visible car disappeared"));return;}
 S.MaxStep=FMath::Max(S.MaxStep,float(FVector::Dist(C->GetActorLocation(),S.Previous)));S.Previous=C->GetActorLocation();
 S.Camera->SetActorLocation(C->GetActorLocation()+FVector(600,400,400));S.Camera->SetActorRotation((C->GetActorLocation()-S.Camera->GetActorLocation()).Rotation());PC->SetViewTarget(S.Camera.Get());
 if(C->CompletedLoops>=1&&C->DistanceTravelled>S.Length+800){Finish(!C->bRouteFinished&&C->Speed>100&&S.MaxStep<80,TEXT("Visible native car completed both turns and continued across route seam"));return;}
 if(S.Age>150)Finish(false,TEXT("Car did not complete loop in time"));
#endif
}
