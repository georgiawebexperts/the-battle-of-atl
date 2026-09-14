#include "BattleBike.h"
#include "BattleQuest.h"
#include "BattleZombie.h"
#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Engine/GameViewportClient.h"
#include "Misc/Paths.h"
void TickBattlePhoneRideAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TArray<FVector> Points;FVector Previous;int Next=1,Stage=0;float Clock=0,Distance=0,Still=0,Progress=0,WaypointAge=0,BestWaypointDistance=MAX_flt;bool Started=false,Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;
 auto Key=[&](FKey K,bool Down){if(PC->IsInputKeyDown(K)!=Down)PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto* B=Cast<ABattleBike>(PC->GetPawn());auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(PC));
 auto End=[&](bool Pass,const TCHAR* Why){for(FKey K:{EKeys::W,EKeys::A,EKeys::D,EKeys::SpaceBar})Key(K,false);S.Done=true;UE_LOG(LogTemp,Display,TEXT("PhoneRideAudit: {\"passed\":%s,\"reason\":\"%s\",\"seconds\":%.2f,\"distance_cm\":%.2f,\"waypoint\":%d,\"points\":%d}"),Pass?TEXT("true"):TEXT("false"),Why,S.Clock,S.Distance,S.Next,S.Points.Num());PC->ConsoleCommand(TEXT("quit"));};
 if(!B||!M||!M->Quest){End(false,TEXT("Missing mounted player or quest"));return;}auto* Q=M->Quest.Get();const bool Full=FParse::Param(FCommandLine::Get(),TEXT("BattleFullRide"));const bool Replay=FParse::Param(FCommandLine::Get(),TEXT("BattleCrossingReplay"));
 if(!S.Started){
  if(!Q->bReady){if(PC->GetWorld()->GetTimeSeconds()>20)End(false,TEXT("Phone placement never became ready"));return;}
  if(Replay){
   const FVector Start(26050,100350,893);B->Ride->StopMovementImmediately();B->Ride->Speed=0;B->SetActorLocationAndRotation(Start,FRotator(0,90,0),false,nullptr,ETeleportType::TeleportPhysics);B->Ride->bForceNextFloorCheck=true;
   S.Points={Start,FVector(26084.626,100829.681,794.759),FVector(26074.985,100888.330,795.444),FVector(26064.573,100946.858,795.763),FVector(26054.161,101005.387,796.319),FVector(26044.694,101071.118,796.819),FVector(26035.226,101136.850,797.072),FVector(26000,101500,800)};
  }else{
   auto* Path=UNavigationSystemV1::FindPathToLocationSynchronously(PC,B->GetActorLocation(),Q->ArtifactLocation,B);
   if(!Path||!Path->IsValid()||Path->IsPartial()||Path->PathPoints.Num()<2){End(false,TEXT("No complete navigation path to phone"));return;}
   S.Points=Path->PathPoints;S.Points.Last()=Q->ArtifactLocation;
  }
  S.Previous=B->GetActorLocation();S.Started=true;
  if(M->Enemies)M->Enemies->bFreezeSpawns=true;
  for(TActorIterator<APiedmontTrafficDirector> I(PC->GetWorld());I;++I)I->DesiredPopulation=0;
  for(TActorIterator<ABattleZombie> I(PC->GetWorld());I;++I)I->Destroy();for(TActorIterator<APiedmontPedestrian> I(PC->GetWorld());I;++I)I->Destroy();
  UE_LOG(LogTemp,Display,TEXT("PhoneRideFixture: phone=%s way=%s start=%s points=%d"),*Q->ArtifactLocation.ToString(),*Q->ArtifactWay,*B->GetActorLocation().ToString(),S.Points.Num());
 }
 S.Clock+=Dt;const FVector P=B->GetActorLocation();const float Step=FVector::Dist2D(P,S.Previous);S.Distance+=Step;S.Previous=P;S.Still=Step<Dt*12?S.Still+Dt:0;
 if(Replay&&P.Y>101400){End(true,TEXT("Isolated replay crossed the stalled location"));return;}
 if(Replay&&S.Clock>5&&S.Clock-Dt<=5)FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("PhoneCrossingReplay.png"),true,false);
 if(Q->bCollected&&!Full&&!Replay){End(true,TEXT("Rode from park start and collected phone through proximity; no teleport"));return;}
 if(Full&&M->bRunEnded){End(M->bWon,M->bWon?TEXT("Phone, ordered checkpoints, tunnel and finish reached through driving"):TEXT("Run ended before arrival"));return;}
 const int Stage=Q->bCollected?1+Q->NextCheckpoint:0;
 if(Full&&Stage!=S.Stage){
  if(Q->RoutePoints.Num()<2){Key(EKeys::W,false);Key(EKeys::SpaceBar,true);if(S.Still>5)End(false,TEXT("Collected objective has no usable onward route"));return;}
  S.Stage=Stage;S.Points=Q->RoutePoints;S.Next=1;S.Still=0;S.WaypointAge=0;S.BestWaypointDistance=MAX_flt;
  UE_LOG(LogTemp,Display,TEXT("PhoneRideStage: stage=%d points=%d position=%s target=%s"),S.Stage,S.Points.Num(),*P.ToString(),*S.Points.Last().ToString());
 }
 S.Progress+=Dt;if(S.Progress>10){S.Progress=0;UE_LOG(LogTemp,Display,TEXT("PhoneRideProgress: stage=%d seconds=%.1f waypoint=%d/%d position=%s speed=%.1f"),S.Stage,S.Clock,S.Next,S.Points.Num(),*P.ToString(),B->Ride->Speed);}
 if((Replay&&S.Clock>5||S.WaypointAge>5)&&FMath::FloorToInt(S.Clock)!=FMath::FloorToInt(S.Clock-Dt)){
  FHitResult Hit;FCollisionQueryParams Params(SCENE_QUERY_STAT(PhoneRideBlocker),false,B);
  const FVector Toward=(S.Points[S.Next]-P).GetSafeNormal2D();
  const bool Blocked=PC->GetWorld()->SweepSingleByChannel(Hit,P,P+Toward*180,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeSphere(32),Params);
  UE_LOG(LogTemp,Display,TEXT("PhoneCrossingContact: position=%s yaw=%.2f blocked=%d actor=%s component=%s normal=%s speed=%.2f"),*P.ToString(),B->GetActorRotation().Yaw,Blocked,*GetNameSafe(Hit.GetActor()),*GetNameSafe(Hit.GetComponent()),*Hit.Normal.ToString(),B->Ride->Speed);
 }
 if(Replay&&FParse::Param(FCommandLine::Get(),TEXT("BattleCrossingTrafficWarmup"))&&S.Clock<12){Key(EKeys::W,false);Key(EKeys::SpaceBar,true);S.Still=S.WaypointAge=0;return;}
 if(S.WaypointAge>5&&S.WaypointAge-Dt<=5)FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("PhoneRouteStall.png"),true,false);
 if(B->bCrashActive||B->RiderHealth<=0){End(false,TEXT("Ride interrupted by crash or death"));return;}
 if(S.Clock>(Replay?25:Full?800:150)||S.Still>12||S.WaypointAge>30){UE_LOG(LogTemp,Display,TEXT("PhoneRideFailure: position=%s target=%s speed=%.2f yaw=%.2f"),*P.ToString(),*S.Points[S.Next].ToString(),B->Ride->Speed,B->GetActorRotation().Yaw);End(false,TEXT("Guided input stalled or timed out; inspect controller and world"));return;}
 const int OldNext=S.Next;while(S.Next<S.Points.Num()-1&&FVector::Dist2D(P,S.Points[S.Next])<160)S.Next++;const float TargetDistance=FVector::Dist2D(P,S.Points[S.Next]);
 // Long clear segments are progress, even before reaching the next waypoint.
 if(S.Next!=OldNext||TargetDistance<S.BestWaypointDistance-50){S.WaypointAge=0;S.BestWaypointDistance=TargetDistance;}else S.WaypointAge+=Dt;
 const float Angle=FMath::FindDeltaAngleDegrees(B->GetActorRotation().Yaw,(S.Points[S.Next]-P).Rotation().Yaw);
 Key(EKeys::A,Angle< -5);Key(EKeys::D,Angle>5);Key(EKeys::W,true);Key(EKeys::SpaceBar,FMath::Abs(Angle)>45&&B->Ride->Speed>220);
#endif
}
