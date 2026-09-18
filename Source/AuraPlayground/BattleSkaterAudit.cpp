#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleSkater.h"
#include "BattleZombie.h"
#include "PiedmontTrafficDirector.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"

/**
 * The skatepark fixture, in two halves, because the promise has two halves.
 *
 * Elliott: "arcade you dont get knocked off by people and trees realistic you
 * do." So the bike is driven into a skater twice: once in arcade, where the hit
 * has to count as an assault and leave the rider upright, and once in realistic,
 * where the same hit also has to throw him off. The second hit uses the other
 * skater on purpose - RecordAssault remembers its victims, so hitting the same
 * one twice only ever counts once, and the audit would pass or fail for the
 * wrong reason.
 */
void ABattleMacController::TickSkaterAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5||SkaterStage==99)return;SkaterClock+=Dt;
 auto* Rider=Cast<ABattleRider>(GetPawn());auto* Bike=Rider?Rider->ParkedBike.Get():Cast<ABattleBike>(GetPawn());auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));if(!Bike||!Mode)return;
 auto* Skater=Cast<ABattleSkater>(SkaterTarget.Get());
 auto* Second=Cast<ABattleSkater>(SkaterSecond.Get());
 auto Finish=[&](bool Pass,const TCHAR* Why){UE_LOG(LogTemp,Display,TEXT("BattleSkaterAudit: {\"passed\":%s,\"stage\":%d,\"reason\":\"%s\",\"max_stance_error_cm\":%.3f,\"travel_cm\":%.1f,\"corners\":%d,\"people_hit\":%d}"),Pass?TEXT("true"):TEXT("false"),SkaterStage,Why,SkaterMaxError,Skater?Skater->Travelled:0,Skater?Skater->Corners:0,Mode->PeopleHit);SkaterStage=99;ConsoleCommand(TEXT("quit"));};
#define KCHECK(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Next=[&](){SkaterStage++;SkaterClock=0;};auto Key=[&](bool Down){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::W,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 // Drop a pawn onto the deck. The centre is known good, but tracing keeps a
 // fixture placement honest if the deck is ever rebuilt underneath it.
 auto PlaceOnDeck=[&](AActor* Who,const FVector& Where,const FRotator& Facing){
  FHitResult Ground;FCollisionQueryParams Q(SCENE_QUERY_STAT(SkaterDeck),false,Who);
  const FVector Spot=GetWorld()->LineTraceSingleByChannel(Ground,Where+FVector(0,0,400),Where-FVector(0,0,700),ECC_Visibility,Q)?Ground.ImpactPoint+FVector(0,0,98):Where;
  Who->SetActorLocationAndRotation(Spot,Facing,false,nullptr,ETeleportType::TeleportPhysics);return Spot;
 };
 if(SkaterStage==0){
  int Count=0,Spectators=0;ABattleSkater* First=nullptr;
  for(TActorIterator<ABattleSkater> It(GetWorld());It;++It){Count++;if(!First)First=*It;SkaterTarget=*It;}
  SkaterSecond=First;
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("SkateparkSpectator")))Spectators++;
  KCHECK(Count==2&&Spectators==3,"Missing skaters or spectators");for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)if(!It->ActorHasTag(TEXT("SkateparkSkater"))&&!It->ActorHasTag(TEXT("SkateparkSpectator")))It->Destroy();
  // Keep the observation phase quiet. A zombie that grabs the wheel during it
  // kills the rider, and the pending checkpoint recovery then fires in the
  // middle of the contact fixture, teleporting the bike 970 m to the gate.
  Mode->Trouble=0;
  for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)It->Destroy();
  Bike->DamageGrace=100;
  Bike->SetActorLocation(FVector(41800,74168,580),false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->SetActorTickEnabled(false);Bike->Ride->SetComponentTickEnabled(false);Next();
 }else if(SkaterStage==1){
  KCHECK(Skater&&!Skater->bDead,"Skater disappeared");SkaterMaxError=FMath::Max(SkaterMaxError,Skater->StanceError);SkaterPushSeen|=Skater->PushAmount>.8f;SkaterCoastSeen|=Skater->PushAmount==0;
  KCHECK(SkaterMaxError<3,"Skating foot target unreachable");KCHECK(FMath::Abs(Skater->GetActorLocation().X-39000)<1850&&FMath::Abs(Skater->GetActorLocation().Y-74000)<1300&&Skater->GetActorLocation().Z>700,"Skater left rideable circuit");
  if(SkaterClock>34){KCHECK(Skater->Travelled>6500&&Skater->Corners>=5&&SkaterPushSeen&&SkaterCoastSeen,"Skating circuit or push/coast cycle failed");
   for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)if(*It!=Skater&&*It!=Second)It->Destroy();
   for(auto* Frozen:{Skater,Second})if(Frozen){Frozen->SetActorTickEnabled(false);Frozen->GetCharacterMovement()->DisableMovement();}
   // Use the unobstructed center deck so a rail or bowl lip cannot turn this into a glancing contact.
   Skater->SetActorLocationAndRotation(FVector(39300,74000,738),FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);
   Bike->SetActorTickEnabled(true);Bike->Ride->SetComponentTickEnabled(true);
   // A zombie can grab the wheel while the skaters are being watched, which
   // throws the rider off - and then the contact fixture below would be holding
   // W on a parked bike nobody is riding, which is exactly how this audit went
   // red on 2026-09-18 (health 20, parked, reason "Zombie grabbed the wheel",
   // bike 25 m from the skater and never moving). Clear the troublemakers, put
   // the rider back on the bike the way the game does, and make the bike immune
   // to another grab for the length of the contact test.
   Mode->Trouble=0;
   for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)It->Destroy();
   // Put the bike on the deck first and remount there. Remounting at the spot the
   // rider was thrown off refuses, and this is the state the fixture wants anyway.
   const FVector Facing=Skater->GetActorForwardVector();
   FVector Deck=Skater->GetActorLocation()-Facing*270+FVector(0,0,10);
   {FHitResult Ground;FCollisionQueryParams GQ(SCENE_QUERY_STAT(SkaterDeck),false,Bike);GQ.AddIgnoredActor(Skater);
    if(GetWorld()->LineTraceSingleByChannel(Ground,Deck+FVector(0,0,400),Deck-FVector(0,0,700),ECC_Visibility,GQ))Deck=Ground.ImpactPoint+FVector(0,0,98);}
   Bike->SetActorLocationAndRotation(Deck,Facing.Rotation(),false,nullptr,ETeleportType::TeleportPhysics);
   Bike->Ride->StopMovementImmediately();Bike->Ride->SetMovementMode(MOVE_Walking);Bike->Ride->bForceNextFloorCheck=true;
   Bike->ClearPhysicalCrash();Bike->StunRemaining=0;Bike->RiderHealth=100;
   // An earlier knock-off can leave a checkpoint recovery already counting down.
   // Clearing the health without clearing that just means it fires a second
   // later, wherever the fixture happens to have moved the bike.
   Bike->RespawnRemaining=0;
   if(auto* Foot=Cast<ABattleRider>(GetPawn())){
    Foot->SetActorLocation(Deck+FVector(0,0,40),false,nullptr,ETeleportType::TeleportPhysics);
    UE_LOG(LogTemp,Display,TEXT("SkaterFixture: skater=%s bike=%s gap=%.0f rider=%s"),*Skater->GetActorLocation().ToCompactString(),*Deck.ToCompactString(),
     FVector::Dist(Deck,Skater->GetActorLocation()),*Foot->GetActorLocation().ToCompactString());
    KCHECK(Bike->Remount(Foot),"Could not put the rider back on the bike for the contact fixture");
   }
   Bike->RiderHealth=100;Bike->DamageGrace=100;
   // The idle observation phase freezes movement, including any startup recovery.
   // Start the contact fixture ready to ride; retain counters to check the new impact.
   Bike->Ride->Recovery=0;
   SkaterInitialWipeouts=Bike->Ride->Wipeouts;
   SkaterContactStart=Bike->GetActorLocation();
   SetControlRotation(Facing.Rotation());Bike->Ride->Gear=4;Bike->Ride->Speed=1100;Key(true);Next();
  }
 }else if(SkaterStage==2&&Skater&&Skater->BikeContacts>0){
  UE_LOG(LogTemp,Display,TEXT("SkaterContact: contacts=%d wipeouts=%d->%d people=%d recovery=%.1f crash=%d"),
   Skater->BikeContacts,SkaterInitialWipeouts,Bike->Ride->Wipeouts,Mode->PeopleHit,Bike->Ride->Recovery,Bike->bCrashActive?1:0);
  KCHECK(Mode->PeopleHit==1,"Skater bike collision did not count as pedestrian assault");
  KCHECK(Bike->Ride->Wipeouts==SkaterInitialWipeouts&&Bike->Ride->Recovery<=0&&!Bike->bCrashActive,"Arcade threw the rider off for hitting a person");
  Key(false);Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;
  // Second half: the same hit in realistic handling, on the skater who has not
  // been hit yet, with the first one parked out of the contact lane.
  KCHECK(Second&&Second!=Skater,"Second skater missing for the realistic half");
  PlaceOnDeck(Skater,FVector(38600,74000,738),FRotator::ZeroRotator);
  const FVector Target=PlaceOnDeck(Second,FVector(39300,74000,738),FRotator::ZeroRotator);
  PlaceOnDeck(Bike,Target-FVector(270,0,10),FRotator::ZeroRotator);
  Bike->Ride->Recovery=0;Bike->Ride->bRealHandling=true;Bike->Ride->SetMovementMode(MOVE_Walking);Bike->Ride->bForceNextFloorCheck=true;Bike->Ride->Gear=4;Bike->Ride->Speed=1100;
  SetControlRotation(FRotator::ZeroRotator);Key(true);Next();
 }
 else if(SkaterStage==3&&Second&&Second->BikeContacts>0){
  UE_LOG(LogTemp,Display,TEXT("SkaterRealisticContact: contacts=%d wipeouts=%d->%d people=%d recovery=%.1f crash=%d"),
   Second->BikeContacts,SkaterInitialWipeouts,Bike->Ride->Wipeouts,Mode->PeopleHit,Bike->Ride->Recovery,Bike->bCrashActive?1:0);
  KCHECK(Mode->PeopleHit==2,"Second pedestrian hit did not raise the assault count");
  KCHECK(Bike->Ride->Wipeouts==SkaterInitialWipeouts+1||Bike->bCrashActive,"Realistic handling did not throw the rider off for hitting a person");
  Key(false);Bike->Ride->bRealHandling=false;Next();
 }
 else if(SkaterStage==2){
  // The fixture puts the bike 270 cm behind a frozen skater and points it at
  // him, so a timeout here means the bike never closed that gap. Say which half
  // of it failed: did it move, and did it stay on the floor while it did.
  if(Skater&&FMath::Fmod(SkaterClock,.5f)<Dt)UE_LOG(LogTemp,Display,TEXT("SkaterProbe: clock=%.1f bike=%s skater=%s travelled=%.0f gap=%.0f speed=%.1f pedal=%.2f recovery=%.2f reason=%s contacts=%d parked=%d crash=%d grounded=%d health=%.0f deaths=%d"),
   SkaterClock,*Bike->GetActorLocation().ToCompactString(),*Skater->GetActorLocation().ToCompactString(),
   FVector::Dist2D(SkaterContactStart,Bike->GetActorLocation()),FVector::Dist(Bike->GetActorLocation(),Skater->GetActorLocation()),
   Bike->Ride->Speed,Bike->Ride->Pedal,Bike->Ride->Recovery,*Bike->Ride->RecoveryReason,Skater->BikeContacts,
   Bike->bParked?1:0,Bike->bCrashActive?1:0,Bike->Ride->IsMovingOnGround()?1:0,Bike->RiderHealth,Bike->Deaths);
 }
 else if(SkaterStage==3){
  if(Second&&FMath::Fmod(SkaterClock,.5f)<Dt)UE_LOG(LogTemp,Display,TEXT("SkaterRealisticProbe: clock=%.1f bike=%s skater=%s gap=%.0f speed=%.1f recovery=%.2f contacts=%d crash=%d parked=%d"),
   SkaterClock,*Bike->GetActorLocation().ToCompactString(),*Second->GetActorLocation().ToCompactString(),
   FVector::Dist(Bike->GetActorLocation(),Second->GetActorLocation()),Bike->Ride->Speed,Bike->Ride->Recovery,Second->BikeContacts,
   Bike->bCrashActive?1:0,Bike->bParked?1:0);
 }
 else if(SkaterStage==4){
  // A realistic knock-off throws the rider off and takes a few seconds to settle;
  // wait for that instead of assuming the bike is still under him.
  if(Bike->bCrashActive||Bike->Ride->Recovery>0)return;
  if(Cast<ABattleBike>(GetPawn()))KCHECK(Bike->Dismount(),"Cannot dismount after skater collision");
  // Move to a clear shooting lane after recovery, away from the parked bike.
  if(auto* Foot=Cast<ABattleRider>(GetPawn()))Foot->SetActorLocation(Skater->GetActorLocation()+FVector(0,-350,110),false,nullptr,ETeleportType::TeleportPhysics);Next();}
 else if(SkaterStage==5){KCHECK(Rider,"Missing on-foot rider");
  // Fire() refuses with the weapon holstered, and a rider who has just been
  // thrown off the bike is holstered - which the old version of this stage never
  // accounted for because the fixture used to leave him in the saddle.
  if(!Rider->bWeaponDrawn){KCHECK(Rider->ToggleDrawWeapon(),"Could not draw a weapon to shoot the skater");return;}
  if(Rider->DrawRemaining>0)return;
  FVector Eye;FRotator View;GetPlayerViewPoint(Eye,View);SetControlRotation((Skater->GetActorLocation()-Eye).Rotation());if(SkaterClock<=.5f)return;FCollisionQueryParams Q(SCENE_QUERY_STAT(SkaterShotAudit),true,Rider);FHitResult Sight;GetWorld()->LineTraceSingleByChannel(Sight,Eye,Eye+View.Vector()*1500,ECC_Visibility,Q);UE_LOG(LogTemp,Display,TEXT("SkaterShotSight: actor=%s eye=%s target=%s"),*GetNameSafe(Sight.GetActor()),*Eye.ToString(),*Skater->GetActorLocation().ToString());KCHECK(Sight.GetActor()==Skater,"Skater camera aim obstructed");const float Before=Mode->TimeRemaining;const bool Fired=Rider->Fire();UE_LOG(LogTemp,Display,TEXT("SkaterShot: fired=%d dead=%d delta=%.2f"),Fired,Skater->bDead,Mode->TimeRemaining-Before);KCHECK(Fired&&Skater->bDead&&FMath::IsNearlyEqual(Mode->TimeRemaining-Before,-10.f,.001f),"Shooting skater did not apply civilian time penalty");KCHECK(Mode->PeopleHit==2,"Same victim counted twice");Finish(true,TEXT("Two skaters/three spectators, circuit, foot reach, push/coast, arcade body hit leaving the rider up, realistic body hit throwing him off, and the civilian shooting penalty pass"));}
 if(SkaterClock>(SkaterStage==1?40:(SkaterStage==4?16:8))&&SkaterStage!=99)Finish(false,TEXT("Skater phase timed out"));
#undef KCHECK
#endif
}
