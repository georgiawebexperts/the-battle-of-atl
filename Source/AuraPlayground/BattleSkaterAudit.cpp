#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleSkater.h"
#include "PiedmontTrafficDirector.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
void ABattleMacController::TickSkaterAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5||SkaterStage==99)return;SkaterClock+=Dt;
 auto* Rider=Cast<ABattleRider>(GetPawn());auto* Bike=Rider?Rider->ParkedBike.Get():Cast<ABattleBike>(GetPawn());auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));if(!Bike||!Mode)return;
 auto* Skater=Cast<ABattleSkater>(SkaterTarget.Get());
 auto Finish=[&](bool Pass,const TCHAR* Why){UE_LOG(LogTemp,Display,TEXT("BattleSkaterAudit: {\"passed\":%s,\"stage\":%d,\"reason\":\"%s\",\"max_stance_error_cm\":%.3f,\"travel_cm\":%.1f,\"corners\":%d,\"people_hit\":%d}"),Pass?TEXT("true"):TEXT("false"),SkaterStage,Why,SkaterMaxError,Skater?Skater->Travelled:0,Skater?Skater->Corners:0,Mode->PeopleHit);SkaterStage=99;ConsoleCommand(TEXT("quit"));};
#define KCHECK(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Next=[&](){SkaterStage++;SkaterClock=0;};auto Key=[&](bool Down){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::W,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 if(SkaterStage==0){
  int Count=0,Spectators=0;for(TActorIterator<ABattleSkater> It(GetWorld());It;++It){Count++;SkaterTarget=*It;}for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("SkateparkSpectator")))Spectators++;
  KCHECK(Count==2&&Spectators==3,"Missing skaters or spectators");for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)if(!It->ActorHasTag(TEXT("SkateparkSkater"))&&!It->ActorHasTag(TEXT("SkateparkSpectator")))It->Destroy();
  Bike->SetActorLocation(FVector(41800,74168,580),false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->SetActorTickEnabled(false);Bike->Ride->SetComponentTickEnabled(false);Next();
 }else if(SkaterStage==1){
  KCHECK(Skater&&!Skater->bDead,"Skater disappeared");SkaterMaxError=FMath::Max(SkaterMaxError,Skater->StanceError);SkaterPushSeen|=Skater->PushAmount>.8f;SkaterCoastSeen|=Skater->PushAmount==0;
  KCHECK(SkaterMaxError<3,"Skating foot target unreachable");KCHECK(FMath::Abs(Skater->GetActorLocation().X-39000)<1850&&FMath::Abs(Skater->GetActorLocation().Y-74000)<1300&&Skater->GetActorLocation().Z>700,"Skater left rideable circuit");
  if(SkaterClock>34){KCHECK(Skater->Travelled>6500&&Skater->Corners>=5&&SkaterPushSeen&&SkaterCoastSeen,"Skating circuit or push/coast cycle failed");
   for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)if(*It!=Skater)It->Destroy();Skater->SetActorTickEnabled(false);Skater->GetCharacterMovement()->DisableMovement();
   // Use the unobstructed center deck so a rail or bowl lip cannot turn this into a glancing contact.
   Skater->SetActorLocationAndRotation(FVector(39300,74000,738),FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);
   Bike->SetActorTickEnabled(true);Bike->Ride->SetComponentTickEnabled(true);
   // The idle observation phase freezes movement, including any startup recovery.
   // Start the contact fixture ready to ride; retain counters to check the new impact.
   Bike->Ride->Recovery=0;
   SkaterInitialWipeouts=Bike->Ride->Wipeouts;const FVector Facing=Skater->GetActorForwardVector();Bike->SetActorLocationAndRotation(Skater->GetActorLocation()-Facing*270+FVector(0,0,10),Facing.Rotation(),false,nullptr,ETeleportType::TeleportPhysics);SetControlRotation(Facing.Rotation());Bike->Ride->StopMovementImmediately();Bike->Ride->SetMovementMode(MOVE_Walking);Bike->Ride->bForceNextFloorCheck=true;Bike->Ride->Gear=4;Bike->Ride->Speed=1100;Key(true);Next();
  }
 }else if(SkaterStage==2&&Skater->BikeContacts>0){UE_LOG(LogTemp,Display,TEXT("SkaterContact: wipeouts_before=%d after=%d contacts=%d speed=%.1f recovery=%.1f"),SkaterInitialWipeouts,Bike->Ride->Wipeouts,Skater->BikeContacts,Bike->Ride->Speed,Bike->Ride->Recovery);KCHECK(Bike->Ride->Wipeouts==SkaterInitialWipeouts+1&&Mode->PeopleHit==1,"Skater bike collision did not count as pedestrian assault");Key(false);Next();}
 else if(SkaterStage==3&&SkaterClock>2.5f){KCHECK(Bike->Dismount(),"Cannot dismount after skater collision");
  // Move to a clear shooting lane after recovery, away from the parked bike.
  if(auto* Foot=Cast<ABattleRider>(GetPawn()))Foot->SetActorLocation(Skater->GetActorLocation()+FVector(0,-350,110),false,nullptr,ETeleportType::TeleportPhysics);Next();}
 else if(SkaterStage==4){KCHECK(Rider,"Missing on-foot rider");FVector Eye;FRotator View;GetPlayerViewPoint(Eye,View);SetControlRotation((Skater->GetActorLocation()-Eye).Rotation());if(SkaterClock<=.5f)return;FCollisionQueryParams Q(SCENE_QUERY_STAT(SkaterShotAudit),true,Rider);FHitResult Sight;GetWorld()->LineTraceSingleByChannel(Sight,Eye,Eye+View.Vector()*1500,ECC_Visibility,Q);UE_LOG(LogTemp,Display,TEXT("SkaterShotSight: actor=%s eye=%s target=%s"),*GetNameSafe(Sight.GetActor()),*Eye.ToString(),*Skater->GetActorLocation().ToString());KCHECK(Sight.GetActor()==Skater,"Skater camera aim obstructed");const float Before=Mode->TimeRemaining;const bool Fired=Rider->Fire();UE_LOG(LogTemp,Display,TEXT("SkaterShot: fired=%d dead=%d delta=%.2f"),Fired,Skater->bDead,Mode->TimeRemaining-Before);KCHECK(Fired&&Skater->bDead&&FMath::IsNearlyEqual(Mode->TimeRemaining-Before,-10.f,.001f),"Shooting skater did not apply civilian time penalty");KCHECK(Mode->PeopleHit==1,"Same victim counted twice");Finish(true,TEXT("Two skaters/three spectators, circuit, foot reach, push/coast, real bike collision and civilian shooting penalty pass"));}
 if(SkaterClock>(SkaterStage==1?40:8)&&SkaterStage!=99)Finish(false,TEXT("Skater phase timed out"));
#undef KCHECK
#endif
}
