#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleQuest.h"
#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

float ABattleBike::TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer){
 // An empty parked bicycle cannot take damage on behalf of its rider.
 return bParked?0:ApplyRiderDamage(Amount);
}
float ABattleBike::ApplyRiderDamage(float Amount){
 auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));
 if(!FMath::IsFinite(Amount)||Amount<=0||RiderHealth<=0||DamageGrace>0||RespawnRemaining>0||
    (Mode&&(Mode->StartCountdown>0||Mode->bRunEnded))||UGameplayStatics::IsGamePaused(this))return 0;
 const float Applied=FMath::Min(RiderHealth,Amount);RiderHealth-=Applied;HurtCooldown=5;
 RideImpact(.35f);
 auto* Person=Cast<ABattleRider>(UGameplayStatics::GetPlayerPawn(this,0));
 if(Person&&Person->ParkedBike==this)Person->Health=RiderHealth;
 if(RiderHealth<=0){
  Deaths++;RespawnRemaining=2;
  if(auto* Park=Cast<ABattleParkMode>(Mode))if(Park->Quest)Park->Quest->ResetAfterDeath();
  if(Mode){Mode->TimeRemaining=FMath::Max(0.f,Mode->TimeRemaining-10);if(Mode->TimeRemaining<=0)Mode->bRunEnded=true;}
  // Override an in-progress water/traffic recovery so it cannot teleport later.
  Ride->Recovery=0;Ride->Wipeout(TEXT("Wipeout | checkpoint -10 seconds"));
  Ride->Pedal=Ride->Steer=Ride->Brake=0;Ride->DisableMovement();
  if(Person&&Person->ParkedBike==this){Person->GetCharacterMovement()->StopMovementImmediately();Person->GetCharacterMovement()->DisableMovement();}
 }
 return Applied;
}
float ABattleBike::RestoreRiderHealth(float Amount){
 const auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));
 if(!FMath::IsFinite(Amount)||Amount<=0||RiderHealth<=0||RiderHealth>=100||RespawnRemaining>0||UGameplayStatics::IsGamePaused(this)||(Mode&&(Mode->StartCountdown>0||Mode->bRunEnded)))return 0;
 const float Applied=FMath::Min(100-RiderHealth,Amount);RiderHealth+=Applied;
 if(auto* Person=Cast<ABattleRider>(UGameplayStatics::GetPlayerPawn(this,0)))if(Person->ParkedBike==this)Person->Health=RiderHealth;
 return Applied;
}
void ABattleBike::UpdateHealth(float Dt){
 PickupNoticeRemaining=FMath::Max(0.f,PickupNoticeRemaining-Dt);
 auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));
 if(Mode&&(Mode->StartCountdown>0||Mode->bRunEnded))return;
 if(RespawnRemaining>0){
  RespawnRemaining=FMath::Max(0.f,RespawnRemaining-Dt);
  if(RespawnRemaining<=0&&!RecoverAtCheckpoint())RespawnRemaining=.2f;
  return;
 }
 DamageGrace=FMath::Max(0.f,DamageGrace-Dt);
 // Only the portion of this frame after the cooldown contributes regeneration.
 const float RegenDt=FMath::Max(0.f,Dt-HurtCooldown);HurtCooldown=FMath::Max(0.f,HurtCooldown-Dt);
 if(RiderHealth>0)RiderHealth=FMath::Min(100.f,RiderHealth+4*RegenDt);
 if(auto* Person=Cast<ABattleRider>(UGameplayStatics::GetPlayerPawn(this,0)))if(Person->ParkedBike==this)Person->Health=RiderHealth;
}
bool ABattleBike::RecoverAtCheckpoint(){
 auto* PC=UGameplayStatics::GetPlayerController(this,0);if(!PC)return false;
 auto* Person=Cast<ABattleRider>(PC->GetPawn());
 if(Person&&Person->ParkedBike!=this)return false;
 FVector Location=CheckpointTransform.GetLocation();const FRotator Rotation=CheckpointTransform.Rotator();
 // Respect actual current capsule clearance (crowds can occupy the checkpoint).
 if(!GetWorld()->FindTeleportSpot(this,Location,Rotation))return false;
 Ride->StopMovementImmediately();Ride->Speed=Ride->Pedal=Ride->Steer=Ride->Brake=0;Ride->Recovery=0;Ride->BoostRemaining=0;
 SetActorLocationAndRotation(Location,Rotation,false,nullptr,ETeleportType::TeleportPhysics);
 if(Person){Person->SaveWeapon();PC->Possess(this);Person->Destroy();}else if(PC->GetPawn()!=this)PC->Possess(this);
 bParked=false;LeanAngle=0;Ride->SmoothedSteer=0;StunRemaining=0;TaserGrace=FMath::Max(TaserGrace,2.f);RiderHealth=100;DamageGrace=2;HurtCooldown=5;RespawnRemaining=0;
 Ride->LastSafeLocation=Location;Ride->SetMovementMode(MOVE_Walking);Ride->bForceNextFloorCheck=true;
 Rider->SetVisibility(!bFirstPerson);PC->SetControlRotation(Rotation);
 UE_LOG(LogTemp,Display,TEXT("BattleHealth: recovered at %s; deaths=%d"),*CheckpointName,Deaths);
 return true;
}
