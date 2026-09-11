#include "BattleMacController.h"
#include "BattleTutorial.h"
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
  TCHECK(M->bTutorialActive&&M->StartCountdown==0&&M->RunElapsed==0&&M->TimeRemaining==M->Difficulty.TimeLimitSeconds,"Practice timer ran before gateway");
  TCHECK(!M->AdjustRunTime(30,TEXT("practice"))&&B->ApplyRiderDamage(10)==0,"Practice changed time or damaged rider");
  for(int I=0;I<UE_ARRAY_COUNT(BattleTutorialBlock::BarrierCenters);I++){
   const FVector C=BattleTutorialBlock::BarrierCenters[I]+FVector(0,0,98),D=BattleTutorialBlock::BarrierDirections[I];FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(PracticeFence),false,B);
   TCHECK(GetWorld()->SweepSingleByChannel(Hit,C-D*220,C+D*220,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(32),Q)&&Hit.GetActor()==T,"Construction fence does not physically block the road");
  }
  const FVector G=BattleTutorialData::Gate;
  TCHECK(!T->TryStart(G-FVector(200,0,-500),G+FVector(200,0,500)),"Started above gate");
  TCHECK(!T->TryStart(G-FVector(200,-500,0),G+FVector(200,500,0)),"Started beside gate");
  TCHECK(B->Dismount(),"Practice dismount failed");auto* Foot=Cast<ABattleRider>(GetPawn());TCHECK(Foot&&Foot->MountBike(),"Practice remount failed");
  B=Cast<ABattleBike>(GetPawn());TCHECK(B,"Bike possession lost");B->Ride->Gear=2;TutorialPrevious=B->GetActorLocation();TutorialStage=1;
 }
 if(TutorialStage==1){
  TutorialClock+=Dt;TCHECK(TutorialClock<180,"Practice route timed out");
  const FVector P=B->GetActorLocation();TutorialDistance+=FVector::Dist2D(P,TutorialPrevious);TutorialPrevious=P;
  TCHECK(B->Ride->Wipeouts==0,"Practice route caused a wipeout");
  if(!M->bTutorialActive){FlushPressedKeys();TCHECK(TutorialDistance>7000&&M->StartCountdown>0&&B->HornUses==5&&B->PistolAmmo==10,"Gate crossing or starting supplies incorrect");TutorialStage=2;TutorialClock=0;return;}
  TCHECK(M->RunElapsed==0&&M->TimeRemaining==M->Difficulty.TimeLimitSeconds,"Clock changed during actual practice ride");
  TArray<FVector> Points;if(FParse::Param(FCommandLine::Get(),TEXT("BattleTutorialAlternate"))){for(const FVector& Q:BattleTutorialBlock::Alternate)Points.Add(Q);}else{for(const FVector& Q:BattleTutorialData::Road)Points.Add(Q);}Points.Add(BattleTutorialData::Gate+FVector(600,0,0));
  float Best=MAX_flt;int Segment=0;FVector Closest;
  for(int I=1;I<Points.Num();I++){FVector A=Points[I-1],C=Points[I],V=P;A.Z=C.Z=V.Z=0;const FVector Q=FMath::ClosestPointOnSegment(V,A,C);const float D=FVector::Dist2D(V,Q);if(D<Best){Best=D;Segment=I-1;Closest=Q;}}
  TutorialMaxError=FMath::Max(TutorialMaxError,Best);if(Best>=200)UE_LOG(LogTemp,Display,TEXT("Tutorial deviation: position=%s segment=%d"),*P.ToString(),Segment);TCHECK(Best<200,"Practice steering left the road");
  FVector Target=Closest;float Ahead=170;
  for(int I=Segment+1;I<Points.Num();I++){const float D=FVector::Dist2D(Target,Points[I]);if(D>=Ahead){Target=FMath::Lerp(Target,Points[I],Ahead/D);break;}Target=Points[I];Ahead-=D;}
  const float Error=FMath::FindDeltaAngleDegrees(B->GetActorRotation().Yaw,(Target-P).Rotation().Yaw);
  auto Key=[&](FKey K,bool Down){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
  Key(EKeys::W,true);Key(EKeys::A,Error< -2);Key(EKeys::D,Error>2);
 }else if(TutorialStage==2){
  TutorialClock+=Dt;if(TutorialClock<4.2f)return;
  TCHECK(M->StartCountdown==0&&M->RunElapsed>0&&M->TimeRemaining<M->Difficulty.TimeLimitSeconds,"Timer did not start after gateway countdown");
  const float Left=M->TimeRemaining;TCHECK(!T->TryStart(BattleTutorialData::Gate-FVector(100,0,0),BattleTutorialData::Gate+FVector(100,0,98))&&M->TimeRemaining==Left,"Crossing gate again reset the run");
  End(true,TEXT("Untimed practice, protected clock/health, dismount/remount, actual W/A/D road ride, one-way gate countdown and single-start timer pass"));
 }
#undef TCHECK
#endif
}
