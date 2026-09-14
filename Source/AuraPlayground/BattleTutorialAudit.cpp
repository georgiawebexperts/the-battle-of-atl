#include "BattleMacController.h"
#include "UnrealClient.h"
#include "HAL/FileManager.h"
#include "BattleTutorial.h"
#include "BattleMarketClosure.h"
#include "BattleTutorialData.h"
#include "BattleTutorialBlock.h"
#include "Misc/CommandLine.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleQuest.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
void ABattleMacController::TickTutorialAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5||TutorialStage<0)return;
 auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* B=Cast<ABattleBike>(GetPawn());ABattleTutorial* T=nullptr;for(TActorIterator<ABattleTutorial> It(GetWorld());It;++It)T=*It;
 auto End=[&](bool Pass,const TCHAR* Why){TutorialStage=-1;FlushPressedKeys();UE_LOG(LogTemp,Display,TEXT("BattleTutorialAudit: {\"passed\":%s,\"reason\":\"%s\",\"distance_cm\":%.1f,\"max_error_cm\":%.1f}"),Pass?TEXT("true"):TEXT("false"),Why,TutorialDistance,TutorialMaxError);ConsoleCommand(TEXT("quit"));};
#define TCHECK(C,R) if(!(C)){End(false,TEXT(R));return;}
 TCHECK(M&&B&&T&&M->Quest,"Missing tutorial, bike or quest");
 if(TutorialStage==0){
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleCountdownControlsAudit"))){
   B->SetActorLocationAndRotation(BattleTutorialData::Gate+FVector(20,0,98),FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);B->Ride->StopMovementImmediately();B->Ride->Speed=0;
   TCHECK(T->TryStart(BattleTutorialData::Gate-FVector(100,0,0),B->GetActorLocation()),"Gate fixture did not start countdown");TutorialStage=2;TutorialClock=0;return;
  }

  TCHECK(M->bTutorialActive&&M->StartCountdown==0&&M->RunElapsed==0&&M->TimeRemaining==M->Difficulty.TimeLimitSeconds,"Practice timer ran before gateway");
  TCHECK(!M->AdjustRunTime(30,TEXT("practice"))&&B->ApplyRiderDamage(10)==0,"Practice changed time or damaged rider");
  for(int I=0;I<UE_ARRAY_COUNT(BattleTutorialBlock::BarrierCenters);I++){
   const FVector C=BattleTutorialBlock::BarrierCenters[I]+FVector(0,0,98),D=BattleTutorialBlock::BarrierDirections[I];FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(PracticeFence),false,B);
   TCHECK(GetWorld()->SweepSingleByChannel(Hit,C-D*220,C+D*220,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(32),Q)&&Hit.GetActor()==T,"Construction fence does not physically block the road");
  }
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleMarketClosureReview"))){
   TActorIterator<ABattleMarketClosure> It(GetWorld());TCHECK(It,"Market closure missing");auto* Closure=*It;
   for(int Y=-780;Y<=780;Y+=20)for(float Z:{90.f,180.f,300.f}){
    const FVector C=Closure->GetActorLocation()+FVector(0,Y,Z);FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(B);Q.AddIgnoredActor(T);
    for(TActorIterator<APawn> Pawn(GetWorld());Pawn;++Pawn)Q.AddIgnoredActor(*Pawn);
    const bool Blocked=GetWorld()->SweepSingleByChannel(Hit,C-FVector(150,0,0),C+FVector(150,0,0),FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(30),Q);
    if(!Blocked||Hit.GetActor()!=Closure)UE_LOG(LogTemp,Display,TEXT("MarketClosureMiss: y=%d z=%.0f closure=%s hit=%s point=%s"),Y,Z,*Closure->GetActorLocation().ToString(),*GetNameSafe(Hit.GetActor()),*Hit.ImpactPoint.ToString());
    TCHECK(Blocked&&Hit.GetActor()==Closure,"Gap in market gate fence");
   }
   UE_LOG(LogTemp,Display,TEXT("MarketClosureAudit: 237 fence crossing sweeps passed"));
  }
  const FVector G=BattleTutorialData::Gate;
  TCHECK(!T->TryStart(G-FVector(200,0,-500),G+FVector(200,0,500)),"Started above gate");
  TCHECK(!T->TryStart(G-FVector(200,-500,0),G+FVector(200,500,0)),"Started beside gate");
  TCHECK(B->Dismount(),"Practice dismount failed");auto* Foot=Cast<ABattleRider>(GetPawn());TCHECK(Foot&&Foot->MountBike(),"Practice remount failed");
  B=Cast<ABattleBike>(GetPawn());TCHECK(B,"Bike possession lost");B->Ride->Gear=2;
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleTutorialMarket"))){
   const FVector A=BattleTutorialBlock::MarketApproach[0],D=BattleTutorialBlock::MarketApproach[1]-A;
   B->SetActorLocationAndRotation(A+FVector(0,0,98),D.Rotation(),false,nullptr,ETeleportType::TeleportPhysics);B->Ride->StopMovementImmediately();B->Ride->Speed=0;B->Ride->bForceNextFloorCheck=true;
  }
  TutorialPrevious=B->GetActorLocation();TutorialStage=1;
 }
 if(TutorialStage==1){
  TutorialClock+=Dt;TCHECK(TutorialClock<180,"Practice route timed out");
  const FVector P=B->GetActorLocation();TutorialDistance+=FVector::Dist2D(P,TutorialPrevious);TutorialPrevious=P;
  TCHECK(B->Ride->Wipeouts==0,"Practice route caused a wipeout");
  if(!M->bTutorialActive){FlushPressedKeys();TCHECK(TutorialDistance>7000&&M->StartCountdown>0&&B->HornUses==5&&B->PistolAmmo==17,"Gate crossing or starting supplies incorrect");TutorialStage=2;TutorialClock=0;return;}
  TCHECK(M->RunElapsed==0&&M->TimeRemaining==M->Difficulty.TimeLimitSeconds,"Clock changed during actual practice ride");
  const bool Market=FParse::Param(FCommandLine::Get(),TEXT("BattleTutorialMarket"));
  if(Market&&FVector::Dist2D(P,BattleTutorialBlock::MarketApproach[UE_ARRAY_COUNT(BattleTutorialBlock::MarketApproach)-1])<130){TCHECK(TutorialDistance>3000&&M->bTutorialActive&&M->RunElapsed==0,"Market ride failed to preserve practice state");End(true,TEXT("Actual keyboard ride down mapped market approach preserves untimed practice"));return;}
  TArray<FVector> Points;if(Market){for(const FVector& Q:BattleTutorialBlock::MarketApproach)Points.Add(Q);}else if(FParse::Param(FCommandLine::Get(),TEXT("BattleTutorialAlternate"))){for(const FVector& Q:BattleTutorialBlock::Alternate)Points.Add(Q);}else{for(const FVector& Q:BattleTutorialData::Road)Points.Add(Q);}if(!Market)Points.Add(BattleTutorialData::Gate+FVector(600,0,0));
  float Best=MAX_flt;int Segment=0;FVector Closest;
  for(int I=1;I<Points.Num();I++){FVector A=Points[I-1],C=Points[I],V=P;A.Z=C.Z=V.Z=0;const FVector Q=FMath::ClosestPointOnSegment(V,A,C);const float D=FVector::Dist2D(V,Q);if(D<Best){Best=D;Segment=I-1;Closest=Q;}}
  TutorialMaxError=FMath::Max(TutorialMaxError,Best);if(Best>=200)UE_LOG(LogTemp,Display,TEXT("Tutorial deviation: position=%s segment=%d"),*P.ToString(),Segment);TCHECK(Best<200,"Practice steering left the road");
  // Account for travel and eased-steering response before the next correction.
  // Still drives real keys; no position/yaw writes or relaxed route bounds.
  FVector Target=Closest;float Ahead=170+B->Ride->Speed*.125f;
  for(int I=Segment+1;I<Points.Num();I++){const float D=FVector::Dist2D(Target,Points[I]);if(D>=Ahead){Target=FMath::Lerp(Target,Points[I],Ahead/D);break;}Target=Points[I];Ahead-=D;}
  static double NextTrace=0;
  if(P.Y< -4000&&GetWorld()->GetTimeSeconds()>=NextTrace){NextTrace=GetWorld()->GetTimeSeconds()+.25;UE_LOG(LogTemp,Display,TEXT("TutorialSteering: dt=%.4f position=%s yaw=%.2f target=%s speed=%.1f steer=%.3f floor=%s"),Dt,*P.ToString(),B->GetActorRotation().Yaw,*Target.ToString(),B->Ride->Speed,B->Ride->SmoothedSteer,*GetNameSafe(B->Ride->CurrentFloor.HitResult.GetActor()));}
  const float TurnRate=FMath::Lerp(180.f,85.f,FMath::Clamp(B->Ride->Speed/1600.f,0.f,1.f));
  const float PredictedYaw=B->GetActorRotation().Yaw+B->Ride->SmoothedSteer*TurnRate*.1f;
  const float Error=FMath::FindDeltaAngleDegrees(PredictedYaw,(Target-P).Rotation().Yaw);
  auto Key=[&](FKey K,bool Down){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
  Key(EKeys::W,true);Key(EKeys::A,Error< -2);Key(EKeys::D,Error>2);
 }else if(TutorialStage==2){
  TutorialClock+=Dt;
  static bool CountdownControlsChecked=false;
  if(TutorialClock<.2f){for(FKey K:{EKeys::W,EKeys::A,EKeys::SpaceBar})InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,IE_Pressed,1.f,false,0));return;}
  if(!CountdownControlsChecked){TCHECK(M->StartCountdown>0&&B->Ride->Pedal>0&&B->Ride->Steer<0&&B->Ride->Brake>0,"Countdown blocked pedal, steering or brake input");CountdownControlsChecked=true;FString Folder;if(FParse::Value(FCommandLine::Get(),TEXT("BattleCountdownReviewDir="),Folder)){IFileManager::Get().MakeDirectory(*Folder,true);FScreenshotRequest::RequestScreenshot(Folder/TEXT("countdown.png"),true,false);}FlushPressedKeys();UE_LOG(LogTemp,Display,TEXT("CountdownControls: pedal, steering and brake stay active"));}
  static bool GoCaptured=false;if(TutorialClock>3.3f&&!GoCaptured){GoCaptured=true;FString Folder;if(FParse::Value(FCommandLine::Get(),TEXT("BattleCountdownReviewDir="),Folder))FScreenshotRequest::RequestScreenshot(Folder/TEXT("go.png"),true,false);}
  if(TutorialClock<4.2f)return;
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleMarketClosureReview"))){
   TActorIterator<ABattleMarketClosure> Closure(GetWorld());TCHECK(!Closure,"Market setup fence remained after tutorial");
   UE_LOG(LogTemp,Display,TEXT("MarketClosureAudit: removed after gateway start"));
  }
  TCHECK(M->StartCountdown==0&&M->RunElapsed>0&&M->TimeRemaining<M->Difficulty.TimeLimitSeconds,"Timer did not start after gateway countdown");
  const float Left=M->TimeRemaining;TCHECK(!T->TryStart(BattleTutorialData::Gate-FVector(100,0,0),BattleTutorialData::Gate+FVector(100,0,98))&&M->TimeRemaining==Left,"Crossing gate again reset the run");
  End(true,FParse::Param(FCommandLine::Get(),TEXT("BattleCountdownControlsAudit"))?TEXT("Gate event, active countdown pedal/steer/brake keys, timer start and no gate reset pass"):TEXT("Untimed practice, actual road ride, controllable gate countdown and single-start timer pass"));
 }
#undef TCHECK
#endif
}
