#include "BattleMacController.h"
#include "BattleZombie.h"
#include "BattleZombieLines.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "AIController.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void ABattleMacController::TickZombieAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5)return;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!Mode||!Mode->Enemies)return;
 auto* Bike=Cast<ABattleBike>(GetPawn());auto* Person=Cast<ABattleRider>(GetPawn());if(Person)Bike=Person->ParkedBike;if(!Bike)return;
 auto Finish=[&](bool Passed,const TCHAR* Reason){
  UE_LOG(LogTemp,Display,TEXT("BattleZombieAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\",\"spawned\":%d,\"desired\":%d,\"kills\":%d,\"nitro\":%.1f,\"subtitleLines\":%d}"),Passed?TEXT("true"):TEXT("false"),ZombiePhase,Reason,Mode->Enemies->Spawned,Mode->Enemies->DesiredZombies,Bike->EnemyKills,Bike->Nitro,UE_ARRAY_COUNT(BattleZombieLines::Lines));
  UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);
 };
#define VERIFY_ZOMBIE(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 ZombieAuditClock+=Dt;
 auto* Z=Cast<ABattleZombie>(ZombieAuditTarget.Get());
 if(ZombiePhase==0){
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  TActorIterator<ABattleZombie> First(GetWorld());if(First)Z=*First;
  if(Z){
   VERIFY_ZOMBIE(Mode->Enemies->DesiredZombies==Mode->Difficulty.Zombies&&Z->GetController()&&Z->GetController()->IsA<AAIController>(),"Missing difficulty-driven AI spawn");
   VERIFY_ZOMBIE(FMath::IsNearlyEqual(Z->GetCharacterMovement()->MaxWalkSpeed,Z->bSprinter?Mode->Difficulty.SprinterSpeed:Mode->Difficulty.ZombieSpeed),"Wrong zombie movement speed");
   Mode->Enemies->bFreezeSpawns=true;ZombieAuditTarget=Z;ZombieAuditInitial=Z->GetActorLocation();ZombiePhase=1;ZombieAuditClock=0;
  }
 }
 else if(ZombiePhase==1&&ZombieAuditClock>3){
  VERIFY_ZOMBIE(Z&&Z->PathRequests>0&&FVector::Dist2D(Z->GetActorLocation(),ZombieAuditInitial)>50,"AI did not navigate and move");
  Z->SetActorLocation(Bike->GetActorLocation()+Bike->GetActorForwardVector()*100-FVector(0,0,8),false,nullptr,ETeleportType::TeleportPhysics);Z->GetCharacterMovement()->StopMovementImmediately();ZombiePhase=2;ZombieAuditClock=0;
 }
 else if(ZombiePhase==2&&ZombieAuditClock>.2f){
  VERIFY_ZOMBIE(Z&&Z->bTelegraphing&&Z->Attacks==0&&Bike->RiderHealth==100,"Attack was not telegraphed");ZombiePhase=3;
 }
 else if(ZombiePhase==3&&ZombieAuditClock>1.2f){
  VERIFY_ZOMBIE(Z&&Z->Attacks==1&&FMath::IsNearlyEqual(Bike->RiderHealth,100-Z->AttackDamage)&&Bike->Ride->Wipeouts==1,"Attack did not damage and wipe out rider");
  Z->SetActorTickEnabled(false);if(auto* AI=Cast<AAIController>(Z->GetController()))AI->StopMovement();ZombiePhase=4;ZombieAuditClock=0;
 }
 else if(ZombiePhase==4&&ZombieAuditClock>2.2f){
  if(Bike->bCrashActive)return; // Current physical fall/get-up can take several seconds.
  if(!Person){VERIFY_ZOMBIE(Bike->Dismount(),"Dismount after zombie recovery failed");Person=Cast<ABattleRider>(GetPawn());}VERIFY_ZOMBIE(Person,"Missing FPS pawn");
  if(!Person->bWeaponDrawn)Person->ToggleWeapon();
  const FVector Desired=Person->GetActorLocation()+Bike->GetActorForwardVector()*700;
  FHitResult Ground;FCollisionQueryParams Q(SCENE_QUERY_STAT(ZombieFixture),false,Person);Q.AddIgnoredActor(Z);Q.AddIgnoredActor(Bike);
  VERIFY_ZOMBIE(GetWorld()->LineTraceSingleByChannel(Ground,Desired+FVector(0,0,300),Desired-FVector(0,0,600),ECC_Visibility,Q),"No firing fixture ground");
  Z->SetActorLocation(Ground.ImpactPoint+FVector(0,0,88),false,nullptr,ETeleportType::TeleportPhysics);Z->GetCharacterMovement()->StopMovementImmediately();
  ZombiePhase=5;ZombieAuditClock=0;ZombieShots=0;
 }
 else if(ZombiePhase==5){
  FVector Eye;FRotator View;GetPlayerViewPoint(Eye,View);SetControlRotation((Z->GetActorLocation()+FVector(0,0,10)-Eye).Rotation());
  if(ZombieAuditClock>.35f){VERIFY_ZOMBIE(Person&&Person->Fire(),"Pistol did not fire");ZombieShots++;ZombieAuditClock=0;
   if(ZombieShots==3){VERIFY_ZOMBIE(Z->bDead&&Bike->EnemyKills==1&&Bike->Nitro==25,"Three body hits did not kill/reward");Z->SetActorTickEnabled(true);ZombieAuditCorpse=Z;
    const FVector Spot=Z->GetActorLocation()+Bike->GetActorRightVector()*350;auto* Head=GetWorld()->SpawnActor<ABattleZombie>(Spot,FRotator::ZeroRotator);VERIFY_ZOMBIE(Head,"Headshot fixture spawn failed");Head->Emergence=0;Head->SetActorTickEnabled(false);ZombieAuditTarget=Head;ZombiePhase=6;ZombieAuditClock=0;
   }
  }
 }
 else if(ZombiePhase==6){
  FVector Eye;FRotator View;GetPlayerViewPoint(Eye,View);SetControlRotation((Z->GetActorLocation()+FVector(0,0,65)-Eye).Rotation());
  if(ZombieAuditClock>.4f){VERIFY_ZOMBIE(Person&&Person->Fire(),"Headshot did not fire");VERIFY_ZOMBIE(Z->bDead&&Z->Headshots==1&&Bike->EnemyKills==2&&Bike->Nitro==50,"Headshot did not kill/reward");Z->SetActorTickEnabled(true);ZombiePhase=7;ZombieAuditClock=0;}
 }
 else if(ZombiePhase==7&&ZombieAuditClock>5.2f){
  VERIFY_ZOMBIE(!ZombieAuditTarget.IsValid()&&!ZombieAuditCorpse.IsValid(),"Corpses did not clean up");Finish(true,TEXT("Spawn, navigation, telegraph, health damage, pistol body/head hits and cleanup pass"));return;
 }
 if(ZombieAuditClock>25)Finish(false,TEXT("Phase timeout"));
#undef VERIFY_ZOMBIE
#endif
}

#include "PiedmontPathSpline.h"
#include "Components/SplineComponent.h"
#include "PiedmontDarkZone.h"
void ABattleMacController::TickZombiePopulationAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5)return;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* Bike=Cast<ABattleBike>(GetPawn());if(!Mode||!Mode->Enemies||!Bike)return;
 auto* Director=Mode->Enemies.Get();ZombieAuditClock+=Dt;int32 Count=0,Sprinters=0;bool Tuned=true,WaveInside=true;
 for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
 for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
 for(TActorIterator<ABattleZombie> It(GetWorld());It;++It){
  It->SetActorTickEnabled(false);if(auto* AI=Cast<AAIController>(It->GetController()))AI->StopMovement();
  if(It->bDead)continue;Count++;if(It->bSprinter)Sprinters++;
  Tuned=Tuned&&FMath::IsNearlyEqual(It->GetCharacterMovement()->MaxWalkSpeed,It->bSprinter?Mode->Difficulty.SprinterSpeed:Mode->Difficulty.ZombieSpeed)&&FMath::IsNearlyEqual(It->AttackDamage,Mode->Difficulty.ZombieDamage)&&FMath::IsNearlyEqual(It->WarningSeconds,Mode->Difficulty.ZombieWarningSeconds);
  if(ZombiePhase==1&&FVector::Dist2D(It->GetActorLocation(),Bike->GetActorLocation())<3000){bool Covered=false;for(TActorIterator<APiedmontDarkZone> Zone(GetWorld());Zone;++Zone)if(Zone->Contains(It->GetActorLocation())){Covered=true;break;}WaveInside=WaveInside&&Covered;}
 }
 auto Finish=[&](bool Passed,const TCHAR* Reason){
  UE_LOG(LogTemp,Display,TEXT("BattleZombiePopulationAudit: {\"passed\":%s,\"difficulty\":\"%s\",\"reason\":\"%s\",\"desired\":%d,\"live\":%d,\"sprinters\":%d,\"waves\":%d,\"waveSpawned\":%d,\"tuned\":%s}"),Passed?TEXT("true"):TEXT("false"),*Mode->DifficultyName.ToString(),Reason,Director->DesiredZombies,Count,Sprinters,Director->Waves,Director->WaveSpawned,Tuned?TEXT("true"):TEXT("false"));
  UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);
 };
 if(!Tuned){Finish(false,TEXT("Difficulty parameters not applied to spawned actors"));return;}
 if(ZombiePhase==0&&Count==Mode->Difficulty.Zombies){
  if(Mode->Difficulty.TunnelWaveSize==0){Finish(true,TEXT("Configured population spawned"));return;}
  bool Found=false;for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("BattleKrog_4"))){
   const FVector P=It->Centerline->GetLocationAtSplinePoint(It->Centerline->GetNumberOfSplinePoints()/2,ESplineCoordinateSpace::World);Bike->SetActorLocation(P+FVector(0,0,98),false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->bForceNextFloorCheck=true;Found=true;break;
  }
  if(!Found){Finish(false,TEXT("Missing tunnel fixture"));return;}ZombiePhase=1;ZombieAuditClock=0;
 }
 else if(ZombiePhase==1&&Director->WaveSpawned>=Mode->Difficulty.TunnelWaveSize){
  if(Director->Waves!=1||Director->RemainingWave!=0||!WaveInside){Finish(false,TEXT("Incomplete or uncovered first wave"));return;}
  ZombiePhase=2;ZombieShots=Director->WaveSpawned;ZombieAuditClock=0;
 }
 else if(ZombiePhase==2&&Director->WaveSpawned>=ZombieShots+4){
  Bike->SetActorLocation(Bike->CheckpointTransform.GetLocation(),false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->bForceNextFloorCheck=true;ZombieShots=Director->Spawned;ZombiePhase=3;ZombieAuditClock=0;
 }
 else if(ZombiePhase==3&&ZombieAuditClock>2){Finish(Director->Waves==2&&Director->RemainingWave==0&&Director->Spawned>ZombieShots,TEXT("Full covered wave, repeat wave and ordinary spawns after leaving pass"));return;}
 if(ZombieAuditClock>60)Finish(false,TEXT("Population phase timed out"));
#endif
}
