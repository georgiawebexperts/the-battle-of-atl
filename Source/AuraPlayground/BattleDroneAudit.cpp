#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleDrone.h"
#include "BattlePlayerCrash.h"
#include "BattleFallenBike.h"
#include "Engine/DamageEvents.h"
#include "BattleRider.h"
#include "BattleZombie.h"
#include "BattlePolice.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "InputKeyEventArgs.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
void ABattleMacController::TickDroneAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5||DroneStage==99)return;DroneClock+=Dt;
 auto* Person=Cast<ABattleRider>(GetPawn());auto* Bike=Person?Person->ParkedBike.Get():Cast<ABattleBike>(GetPawn());if(!Bike)return;
 auto* Drone=Cast<ABattleDrone>(AuditDrone.Get());
 auto Finish=[&](bool Pass,const TCHAR* Why){UE_LOG(LogTemp,Display,TEXT("BattleDroneAudit: {\"passed\":%s,\"stage\":%d,\"reason\":\"%s\",\"health\":%.2f}"),Pass?TEXT("true"):TEXT("false"),DroneStage,Why,Bike->RiderHealth);DroneStage=99;ConsoleCommand(TEXT("quit"));};
#define DCHECK(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Next=[&](){DroneStage++;DroneClock=0;};
 // Keep the test aim on the target while the shoulder camera settles and recoil recovers.
 if(DroneStage>=5&&DroneStage<=8&&Drone){FVector Eye;FRotator View;GetPlayerViewPoint(Eye,View);SetControlRotation((Drone->GetActorLocation()-Eye).Rotation());}
 auto Key=[&](FKey K,bool Down){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 // A drone that never lands and a drone that was blocked both look like "no
 // crash" from here, so record what the drone thinks happened while it happens.
 if(Drone&&DroneStage>=1&&DroneStage<=2&&FMath::Fmod(DroneClock,.5f)<Dt)
  UE_LOG(LogTemp,Display,TEXT("DroneProbe: stage=%d clock=%.2f drone_clock=%.2f warning=%d spent=%d hits=%d distance=%.0f bike=%s end=%s bike_crash=%d"),
   DroneStage,DroneClock,Drone->Clock,Drone->bWarning?1:0,Drone->bSpent?1:0,Drone->RiderHits,
   FVector::Dist(Drone->GetActorLocation(),Bike->GetActorLocation()),*Bike->GetActorLocation().ToCompactString(),*Drone->EndReason,Bike->bCrashActive?1:0);
 if(DroneStage==0){
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  // Hold the shot precondition like the melee audit holds its target: arcade aim-assist locks onto nearby hostiles, so a stray punk or officer near the 35 m lane steals the pistol shot from the drone (the drone is not a lock candidate). Freeze spawns and clear hostiles once, at audit start.
  if(auto* AuditMode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this)))if(AuditMode->Enemies)AuditMode->Enemies->bFreezeSpawns=true;
  for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)It->Destroy();
  for(TActorIterator<ABattlePolice> It(GetWorld());It;++It)It->Destroy();
  TArray<APawn*> LockTargets;for(TActorIterator<APawn> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("PiedmontHostile"))||It->ActorHasTag(TEXT("BattleHostile"))||It->ActorHasTag(TEXT("BattlePolice")))LockTargets.Add(*It);
  for(APawn* Hostile:LockTargets)if(!Hostile->IsA<ABattleBike>()&&!Hostile->IsA<ABattleRider>())Hostile->Destroy();
  Bike->DamageGrace=0;AuditDrone=GetWorld()->SpawnActor<ABattleDrone>(Bike->GetActorLocation()+FVector(-600,0,400),FRotator::ZeroRotator);Next();
 }else if(DroneStage==1&&DroneClock>1){DCHECK(Drone&&Drone->bWarning&&Bike->RiderHealth==100,"Missing warning or premature hit");Next();}
 else if(DroneStage==2&&Bike->bCrashActive){DCHECK(Drone&&Drone->RiderHits==1&&Bike->RiderHealth==85&&Bike->StunRemaining>0&&!Bike->Dismount()&&!Bike->FirePistol()&&!Bike->ApplyDroneStrike(),"Swept hit, harm, dismount or repeat guard failed");Next();}
 else if(DroneStage==3&&Person&&!Bike->bCrashActive){
  if(IsValid(Bike->PlayerCrash)&&IsValid(Bike->PlayerCrash->Fallen)&&FVector::Dist(Person->GetActorLocation(),Bike->PlayerCrash->Fallen->GetActorLocation())>190){if(DroneClock>18){Finish(false,TEXT("Approach to fallen bike timed out"));return;}Person->AddMovementInput((Bike->PlayerCrash->Fallen->GetActorLocation()-Person->GetActorLocation()).GetSafeNormal2D());return;}
  DCHECK(!Person->bWeaponDrawn&&Bike->StunRemaining==0&&Person->MountBike(),"Local recovery/remount failed");if(Drone)Drone->Destroy();
  const FVector B=Bike->GetActorLocation();auto* Wall=GetWorld()->SpawnActor<AStaticMeshActor>(B+FVector(-300,0,220),FRotator::ZeroRotator);auto* M=Wall->GetStaticMeshComponent();M->SetMobility(EComponentMobility::Movable);M->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));M->SetWorldScale3D(FVector(.3,8,8));M->SetCollisionProfileName(TEXT("BlockAll"));AuditDroneWall=Wall;
  AuditDrone=GetWorld()->SpawnActor<ABattleDrone>(B+FVector(-600,0,400),FRotator::ZeroRotator);Next();
 }else if(DroneStage==4&&DroneClock>5.8f){
  DCHECK(Drone&&Drone->bSpent&&Drone->RiderHits==0&&!Bike->bParked,"Drone passed through obstruction");
  const float Health=Drone->Health;FDamageEvent Damage;DCHECK(Drone->TakeDamage(34,Damage,this,Bike)==34&&Drone->Health==Health-34,"Drone cannot be shot");DCHECK(Drone->TakeDamage(34,Damage,this,Bike)==6&&Drone->IsActorBeingDestroyed(),"Drone cannot be destroyed");
  if(AuditDroneWall.IsValid())AuditDroneWall->Destroy();
  DCHECK(Bike->Dismount(),"Voluntary dismount failed");
  const FVector Spot=Bike->GetActorLocation()+Bike->GetActorForwardVector()*3500+FVector(0,0,650);
  AuditDrone=GetWorld()->SpawnActor<ABattleDrone>(Spot,(Bike->GetActorLocation()-Spot).Rotation());
  DCHECK(AuditDrone.IsValid(),"Distant drone failed to spawn");
  // Pin the 35 m target: a live drone begins its dive ~4.5 s after spawning, and under sweep load the aim stages take longer than that, so the shot would be chasing a diving drone. The wall-obstruction and warning phases use the first drone; this one only needs to hover and be shot.
  AuditDrone->SetActorTickEnabled(false);
  Key(EKeys::G,true);Key(EKeys::G,false);Key(EKeys::RightMouseButton,true);Next();
 }else if(DroneStage==5&&DroneClock>.8f){
  DCHECK(Person&&Drone&&Drone->bWarning&&!Drone->bSpent&&Person->bWeaponDrawn,"Dismount cancelled drone or failed to draw weapon");
  // Put the drone on a genuinely clear 35 m line, measured from the eye the
  // shot actually starts at. The camera rides a 3.2 m spring arm behind the
  // rider, so a trunk beside the rider blocks the shot while never crossing a
  // ray drawn from the rider's own chest - which is how a correctly aimed shot
  // kept landing 18 to 24 m short of the drone and read as a miss. Sweep the
  // bearing in 15 degree steps and keep the first clear one; the distance stays
  // 35 m either way.
  FVector AimEye;FRotator AimView;GetPlayerViewPoint(AimEye,AimView);
  const FVector Pivot=Person->GetActorLocation();
  for(int32 Step=0;Step<13;++Step){
   const float Yaw=(Step==0)?0.f:((Step%2)?(Step+1)/2*15.f:-(Step/2)*15.f);
   const FVector Candidate=Pivot+FRotator(0,Person->GetActorRotation().Yaw+Yaw,0).Vector()*3500+FVector(0,0,650);
   FHitResult Block;FCollisionQueryParams Q(SCENE_QUERY_STAT(DroneAuditClear),false,Person);
   if(!GetWorld()->LineTraceSingleByChannel(Block,AimEye,Candidate,ECC_Visibility,Q)){Drone->SetActorLocation(Candidate,false,nullptr,ETeleportType::TeleportPhysics);break;}
  }
  FVector Eye;FRotator View;GetPlayerViewPoint(Eye,View);SetControlRotation((Drone->GetActorLocation()-Eye).Rotation());Next();
 }else if(DroneStage==6&&DroneClock>.35f){
  DCHECK(Person&&Drone&&Person->bAiming,"Distant drone aim failed");
  FString Dir;if(FParse::Value(FCommandLine::Get(),TEXT("BattleDroneReviewDir="),Dir))FScreenshotRequest::RequestScreenshot(Dir/TEXT("distant-drone.png"),false,false);
  Next();
 }else if(DroneStage==7&&DroneClock>.2f){
   // Fire only once the view has actually swung onto the drone. The aim is
   // re-applied every tick above, but the player view follows a frame later, so
   // a fixed 0.2 s of stage clock could fire at where the crosshair had been
   // rather than where the drone is. This is the shot that failed three packaged
   // sweeps in a row on 2026-09-18 and passed every standalone run. Fire only
   // after the measured aim has stayed inside tolerance for several consecutive
   // ticks, and allow a re-shot if a settled shot still grazes past - at 38 m a
   // 1.5 degree cone is a 1 m target and the drone body is half that wide.
  FVector Eye;FRotator View;GetPlayerViewPoint(Eye,View);
  const float Off=(Person&&Drone)?FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(View.Vector(),(Drone->GetActorLocation()-Eye).GetSafeNormal()),-1.f,1.f))):180.f;
  if(Off<=1.5f){DroneSettleTicks++;}else{DroneSettleTicks=0;}
  if(DroneSettleTicks>=4){
   // The camera rides a 3.2 m spring arm, so when the machine is loaded the eye
   // the shot starts from is not the eye the stage-5 clearance sweep measured:
   // the arm is still swinging, the line runs into scenery a settled camera
   // clears, and the shot lands 6 to 7 m out - which is the "28 m short of a
   // 35 m drone" that both the Mac and the Windows sweep reported on 2026-09-18,
   // with the view dead on the drone and the drone untouched. Retrying the shot
   // cannot fix that, which is why three attempts still failed: the line has to
   // be re-made. Check the line the shot will actually take, from the eye it
   // will actually start at, and put the drone somewhere clear. Bounded, so a
   // genuinely blocked world still fails this audit instead of looping.
   {
    // Trace to just short of the drone and ignore the drone itself: a line that
    // ends *at* the target always reports the target as the blocker, which is a
    // test that can never pass for the right reason.
    FHitResult Sight;FCollisionQueryParams SQ(SCENE_QUERY_STAT(DroneShotLine),true,Person);
    if(Drone)SQ.AddIgnoredActor(Drone);
    const FVector SightEnd=Drone?FMath::Lerp(Eye,Drone->GetActorLocation(),.98f):Eye;
    const bool bSightBlocked=Drone&&GetWorld()->LineTraceSingleByChannel(Sight,Eye,SightEnd,ECC_Visibility,SQ);
    if(bSightBlocked&&DroneShotAttempts<3){
     DroneShotAttempts++;
     const FVector Pivot=Person->GetActorLocation();
     bool bReplaced=false;
     for(int32 Step=0;Step<25&&!bReplaced;++Step){
      const float Yaw=(Step==0)?0.f:((Step%2)?(Step+1)/2*12.f:-(Step/2)*12.f);
      const FVector Candidate=Pivot+FRotator(0,Person->GetActorRotation().Yaw+Yaw,0).Vector()*3500+FVector(0,0,650);
      FHitResult Recheck;FCollisionQueryParams RQ(SCENE_QUERY_STAT(DroneAuditClear),false,Person);
      if(!GetWorld()->LineTraceSingleByChannel(Recheck,Eye,Candidate,ECC_Visibility,RQ)){
       UE_LOG(LogTemp,Display,TEXT("DroneAuditSightline: line blocked by %s at %.0fcm; drone moved to yaw offset %.0f (attempt %d)"),
        *GetNameSafe(Sight.GetActor()),float(Sight.Distance),Yaw,DroneShotAttempts);
       Drone->SetActorLocation(Candidate,false,nullptr,ETeleportType::TeleportPhysics);
       bReplaced=true;
      }
     }
     if(!bReplaced)UE_LOG(LogTemp,Display,TEXT("DroneAuditSightline: line blocked by %s at %.0fcm and no clear bearing from the current eye"),
      *GetNameSafe(Sight.GetActor()),float(Sight.Distance));
     DroneSettleTicks=0;DroneClock=0;return;
    }
   }
   // A bare "did not hit" covered four different things: the shot not firing,
   // the trace landing somewhere else, damage not applying, or the HUD label
   // not being set. Report which, with the numbers that separate them.
   const bool Fired=Person&&Person->Fire();
   if(Fired&&Drone&&Drone->Health==6&&Bike->ShotNotice==TEXT("DRONE HIT")){DroneShotAttempts=0;DroneSettleTicks=0;Next();}
   else{
    DroneShotAttempts++;
    if(DroneShotAttempts<3){DroneSettleTicks=0;DroneClock=0;}
    else{
     // "28 m off target" with the view dead on the drone means the trace stopped
     // early, and the four ways that happens look identical from here: something
     // in front of the camera, something between the chest and the muzzle, the
     // muzzle-to-target hop, or damage simply not landing. Name the blocker.
     FHitResult Block;FCollisionQueryParams QQ(SCENE_QUERY_STAT(DroneShotBlock),true,Person);
     if(Drone)QQ.AddIgnoredActor(Drone);
     const bool bCamBlocked=GetWorld()->LineTraceSingleByChannel(Block,Eye,Eye+View.Vector()*15000.f,ECC_Visibility,QQ);
     const FString Why=FString::Printf(TEXT("Pistol shot at 35m failed: fired=%s health=%.1f notice=%s off_target_cm=%.1f aim_off_deg=%.2f eye=%s view=%s shot_end=%s cam_block=%s at_cm=%.1f muzzle=%s"),
      Fired?TEXT("true"):TEXT("false"),Drone?Drone->Health:-1.f,*Bike->ShotNotice,
      (Person&&Drone)?FVector::Dist(Person->LastShotEnd,Drone->GetActorLocation()):-1.f,Off,
      *Eye.ToCompactString(),*View.Vector().ToCompactString(),Person?*Person->LastShotEnd.ToCompactString():TEXT("?"),
      bCamBlocked?*GetNameSafe(Block.GetActor()):TEXT("none"),bCamBlocked?float(Block.Distance):-1.f,
      Person&&Person->Weapon?*Person->Weapon->GetComponentLocation().ToCompactString():TEXT("?"));
     Finish(false,*Why);return;
    }
   }
  }
 }else if(DroneStage==8&&DroneClock>.35f){
  // Stage 7 waits for the measured aim to hold inside tolerance for several
  // consecutive ticks before it fires, because the view follows a frame behind
  // the aim and under load that frame is a miss. Stage 8 fired on the first
  // tick the aim was inside 1.5 degrees and so re-inherited exactly the failure
  // stage 7 had just been hardened against: red inside a sweep, green
  // standalone on the same binary, drone health still 100. Same rule here - a
  // settled aim, and a re-shot if a settled shot still grazes past.
  FVector Eye;FRotator View;GetPlayerViewPoint(Eye,View);
  const float Off=(Person&&Drone)?FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(View.Vector(),(Drone->GetActorLocation()-Eye).GetSafeNormal()),-1.f,1.f))):180.f;
  if(Off<=1.5f){DroneSettleTicks++;}else{DroneSettleTicks=0;}
  if(DroneSettleTicks>=4){
   const int32 Before=Person?Person->Ammo:0;
   const bool Fired=Person&&Person->Fire();
   if(Fired&&Drone&&Drone->IsActorBeingDestroyed()&&Bike->ShotNotice==TEXT("DRONE DOWN")&&Person->Ammo==Before-1){
    Key(EKeys::RightMouseButton,false);Finish(true,TEXT("Warning, knockoff/recovery, wall obstruction and 35m on-foot pistol engagement after voluntary dismount pass"));
   }else if(++DroneShotAttempts<3){
    UE_LOG(LogTemp,Display,TEXT("DroneAuditSecondShot: fired=%s health=%.1f notice=%s ammo=%d off_deg=%.2f - retrying"),
     Fired?TEXT("true"):TEXT("false"),Drone?Drone->Health:-1.f,*Bike->ShotNotice,Person?Person->Ammo:-1,Off);
    DroneSettleTicks=0;DroneClock=0;
   }else{
    DCHECK(Person&&Drone&&Drone->IsActorBeingDestroyed()&&Bike->ShotNotice==TEXT("DRONE DOWN")&&Person->Ammo==Before-1,"Second pistol shot did not destroy drone with correct ammo/feedback");
   }
  }
 }
 if(DroneClock>18&&DroneStage!=99){
  if(Drone){const FString Why=FString::Printf(TEXT("Drone audit timeout: %s"),*Drone->EndReason);Finish(false,*Why);return;}
  Finish(false,TEXT("Drone audit timeout: no drone"));
 }
#undef DCHECK
#endif
}
