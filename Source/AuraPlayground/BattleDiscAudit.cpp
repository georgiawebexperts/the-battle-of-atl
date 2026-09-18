#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattlePickup.h"
#include "BattleDisc.h"
#include "BattleZombie.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "PiedmontPathSpline.h"
#include "PiedmontBike.h"
#include "Components/SplineComponent.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
void ABattleMacController::TickDiscAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5)return;
 auto* Person=Cast<ABattleRider>(GetPawn());auto* Bike=Person?Person->ParkedBike.Get():Cast<ABattleBike>(GetPawn());if(!Bike)return;
 DiscClock+=Dt;auto* First=Cast<ABattleZombie>(DiscFirst.Get());auto* Second=Cast<ABattleZombie>(DiscSecond.Get());auto* Projectile=Cast<ABattleDisc>(DiscProjectile.Get());
 auto Finish=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BattleDiscAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\",\"kills\":%d,\"ammo\":%d}"),Pass?TEXT("true"):TEXT("false"),DiscPhase,Reason,Bike->EnemyKills,Person?Person->Ammo:0);UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);};
#define CHECK_DISC(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Key=[&](FKey K){for(bool Down:{true,false})InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Target=[&](FVector P){auto* Z=GetWorld()->SpawnActor<ABattleZombie>(P,FRotator::ZeroRotator);if(Z){Z->SetActorTickEnabled(false);Z->GetCharacterMovement()->DisableMovement();Z->WeaponDropChance=0;}return Z;};
 auto Wall=[&](FVector P){auto* A=GetWorld()->SpawnActor<AStaticMeshActor>(P,FRotator::ZeroRotator);if(A){auto* M=A->GetStaticMeshComponent();M->SetMobility(EComponentMobility::Movable);M->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));A->SetActorScale3D(FVector(.1,20,20));M->SetCollisionProfileName(TEXT("BlockAll"));}return A;};
 if(DiscPhase==0){
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  ABattleWeaponCrate* Crate=nullptr;for(TActorIterator<ABattleWeaponCrate> It(GetWorld());It;++It)if(It->WeaponSlot==3){Crate=*It;break;}
  CHECK_DISC(Crate,"No real launcher crate");Bike->SetActorLocation(Crate->GetActorLocation()+FVector(0,0,33),false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->bForceNextFloorCheck=true;DiscPhase=1;DiscClock=0;
 }
 else if(DiscPhase==1&&DiscClock>.3f){
  CHECK_DISC(Bike->Inventory[3].Owned&&Bike->Inventory[3].Magazine==8&&Bike->Inventory[3].Reserve==8,"Launcher crate supply failed");
  // The arena needs dry, level, open ground. The bike's checkpoint is over a
  // water polygon, and the old floating arena does not survive that: the swim
  // code pulls the rider straight back down to the water surface, so the arena
  // ended up 40 m above the muzzle and the disc flew through empty air. Pick a
  // dry stretch of route instead and let the disc fly at live muzzle height.
  FVector Spot;bool bFound=false;
  for(TActorIterator<APiedmontPathSpline> Path(GetWorld());Path&&!bFound;++Path){
   auto* Route=Path->Centerline.Get();if(!Route)continue;
   const float Length=Route->GetSplineLength();
   for(float D=800.f;D+1600.f<Length;D+=1200.f){
    const FVector P=Route->GetLocationAtDistanceAlongSpline(D,ESplineCoordinateSpace::World);
    const FVector Far=Route->GetLocationAtDistanceAlongSpline(D+1600.f,ESplineCoordinateSpace::World);
    if(FVector::Dist2D(P,Far)<1400.f)continue;
    FHitResult G;FCollisionQueryParams Q;Q.bIgnoreTouches=true;
    if(!GetWorld()->LineTraceSingleByChannel(G,P+FVector(0,0,2000),P-FVector(0,0,2500),ECC_Visibility,Q))continue;
    if(G.ImpactNormal.Z<.98f)continue;                                  // level pad
    const FVector At=G.ImpactPoint+FVector(0,0,98);
    bool bWet=false;for(TActorIterator<APiedmontWaterHazard> Water(GetWorld());Water;++Water)if(Water->ContainsBike(At)){bWet=true;break;}
    if(bWet)continue;
    // Clear lane for the disc: no wall or trunk in the first 12 m.
    FHitResult Lane;FCollisionQueryParams LQ(SCENE_QUERY_STAT(DiscLane),false,Bike);
    if(GetWorld()->SweepSingleByChannel(Lane,At,At+FVector(1200,0,0),FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(14),LQ))continue;
    Spot=At;bFound=true;break;
   }
  }
  if(!bFound){Finish(false,TEXT("No dry open arena for the launcher"));return;}
  Bike->SetActorLocationAndRotation(Spot,FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->Ride->SetMovementMode(MOVE_Walking);Bike->Ride->bForceNextFloorCheck=true;
  CHECK_DISC(Bike->Dismount(),"Dismount failed");Person=Cast<ABattleRider>(GetPawn());CHECK_DISC(Person,"Missing FPS rider");
  Person->GetCharacterMovement()->DisableMovement();SetControlRotation(FRotator::ZeroRotator);Key(EKeys::Four);
  DiscPhase=2;DiscClock=0;
 }
 else if(DiscPhase==2&&DiscClock>1.2f){
  // Build the arena on the muzzle line, not on the rider's feet. The disc is
  // fired down the camera's view ray, and the chase camera sits 320 cm back and
  // 55 cm to the side, so an arena built from the actor location put the targets
  // about 70 cm off the flight line and the disc sailed past both of them.
  // It is built after the rider settles for the same reason.
  FVector MuzzleEye;FRotator MuzzleView;GetPlayerViewPoint(MuzzleEye,MuzzleView);
  DiscFirst=Target(MuzzleEye+FVector(400,0,0));DiscSecond=Target(MuzzleEye+FVector(700,0,0));DiscWall=Wall(MuzzleEye+FVector(1000,0,0));CHECK_DISC(DiscFirst.IsValid()&&DiscSecond.IsValid()&&DiscWall.IsValid(),"Arena setup failed");
  UE_LOG(LogTemp,Display,TEXT("DiscAudit: arena muzzle=%s first=%s second=%s wall=%s"),*MuzzleEye.ToString(),*DiscFirst->GetActorLocation().ToString(),*DiscSecond->GetActorLocation().ToString(),*DiscWall->GetActorLocation().ToString());
  if(Person&&!(Person->CurrentWeapon==3&&Person->bWeaponDrawn&&Person->DrawRemaining<=0))
   UE_LOG(LogTemp,Display,TEXT("DiscAudit: weapon=%d drawn=%d draw=%.2f ammo=%d owned=%d"),Person->CurrentWeapon,Person->bWeaponDrawn?1:0,Person->DrawRemaining,Person->Ammo,Bike->Inventory[3].Owned?1:0);
  CHECK_DISC(Person->CurrentWeapon==3&&Person->Fire()&&Person->Ammo==7,"4 key/launcher fire failed");TActorIterator<ABattleDisc> It(GetWorld());CHECK_DISC(It,"No moving disc");DiscProjectile=*It;DiscPhase=3;DiscClock=0;
 }
 else if(DiscPhase==3&&DiscClock>.65f){
  UE_LOG(LogTemp,Display,TEXT("DiscAudit: flight valid=%d hits=%d bounces=%d vx=%.0f at=%s"),Projectile?1:0,Projectile?Projectile->Hits:-1,Projectile?Projectile->Bounces:-1,Projectile?Projectile->Velocity.X:0.f,Projectile?*Projectile->GetActorLocation().ToString():TEXT("gone"));
  CHECK_DISC(Projectile&&Projectile->Hits==2&&Projectile->Bounces==1&&Projectile->Velocity.X<0,"Multi-target flight or wall reflection failed");CHECK_DISC(First->Health==35&&Second->Health==35&&Bike->EnemyKills==0,"Disc damage or duplicate-hit guard failed");SetControlRotation(FRotator::ZeroRotator);CHECK_DISC(Person->Fire()&&Person->Ammo==6,"Second disc failed");DiscPhase=4;DiscClock=0;
 }
 else if(DiscPhase==4&&DiscClock>.65f){
  CHECK_DISC(First->bDead&&Second->bDead&&Bike->EnemyKills==2&&Bike->Nitro==50,"Second disc kills/reward failed");CHECK_DISC(Projectile&&Projectile->Hits==2,"Returning disc hit a target twice");Key(EKeys::R);DiscPhase=5;DiscClock=0;
 }
 else if(DiscPhase==5&&DiscClock>1.7f){CHECK_DISC(Person->Ammo==8&&Bike->Inventory[3].Reserve==6,"Disc reload did not conserve ammo");DiscPhase=6;}
 else if(DiscPhase==6&&DiscClock>8.2f){
  TActorIterator<ABattleDisc> Remaining(GetWorld());CHECK_DISC(!Remaining,"Disc lifetime cleanup failed");
  if(First)First->Destroy();if(Second)Second->Destroy();if(DiscWall.IsValid())DiscWall->Destroy();
  Person->SetActorLocation(Bike->GetActorLocation()+FVector(0,145,0),false,nullptr,ETeleportType::TeleportPhysics);
  // Grounded zombie makes the normal drop placement code inspect real ground.
  FVector Spot=Bike->GetActorLocation()+FVector(400,0,0);FHitResult Floor;FCollisionQueryParams Q(SCENE_QUERY_STAT(DropFixture),false,Person);Q.AddIgnoredActor(Bike);
  CHECK_DISC(GetWorld()->LineTraceSingleByChannel(Floor,Spot+FVector(0,0,200),Spot-FVector(0,0,500),ECC_Visibility,Q),"No drop fixture floor");
  First=Target(Floor.ImpactPoint+FVector(0,0,88));CHECK_DISC(First,"Drop target failed");First->WeaponDropChance=1;First->TakeDamage(100,FDamageEvent(),this,Person);
  CHECK_DISC(First->bDead&&IsValid(First->DroppedWeapon)&&First->DroppedWeapon->GetLifeSpan()>59,"Enemy drop placement/expiry failed");auto* Drop=First->DroppedWeapon.Get();const int32 Slot=Drop->WeaponSlot;const int32 Before=Bike->Inventory[Slot].Reserve;
  const float DropDistance=FVector::Dist(Drop->GetActorLocation(),Person->GetActorLocation());
  // Clear any body out of the pickup line. The collect guard is a line trace
  // from the rider to the crate, and a neighbouring corpse standing over the
  // drop blocked it - the crate is then unreachable through no fault of its own.
  int32 Cleared=0;
  for(TActorIterator<ABattleZombie> Body(GetWorld());Body;++Body)
   if(FVector::Dist2D(Body->GetActorLocation(),Drop->GetActorLocation())<300.f){
    TInlineComponentArray<UPrimitiveComponent*> Parts(*Body);
    for(auto* Part:Parts)Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Cleared++;
   }
  Person->SetActorLocation(Drop->GetActorLocation()+FVector(0,0,30),false,nullptr,ETeleportType::TeleportPhysics);
  const bool Collected=Drop->TryCollect(Person);const bool Owned=Bike->Inventory[Slot].Owned;const int32 After=Bike->Inventory[Slot].Reserve;const bool Duplicate=Drop->TryCollect(Person);const bool Consumed=Drop->bConsumed;
  if(!Collected||!Owned||After<=Before||Duplicate){
   FCollisionQueryParams DQ(SCENE_QUERY_STAT(DropProbe),false,Person);DQ.AddIgnoredActor(Drop);FHitResult DHit;
   const bool Blocked=GetWorld()->LineTraceSingleByChannel(DHit,Person->GetActorLocation(),Drop->GetActorLocation(),ECC_Visibility,DQ);
   const auto* DM=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));
   const int32 Controlled=Person->IsPlayerControlled()?1:0;const int32 Paused=UGameplayStatics::IsGamePaused(this)?1:0;
   const int32 RunEnded=DM?DM->bRunEnded:0;const float Countdown=DM?DM->StartCountdown:-1.f;
   UE_LOG(LogTemp,Display,TEXT("DiscAudit: drop collected=%d owned=%d slot=%d before=%d after=%d duplicate=%d consumed=%d controlled=%d blocked=%d health=%.0f countdown=%.2f"),
    Collected?1:0,Owned?1:0,Slot,Before,After,Duplicate?1:0,Consumed?1:0,Controlled,Blocked?1:0,Bike->RiderHealth,Countdown);
   UE_LOG(LogTemp,Display,TEXT("DiscAudit: drop2 dist_before=%.0f respawn=%.2f paused=%d run_ended=%d cleared=%d parked=%s blocked_by=%s component=%s"),DropDistance,Bike->RespawnRemaining,Paused,RunEnded,Cleared,*GetNameSafe(Person->ParkedBike.Get()),*GetNameSafe(DHit.GetActor()),*GetNameSafe(DHit.GetComponent()));
   Finish(false,TEXT("Enemy drop pickup/duplicate guard failed"));return;
  }
  Finish(true,TEXT("Launcher crate/key, two-target flight, ricochet, one hit per target, reload, cleanup and enemy drop pass"));return;
 }
 if(DiscClock>15)Finish(false,TEXT("Disc phase timed out"));
#undef CHECK_DISC
#endif
}
