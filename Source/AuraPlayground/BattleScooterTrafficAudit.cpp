#include "BattleBike.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

// The moving BeltLine scooters are the one placed prop in this project that used
// to sit at a fixed height above a path spline with no ground trace, so wherever
// the spline ran above the pavement they hovered and slid along in mid-air.
// Elliott reported that twice as "a flying log" before anyone could name it, and
// it was only found by reading BattleScooterTraffic for a LineTrace that was not
// there. This asserts the property directly so it cannot come back quietly.
//
// The convention every other prop follows is origin = surface + 92 cm, so the
// check is that a downward trace under each rider lands 92 cm below its origin.
void TickBattleScooterTrafficAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;float Clock=0,FirstSample=-1;int32 Samples=0;TArray<FVector> Starts;int32 Moved=0,Worst=0;float WorstGap=0;bool Done=false;};
 static FState S;
 auto* World=PC->GetWorld();if(S.World!=World){S=FState();S.World=World;}if(S.Done||World->GetTimeSeconds()<6)return;S.Clock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Why){S.Done=true;UE_LOG(LogTemp,Display,TEXT("ScooterTrafficAudit: {\"passed\":%s,\"reason\":\"%s\",\"riders\":%d,\"moved\":%d,\"worst_gap_cm\":%.2f}"),Pass?TEXT("true"):TEXT("false"),Why,S.Starts.Num(),S.Moved,S.WorstGap);PC->ConsoleCommand(TEXT("quit"));};
 if(S.Clock>40){Finish(false,TEXT("Timed out gathering riders"));return;}

 TArray<AActor*> Riders;
 for(TActorIterator<AActor> It(World);It;++It)if(It->ActorHasTag(TEXT("BattleScooterRider")))Riders.Add(*It);
 if(Riders.Num()<8){Finish(false,TEXT("Fewer than eight moving scooters in the world"));return;}

 // Sample twice a second apart: these are traffic, so they must actually move.
 if(S.Samples==0){S.FirstSample=S.Clock;for(auto* R:Riders)S.Starts.Add(R->GetActorLocation());++S.Samples;return;}
 if(S.Samples==1){if(S.Clock-S.FirstSample<1.f)return;++S.Samples;}
 for(int32 I=0;I<Riders.Num()&&I<S.Starts.Num();++I)if(FVector::Dist2D(Riders[I]->GetActorLocation(),S.Starts[I])>40.f)++S.Moved;

 for(int32 I=0;I<Riders.Num();++I){
  const FVector Location=Riders[I]->GetActorLocation();
  FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(ScooterTrafficGround),false,Riders[I]);
  // No surface under a rider is not a failure - a scooter crossing water or a
  // gap has nothing to stand on and is skipped rather than scored.
  if(!World->LineTraceSingleByChannel(Hit,Location+FVector(0,0,60),Location-FVector(0,0,400),ECC_Visibility,Q))continue;
  if(Hit.ImpactNormal.Z<.6f)continue;
  const float Gap=FMath::Abs((Location.Z-92.f)-Hit.ImpactPoint.Z);
  if(Gap>S.WorstGap){S.WorstGap=Gap;S.Worst=I;}
 }
 if(S.Moved<Riders.Num()){const FString Why=FString::Printf(TEXT("Only %d of %d scooters moved in one second"),S.Moved,Riders.Num());Finish(false,*Why);return;}
 if(S.WorstGap>20.f){const FString Why=FString::Printf(TEXT("Scooter %d floats %.1f cm above its surface"),S.Worst,S.WorstGap);Finish(false,*Why);return;}
 Finish(true,TEXT("every moving scooter sits on its surface and travels"));
#endif
}
