#include "BattleSpirit.h"
#include "BattleSpiritData.h"
#include "BattleBike.h"
#include "PiedmontBike.h"
#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
ABattleSpirit::ABattleSpirit(){
 PrimaryActorTick.bCanEverTick=true;
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("SpiritRoot"));
 SetCanBeDamaged(false);SetActorEnableCollision(false);
}
bool ABattleSpirit::IsLiveRun() const {
 const auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));
 return M&&!M->bTutorialActive&&!M->bRunEnded&&M->StartCountdown<=0&&M->TimeRemaining>0&&!UGameplayStatics::IsGamePaused(this);
}
ABattleBike* ABattleSpirit::MountedRider() const {
 auto* B=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(this,0));
 if(!B||!B->GetController()||B->bParked||B->RiderHealth<=0||B->RespawnRemaining>0||B->StunRemaining>0||B->Ride->Recovery>0)return nullptr;
 for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->ContainsBike(B->GetActorLocation()))return nullptr;
 return B;
}
bool ABattleSpirit::TryApproach(float Roll){
 if(State!=EBattleSpiritState::Untried||!IsLiveRun()||!FMath::IsFinite(Roll)||Roll<0||Roll>=1)return false;
 State=EBattleSpiritState::Absent;
 if(!MountedRider()||Roll>=.15f)return false;
 State=EBattleSpiritState::Appearing;Age=Fade=0;return true;
}
void ABattleSpirit::Cancel(){
 if(State==EBattleSpiritState::Appearing||State==EBattleSpiritState::Active){State=EBattleSpiritState::Fading;Fade=0;}
}
void ABattleSpirit::CancelForRider(const UObject* Context){
 if(!Context||!Context->GetWorld())return;
 for(TActorIterator<ABattleSpirit> It(Context->GetWorld());It;++It)It->Cancel();
}
bool ABattleSpirit::TryCatch(){
 if(State!=EBattleSpiritState::Active||!IsLiveRun())return false;
 auto* B=MountedRider();if(!B){Cancel();return false;}
 if(FVector::Dist2D(B->GetActorLocation(),GetActorLocation())>160||FMath::Abs(B->GetActorLocation().Z-GetActorLocation().Z)>160)return false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(SpiritCatch),false,this);Q.AddIgnoredActor(B);FHitResult H;
 if(GetWorld()->LineTraceSingleByChannel(H,B->GetActorLocation(),GetActorLocation(),ECC_Visibility,Q))return false;
 auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));
 B->RestoreRiderHealth(100);
 const float Added=FMath::Max(0.f,M->Difficulty.TimeLimitSeconds-M->TimeRemaining);
 if(Added>0)M->AdjustRunTime(Added,TEXT("TIME RESTORED"));
 else {M->LastTimeDelta=0;M->TimeNotice=TEXT("TIME RESTORED");M->TimeNoticeRemaining=2.5f;}
 Rewards++;Cancel();return true;
}
void ABattleSpirit::AdvanceEncounter(float Dt){
 if(!FMath::IsFinite(Dt)||Dt<=0||UGameplayStatics::IsGamePaused(this))return;
 if(State==EBattleSpiritState::Fading){Fade+=Dt;if(Fade>=1.5f)State=EBattleSpiritState::Resolved;return;}
 if(State!=EBattleSpiritState::Appearing&&State!=EBattleSpiritState::Active)return;
 if(!IsLiveRun()||!MountedRider()){Cancel();return;}
 Age+=Dt;
 if(Age>=12){Cancel();return;}
 if(Age>=1.5f)State=EBattleSpiritState::Active;
}
void ABattleSpirit::Tick(float Dt){
 Super::Tick(Dt);
 if(!bPresentationReady)return;
 auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);
 if(State==EBattleSpiritState::Untried&&Pawn&&FVector::Dist2D(Pawn->GetActorLocation(),ApproachPoint)<700&&FMath::Abs(Pawn->GetActorLocation().Z-ApproachPoint.Z)<200){
  if(ChaseRoute.Num()>1){SetActorLocation(ChaseRoute[0]);TryApproach(FMath::FRand());}
 }
 AdvanceEncounter(Dt);
 if(State==EBattleSpiritState::Active){
  RouteDistance+=110*Dt;float Left=RouteDistance;
  for(int32 I=1;I<ChaseRoute.Num();I++){const FVector D=ChaseRoute[I]-ChaseRoute[I-1];const float Length=D.Size();if(Left<=Length){SetActorLocation(ChaseRoute[I-1]+D.GetSafeNormal()*Left);SetActorRotation(D.Rotation());break;}Left-=Length;if(I==ChaseRoute.Num()-1)SetActorLocation(ChaseRoute.Last());}
  TryCatch();
 }
}

void ABattleSpirit::BeginPlay(){Super::BeginPlay();ApproachPoint=BattleSpiritData::Approach;for(const FVector& P:BattleSpiritData::Chase)ChaseRoute.Add(P);}
