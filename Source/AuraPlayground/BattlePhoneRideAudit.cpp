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
void TickBattlePhoneRideAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TArray<FVector> Points;FVector Previous;int Next=1;float Clock=0,Distance=0,Still=0;bool Started=false,Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;
 auto Key=[&](FKey K,bool Down){if(PC->IsInputKeyDown(K)!=Down)PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto* B=Cast<ABattleBike>(PC->GetPawn());auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(PC));
 auto End=[&](bool Pass,const TCHAR* Why){for(FKey K:{EKeys::W,EKeys::A,EKeys::D,EKeys::SpaceBar})Key(K,false);S.Done=true;UE_LOG(LogTemp,Display,TEXT("PhoneRideAudit: {\"passed\":%s,\"reason\":\"%s\",\"seconds\":%.2f,\"distance_cm\":%.2f,\"waypoint\":%d,\"points\":%d}"),Pass?TEXT("true"):TEXT("false"),Why,S.Clock,S.Distance,S.Next,S.Points.Num());PC->ConsoleCommand(TEXT("quit"));};
 if(!B||!M||!M->Quest){End(false,TEXT("Missing mounted player or quest"));return;}auto* Q=M->Quest.Get();
 if(!S.Started){
  if(!Q->bReady){if(PC->GetWorld()->GetTimeSeconds()>20)End(false,TEXT("Phone placement never became ready"));return;}
  auto* Path=UNavigationSystemV1::FindPathToLocationSynchronously(PC,B->GetActorLocation(),Q->ArtifactLocation,B);
  if(!Path||!Path->IsValid()||Path->IsPartial()||Path->PathPoints.Num()<2){End(false,TEXT("No complete navigation path to phone"));return;}
  S.Points=Path->PathPoints;S.Points.Last()=Q->ArtifactLocation;S.Previous=B->GetActorLocation();S.Started=true;
  if(M->Enemies)M->Enemies->bFreezeSpawns=true;
  for(TActorIterator<APiedmontTrafficDirector> I(PC->GetWorld());I;++I)I->DesiredPopulation=0;
  for(TActorIterator<ABattleZombie> I(PC->GetWorld());I;++I)I->Destroy();for(TActorIterator<APiedmontPedestrian> I(PC->GetWorld());I;++I)I->Destroy();
  UE_LOG(LogTemp,Display,TEXT("PhoneRideFixture: phone=%s way=%s start=%s points=%d"),*Q->ArtifactLocation.ToString(),*Q->ArtifactWay,*B->GetActorLocation().ToString(),S.Points.Num());
 }
 S.Clock+=Dt;const FVector P=B->GetActorLocation();const float Step=FVector::Dist2D(P,S.Previous);S.Distance+=Step;S.Previous=P;S.Still=Step<Dt*12?S.Still+Dt:0;
 if(Q->bCollected){End(true,TEXT("Rode from park start and collected phone through proximity; no teleport"));return;}
 if(B->bCrashActive||B->RiderHealth<=0){End(false,TEXT("Ride interrupted by crash or death"));return;}
 if(S.Clock>150||S.Still>12){UE_LOG(LogTemp,Display,TEXT("PhoneRideFailure: position=%s target=%s speed=%.2f yaw=%.2f"),*P.ToString(),*S.Points[S.Next].ToString(),B->Ride->Speed,B->GetActorRotation().Yaw);End(false,TEXT("Guided input stalled or timed out; inspect controller and world"));return;}
 while(S.Next<S.Points.Num()-1&&FVector::Dist2D(P,S.Points[S.Next])<160)S.Next++;
 const float Angle=FMath::FindDeltaAngleDegrees(B->GetActorRotation().Yaw,(S.Points[S.Next]-P).Rotation().Yaw);
 Key(EKeys::A,Angle< -5);Key(EKeys::D,Angle>5);Key(EKeys::W,true);Key(EKeys::SpaceBar,FMath::Abs(Angle)>45&&B->Ride->Speed>220);
#endif
}
