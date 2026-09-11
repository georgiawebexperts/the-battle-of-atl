#include "BattleBike.h"
#include "BattleRider.h"
#include "BattlePolice.h"
#include "BattleZombie.h"
#include "BattleQuest.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

void ABattleLabMode::RecordGunfire(){
 if(bRunEnded||StartCountdown>0||UGameplayStatics::IsGamePaused(this))return;
 Trouble=FMath::Min(12.f,Trouble+.75f);QuietTime=0;
}
bool ABattleLabMode::RecordAssault(AActor* Victim){
 if(!IsValid(Victim)||!Victim->ActorHasTag(TEXT("PiedmontTraffic"))||bRunEnded||StartCountdown>0||UGameplayStatics::IsGamePaused(this)||AssaultVictims.Contains(Victim))return false;
 AssaultVictims.Add(Victim);PeopleHit++;Trouble=FMath::Min(12.f,Trouble+3);QuietTime=0;
 if(PeopleHit>=3){bPoliceAlert=true;PoliceDelay=0;}
 UE_LOG(LogTemp,Display,TEXT("BattleTrouble: people=%d police=%d heat=%.2f"),PeopleHit,bPoliceAlert,Trouble);return true;
}
void ABattleLabMode::TickTrouble(float Dt){
 if(bRunEnded||StartCountdown>0||UGameplayStatics::IsGamePaused(this))return;
 QuietTime+=Dt;if(QuietTime>20)Trouble=FMath::Max(0.f,Trouble-Dt*.15f);
 auto* Mode=Cast<ABattleParkMode>(this);auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);if(!Mode||!Pawn||(Mode->Enemies&&Mode->Enemies->bFreezeSpawns))return;
 auto* Bike=Cast<ABattleBike>(Pawn);if(auto* Person=Cast<ABattleRider>(Pawn))Bike=Person->ParkedBike;if(!Bike||Bike->RiderHealth<=0||Bike->RespawnRemaining>0)return;
 int32 Live=0;for(TActorIterator<ABattlePolice> It(GetWorld());It;++It)if(!It->bDead){if(FVector::DistSquared2D(It->GetActorLocation(),Pawn->GetActorLocation())>FMath::Square(7500.f))It->Destroy();else Live++;}
 PoliceDelay-=Dt;if(PoliceDelay>0)return;PoliceDelay=12;
 if(Live>=2||(!bPoliceAlert&&(Trouble<6||FMath::FRand()>.3f)))return;
 auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());if(!Nav)return;
 for(int32 Try=0;Try<16;Try++){
  FNavLocation Point;if(!Nav->GetRandomReachablePointInRadius(Pawn->GetActorLocation(),2400,Point)||FVector::Dist2D(Point.Location,Pawn->GetActorLocation())<1100)continue;
  const FVector Spot=Point.Location+FVector(0,0,90);bool Wet=false;for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->ContainsBike(Spot)){Wet=true;break;}if(Wet)continue;
  FActorSpawnParameters P;P.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
  if(auto* Officer=GetWorld()->SpawnActor<ABattlePolice>(Spot,(Pawn->GetActorLocation()-Spot).Rotation(),P)){PoliceSpawned++;UE_LOG(LogTemp,Display,TEXT("BattlePolice: spawned=%d"),PoliceSpawned);return;}
 }
 PoliceDelay=2;
}
bool ABattleBike::ApplyTaser(){
 auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));
 if(!Mode||Mode->StartCountdown>0||Mode->bRunEnded||UGameplayStatics::IsGamePaused(this)||RiderHealth<=0||RespawnRemaining>0||StunRemaining>0||TaserGrace>0||Ride->Recovery>0)return false;
 if(!bParked)Dismount();
 StunLabel=TEXT("TASED");StunRemaining=3;TaserGrace=12;TaserHits++;ReloadTimer=0;Ride->BoostRemaining=0;Ride->Speed=Ride->Pedal=Ride->Steer=Ride->Brake=0;Ride->StopMovementImmediately();
 if(auto* Person=Cast<ABattleRider>(UGameplayStatics::GetPlayerPawn(this,0))){Person->ReloadRemaining=0;Person->GetCharacterMovement()->StopMovementImmediately();Person->GetCharacterMovement()->DisableMovement();}
 Mode->AdjustRunTime(-10,TEXT("TASED"));RideImpact(.8f);
 UE_LOG(LogTemp,Display,TEXT("BattleTaser: hit=%d parked=%d time_penalty=10 stun=3"),TaserHits,bParked);return true;
}
void ABattleBike::UpdateStun(float Dt){
 const auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));if(Mode&&(Mode->bRunEnded||Mode->StartCountdown>0))return;
 TaserGrace=FMath::Max(0.f,TaserGrace-Dt);if(StunRemaining<=0)return;
 // Retry a safe dismount if another actor briefly occupied the exit space.
 if(!bParked&&RiderHealth>0)Dismount();
 auto* Person=Cast<ABattleRider>(UGameplayStatics::GetPlayerPawn(this,0));
 if(Person&&Person->ParkedBike==this){Person->GetCharacterMovement()->StopMovementImmediately();Person->GetCharacterMovement()->DisableMovement();}
 StunRemaining=FMath::Max(0.f,StunRemaining-Dt);
 if(StunRemaining<=0&&Person&&Person->ParkedBike==this&&RiderHealth>0&&RespawnRemaining<=0){Person->ConsumeMovementInputVector();Person->GetCharacterMovement()->SetMovementMode(Person->bSwimming?MOVE_Swimming:MOVE_Walking);}
}
