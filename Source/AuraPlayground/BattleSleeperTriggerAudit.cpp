#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

void TickBattleSleeperTriggerAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<APiedmontPedestrian> Person,Player;FVector Near,Far;int Phase=0;float Clock=0,Total=0,Closest=10000,StartDistance=0;bool Done=false;};
 static FState S;if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}
 if(S.Done||PC->GetWorld()->GetTimeSeconds()<6)return;S.Clock+=Dt;S.Total+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){S.Done=true;UE_LOG(LogTemp,Display,TEXT("BattleSleeperTriggerAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\",\"start_distance\":%.3f,\"closest_distance\":%.3f}"),Pass?TEXT("true"):TEXT("false"),S.Phase,Reason,S.StartDistance,S.Closest);
 UE_LOG(LogTemp,Display,TEXT("SleeperTriggerEnd: sleep_phase=%d location=%s"),S.Person.IsValid()?S.Person->SleepPhase:-1,S.Person.IsValid()?*S.Person->GetActorLocation().ToString():TEXT("missing"));UKismetSystemLibrary::QuitGame(PC,PC,EQuitPreference::Quit,false);};
#define REQUIRE_TRIGGER(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 REQUIRE_TRIGGER(S.Total<65,"Timed out");
 if(S.Phase==0){
  for(TActorIterator<APiedmontPedestrian> It(PC->GetWorld());It;++It)if(It->ActorHasTag(TEXT("AmbientSleeperReview"))){S.Person=*It;break;}
  auto* P=S.Person.Get();REQUIRE_TRIGGER(P&&P->SleepPhase==1,"Ambient sleeper missing or awake");
  auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(PC->GetWorld());REQUIRE_TRIGGER(Nav,"Navigation missing");
  bool Found=false;
  for(int Try=0;Try<80&&!Found;++Try){
   FNavLocation N;if(!Nav->GetRandomReachablePointInRadius(P->GetActorLocation(),420,N))continue;
   const float D=FVector::Dist2D(N.Location,P->GetActorLocation());if(D<300)continue;
   FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(SleeperTriggerPlacement),false,P);const FVector Goal=N.Location+FVector(0,0,90);
   if(PC->GetWorld()->OverlapBlockingTestByChannel(Goal,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,86),Q)||PC->GetWorld()->LineTraceSingleByChannel(Hit,P->GetActorLocation(),Goal,ECC_Visibility,Q))continue;
   S.Near=Goal;Found=true;
  }
  REQUIRE_TRIGGER(Found,"No clear reachable player approach");
  S.Far=S.Near+FVector(0,0,1200); // Distance and height gate establish an ineligible approach first.
  auto* Player=PC->GetWorld()->SpawnActor<APiedmontPedestrian>(S.Far,FRotator::ZeroRotator);REQUIRE_TRIGGER(Player,"Player fixture spawn failed");
  PC->Possess(Player);Player->PauseRemaining=100;Player->GetCharacterMovement()->DisableMovement();S.Player=Player;
  P->AmbientWakeChance=1;S.Phase=1;S.Clock=0;return;
 }
 auto* P=S.Person.Get();auto* Player=S.Player.Get();REQUIRE_TRIGGER(P&&Player&&!P->bDead,"Actor lost");
 auto Place=[&](FVector Point){Player->SetActorLocation(Point,false,nullptr,ETeleportType::TeleportPhysics);};
 if(S.Phase==1&&S.Clock>.5f){REQUIRE_TRIGGER(P->SleepPhase==1,"Vertical separation did not suppress wake");Place(S.Near);S.Phase=2;S.Clock=0;return;}
 if(S.Phase==2&&S.Clock>.5f){
  REQUIRE_TRIGGER(P->SleepPhase==1,"Ineligible entry incorrectly rerolled without leaving");
  Place(S.Near+FVector(1500,0,1200));S.Phase=3;S.Clock=0;return;
 }
 if(S.Phase==3&&S.Clock>.5f){Place(S.Near);S.StartDistance=FVector::Dist2D(S.Near,P->GetActorLocation());S.Phase=4;S.Clock=0;return;}
 if(S.Phase==4&&S.Clock>.5f){REQUIRE_TRIGGER(P->SleepPhase==2,"Automatic eligible approach did not wake");S.Phase=5;S.Clock=0;return;}
 if(S.Phase==5){
  S.Closest=FMath::Min(S.Closest,float(FVector::Dist2D(Player->GetActorLocation(),P->GetActorLocation())));
  if(S.Clock>8){REQUIRE_TRIGGER(S.Closest<S.StartDistance-70,"Automatic chase did not approach player");S.Phase=6;S.Clock=0;return;}
 }
 if(S.Phase==6&&P->SleepPhase==1){S.Phase=7;S.Clock=0;return;}
 if(S.Phase==7){
  REQUIRE_TRIGGER(P->SleepPhase==1,"Staying nearby caused repeated wake");
  if(S.Clock>2){Place(P->GetActorLocation()+FVector(1500,0,1200));S.Phase=8;S.Clock=0;return;}
 }
 if(S.Phase==8&&S.Clock>.5f){Place(P->GetActorLocation()+FVector(250,0,0));S.Phase=9;S.Clock=0;return;}
 if(S.Phase==9&&S.Clock>1){REQUIRE_TRIGGER(P->SleepPhase==1,"Cooldown re-entry woke sleeper");Finish(true,TEXT("height gate, fresh approach, automatic chase, sleep return and cooldown passed"));}
#undef REQUIRE_TRIGGER
#endif
}
