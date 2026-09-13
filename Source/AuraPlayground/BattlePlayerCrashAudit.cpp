#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleSpareBikes.h"
#include "BattlePlayerCrash.h"
#include "BattleFallenBike.h"
#include "BattleRoadCar.h"
#include "PiedmontPedestrian.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BattlePolice.h"
#include "BattleDrone.h"
#include "PiedmontDarkZone.h"
#include "Components/BoxComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
void TickBattlePlayerCrashAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{FString Obstacle;TWeakObjectPtr<AActor> ImpactTarget;TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ABattleBike> Bike;TWeakObjectPtr<ABattleRoadCar> Car;TWeakObjectPtr<ABattlePolice> Officer;TWeakObjectPtr<ABattleDrone> Drone;TWeakObjectPtr<AActor> RecoveryBlocker;FVector RecoveryAnchor,HoldHead;float BlockClock=0;bool BlockPlaced=false,BlockReleased=false,HoldSample=false,HoldVerified=false;TWeakObjectPtr<APiedmontDarkZone> LightZone;TWeakObjectPtr<USceneComponent> HeadParent,TailParent;FTransform HeadRelative,TailRelative;bool LightChecked=false;float LightClock=0;float ExpectedHealth=70;bool Taser=false,DroneMode=false,WarningSeen=false;float Clock=0,CrashTime=0,StartTime=0,StartRunTime=0,Elapsed=0;int RecoveryShot=0;int Stage=0,Wipeouts=0,Ammo=0,Cycles=0;bool Done=false,FallShot=false,FootShot=false,Death=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;
 auto Key=[&](FKey K,bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Shot=[&](const TCHAR* Name){FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);FScreenshotRequest::RequestScreenshot(Folder/(S.Cycles?FString::Printf(TEXT("cycle%d-%s"),S.Cycles+1,Name):FString(Name)),false,false);};
 auto Finish=[&](bool Pass,const TCHAR* Why){Key(EKeys::W,false);S.Done=true;UE_LOG(LogTemp,Display,TEXT("PlayerCrashAudit: {\"passed\":%s,\"reason\":\"%s\",\"stage\":%d}"),Pass?TEXT("true"):TEXT("false"),Why,S.Stage);PC->ConsoleCommand(TEXT("quit"));};
 S.Clock+=Dt;if(S.Clock>35){Finish(false,TEXT("Live crash flow timed out"));return;}
 if(S.Stage==0){
  S.Bike=Cast<ABattleBike>(PC->GetPawn());if(!S.Bike.IsValid()){Finish(false,TEXT("No bike"));return;}
  if(S.Car.IsValid())S.Car->Destroy();S.FallShot=false;
  S.Taser=FParse::Param(FCommandLine::Get(),TEXT("BattleCrashTaser"));S.DroneMode=FParse::Param(FCommandLine::Get(),TEXT("BattleCrashDrone"));
  auto* Car=PC->GetWorld()->SpawnActor<ABattleRoadCar>();S.Car=Car;Car->Route={FVector(-18000,13048,350),FVector(-17000,13030,350)};if(!Car->StartRoute()){Finish(false,TEXT("Car setup failed"));return;}Car->SetActorTickEnabled(false);
  FHitResult Ground;FCollisionQueryParams Q;Q.AddIgnoredActor(S.Bike.Get());if(!PC->GetWorld()->LineTraceSingleByChannel(Ground,FVector(-19800,13075,2000),FVector(-19800,13075,-2000),ECC_Visibility,Q)){Finish(false,TEXT("No road"));return;}
  auto* B=S.Bike.Get();S.HeadParent=B->Headlight->GetAttachParent();S.TailParent=B->TailLight->GetAttachParent();S.HeadRelative=B->Headlight->GetRelativeTransform();S.TailRelative=B->TailLight->GetRelativeTransform();B->Ride->StopMovementImmediately();B->Ride->Speed=B->Ride->Recovery=0;B->Ride->Gear=4;B->SetActorLocationAndRotation(Ground.ImpactPoint+FVector(0,0,98),FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);B->Ride->SetMovementMode(MOVE_Walking);B->Ride->bForceNextFloorCheck=true;
  B->RiderHealth=70;B->HurtCooldown=100;B->DamageGrace=100;S.Wipeouts=B->Ride->Wipeouts;S.Ammo=B->PistolAmmo;S.Death=FParse::Param(FCommandLine::Get(),TEXT("BattleCrashDeathInterrupt"));S.Stage=1;S.Clock=0;
  if(S.Taser||S.DroneMode){
   Car->Destroy();B->DamageGrace=S.Taser?100:0;B->RiderHealth=100;S.ExpectedHealth=S.Taser?100:85;
   if(S.Taser){S.Officer=PC->GetWorld()->SpawnActor<ABattlePolice>(B->GetActorLocation()+FVector(350,0,0),FRotator::ZeroRotator);if(!S.Officer.IsValid()){Finish(false,TEXT("Officer spawn failed"));return;}S.Officer->Cooldown=0;}
   else S.Drone=PC->GetWorld()->SpawnActor<ABattleDrone>(B->GetActorLocation()+FVector(-600,0,400),FRotator::ZeroRotator);
  }else {
   FParse::Value(FCommandLine::Get(),TEXT("BattleCrashObstacle="),S.Obstacle);
   if(!S.Obstacle.IsEmpty()){
    Car->Destroy();B->DamageGrace=0;B->Ride->Gear=1;B->Ride->bRealHandling=true;
    const FVector Place=B->GetActorLocation()+FVector(600,0,0);
    if(S.Obstacle==TEXT("person")){
     auto* Person=PC->GetWorld()->SpawnActor<APiedmontPedestrian>(Place,FRotator(0,180,0));S.ImpactTarget=Person;
     Person->SetActorTickEnabled(false);Person->GetCharacterMovement()->DisableMovement();
    }else {
     auto* A=PC->GetWorld()->SpawnActor<AActor>();S.ImpactTarget=A;auto* Box=NewObject<UBoxComponent>(A);A->AddInstanceComponent(Box);A->SetRootComponent(Box);Box->SetBoxExtent(S.Obstacle==TEXT("tree")?FVector(40,40,180):FVector(40,250,180));Box->SetCollisionProfileName(TEXT("BlockAll"));Box->RegisterComponent();A->SetActorLocation(Place);if(S.Obstacle==TEXT("tree"))A->Tags.Add(TEXT("RideTree"));
    }
   }
   Key(EKeys::W,true);
  }return;
 }
 auto* B=S.Bike.Get();if(!B){Finish(false,TEXT("Bike lost"));return;}
 auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(B));
 if(S.Stage==1){if(S.Taser&&S.Officer.IsValid()&&S.Officer->bWarning)S.WarningSeen=true;if(S.DroneMode&&S.Drone.IsValid()&&S.Drone->bWarning)S.WarningSeen=true;}
 if(S.Stage==1&&B->bCrashActive){
  if(!S.Obstacle.IsEmpty()){
   if(B->RiderHealth>=70||B->RiderHealth<39||!Mode||Mode->LastTimeDelta!=-10||Mode->TimeNotice!=TEXT("CRASH")){Finish(false,TEXT("Obstacle collision did not apply harm and crash penalty"));return;}
   if(S.Obstacle==TEXT("person")){auto* Person=Cast<APiedmontPedestrian>(S.ImpactTarget.Get());if(!Person||Person->BikeContacts<1){Finish(false,TEXT("Pedestrian did not react to impact"));return;}}
   if(S.Obstacle==TEXT("tree")&&B->Ride->TreeContacts<1){Finish(false,TEXT("Tree collision not recorded"));return;}
   S.ExpectedHealth=B->RiderHealth;B->HurtCooldown=100;B->DamageGrace=100;
  }
  if(S.Taser||S.DroneMode){
   const bool Hit=S.Taser?B->TaserHits==1&&Mode&&Mode->LastTimeDelta==-10&&Mode->TimeNotice==TEXT("TASED"):S.Drone.IsValid()&&S.Drone->RiderHits==1;
   const bool RepeatBlocked=S.Taser?!B->ApplyTaser():!B->ApplyDroneStrike();
   if(!S.WarningSeen||!Hit||!RepeatBlocked||B->RiderHealth!=S.ExpectedHealth||B->StunRemaining<=0||B->Dismount()||B->FirePistol()){Finish(false,TEXT("Hazard warning, harm, penalty or knockdown guards failed"));return;}
   B->DamageGrace=100;B->HurtCooldown=100;
   if(S.Officer.IsValid())S.Officer->SetActorTickEnabled(false);
  }
  Key(EKeys::W,false);S.Stage=2;S.CrashTime=S.Clock;S.StartTime=Mode?Mode->TimeRemaining:0;S.StartRunTime=Cast<ABattleParkMode>(Mode)?Cast<ABattleParkMode>(Mode)->RunElapsed:0;S.Elapsed=0;return;}
 if(S.Stage==2){
  S.Elapsed+=Dt;
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleDetailedRider"))&&IsValid(B->PlayerCrash)&&B->PlayerCrash->GetRecoveryPose()&&S.RecoveryShot<3&&B->PlayerCrash->GetRecoveryTime()>.15f+S.RecoveryShot*1.4f){
   Shot(*FString::Printf(TEXT("detailed-recovery%d.png"),S.RecoveryShot));S.RecoveryShot++;
  }
  const bool CrowdBlock=FParse::Param(FCommandLine::Get(),TEXT("BattleCrashCrowdBlock")),SolidBlock=FParse::Param(FCommandLine::Get(),TEXT("BattleCrashSolidBlock"));
  if((CrowdBlock||SolidBlock)&&IsValid(B->PlayerCrash)){
   auto* Pose=B->PlayerCrash->GetRecoveryPose();const float Progress=B->PlayerCrash->GetRecoveryTime();
   if(!S.BlockPlaced&&Pose&&Progress>4){
    S.RecoveryAnchor=Pose->GetComponentLocation()+FVector(0,0,90);S.RecoveryBlocker=PC->GetWorld()->SpawnActor<AActor>();
    auto* Box=NewObject<UBoxComponent>(S.RecoveryBlocker.Get());S.RecoveryBlocker->AddInstanceComponent(Box);S.RecoveryBlocker->SetRootComponent(Box);Box->SetBoxExtent(SolidBlock?FVector(180,180,100):FVector(25,25,88));Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);Box->SetCollisionObjectType(SolidBlock?ECC_WorldStatic:ECC_Pawn);Box->SetCollisionResponseToAllChannels(ECR_Block);Box->SetWorldLocation(S.RecoveryAnchor);Box->RegisterComponent();S.BlockClock=S.Clock;S.BlockPlaced=true;
   }
   if(SolidBlock&&S.RecoveryBlocker.IsValid()){
    if(Cast<ABattleRider>(PC->GetPawn())){Finish(false,TEXT("Recovered inside solid blocker"));return;}
    if(Pose&&Progress>5.7f){
     if(!S.HoldSample){S.HoldHead=Pose->GetSocketLocation(TEXT("Head"));S.HoldSample=true;}
     if(Progress>6.7f){if(FVector::Dist(S.HoldHead,Pose->GetSocketLocation(TEXT("Head")))>.1f){Finish(false,TEXT("Blocked final pose moved"));return;}S.HoldVerified=true;}
    }
    if(S.Clock-S.BlockClock>3.5f){S.RecoveryBlocker->Destroy();S.BlockReleased=true;}
   }
  }
  if(!S.FallShot&&S.Clock-S.CrashTime>1){Shot(TEXT("live-fall.png"));S.FallShot=true;}
  if(S.Death&&!FParse::Param(FCommandLine::Get(),TEXT("BattleCrashDeathOnFoot"))&&S.Clock-S.CrashTime>1.5f){B->DamageGrace=0;B->ApplyRiderDamage(1000);S.Stage=5;return;}
  if(auto* Person=Cast<ABattleRider>(PC->GetPawn())){
   if(CrowdBlock||SolidBlock){
    const float Shift=FVector::Dist(Person->GetActorLocation(),S.RecoveryAnchor);
    bool Clear=true;if(S.RecoveryBlocker.IsValid()){FCollisionQueryParams Q;Q.AddIgnoredActor(B);Q.AddIgnoredActor(Person);Clear=!PC->GetWorld()->OverlapBlockingTestByChannel(Person->GetActorLocation(),FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,88),Q);}
    const bool SpacePass=S.BlockPlaced&&Clear&&(CrowdBlock?(Shift>=60&&Shift<=115):(S.BlockReleased&&S.HoldVerified&&Shift<5));
    UE_LOG(LogTemp,Display,TEXT("RecoverySpaceAudit: {\"passed\":%s,\"shift_cm\":%.3f,\"solid_hold_verified\":%s}"),SpacePass?TEXT("true"):TEXT("false"),Shift,S.HoldVerified?TEXT("true"):TEXT("false"));
    if(!SpacePass){Finish(false,TEXT("Recovery obstacle resolution failed"));return;}if(S.RecoveryBlocker.IsValid())S.RecoveryBlocker->Destroy();
   }
   const bool State=B->bParked&&!B->bCrashActive&&IsValid(B->PlayerCrash)&&B->PlayerCrash->bRecovered&&Person->ParkedBike==B&&!Person->bWeaponDrawn&&Person->Health==S.ExpectedHealth&&B->PistolAmmo==S.Ammo;
   const float Lost=Mode?S.StartTime-Mode->TimeRemaining:0;const auto* ParkMode=Cast<ABattleParkMode>(Mode);const float TimerElapsed=ParkMode?ParkMode->RunElapsed-S.StartRunTime:0;const bool Timer=ParkMode&&FMath::Abs(Lost-TimerElapsed*Mode->FootTimeMultiplier)<.08f;
   UE_LOG(LogTemp,Display,TEXT("PlayerCrashFoot: state=%d timer=%d elapsed=%.3f lost=%.3f"),State,Timer,TimerElapsed,Lost);
   if(!State||!Timer){Finish(false,TEXT("Foot state or timer mismatch"));return;}Shot(TEXT("live-foot.png"));
   if(S.Death){B->DamageGrace=0;Person->TakeDamage(1000,FDamageEvent(),PC,nullptr);S.Stage=5;return;}
   if(FParse::Param(FCommandLine::Get(),TEXT("BattleCrashSpareBike"))){
    TArray<FVector> Sites;BattleSpareBikes::Locations(PC,Sites);bool Found=false;
    FCollisionQueryParams Q(SCENE_QUERY_STAT(CrashSpareApproach),false,Person);Q.AddIgnoredActor(B);
    for(FVector Site:Sites){for(FVector Offset:{FVector(145,0,0),FVector(-145,0,0),FVector(0,145,0),FVector(0,-145,0)}){
     FHitResult Ground;FVector P=Site+Offset;if(!PC->GetWorld()->LineTraceSingleByChannel(Ground,P+FVector(0,0,180),P-FVector(0,0,300),ECC_Visibility,Q)||Ground.ImpactNormal.Z<.8f)continue;P=Ground.ImpactPoint+FVector(0,0,91);
     if(PC->GetWorld()->OverlapBlockingTestByChannel(P,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,88),Q))continue;Person->SetActorLocation(P,false,nullptr,ETeleportType::TeleportPhysics);if(BattleSpareBikes::Nearest(Person)){Found=true;break;}
    }if(Found)break;}
    if(!Found){Finish(false,TEXT("No accessible spare after crash"));return;}
   }
   S.Stage=3;
  }
 }
 if(S.Stage==3){
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleCrashSpareBike"))){Key(EKeys::E,true);Key(EKeys::E,false);S.Stage=4;S.CrashTime=S.Clock;return;}
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleCrashLights"))&&!S.LightChecked){
   if(!IsValid(B->PlayerCrash)||!IsValid(B->PlayerCrash->Fallen)){Finish(false,TEXT("Missing fallen bike for lamp test"));return;}
   if(!S.LightZone.IsValid()){
    S.LightZone=PC->GetWorld()->SpawnActor<APiedmontDarkZone>(B->Headlight->GetComponentLocation(),FRotator::ZeroRotator);S.LightZone->Bounds->SetBoxExtent(FVector(25));S.LightClock=S.Clock;return;
   }
   if(S.Clock-S.LightClock<.8f)return;
   const bool Attached=B->Headlight->GetAttachParent()==B->PlayerCrash->Fallen->Frame&&B->TailLight->GetAttachParent()==B->PlayerCrash->Fallen->Frame;
   const bool Probe=S.LightZone->Contains(B->Headlight->GetComponentLocation())&&!S.LightZone->Contains(B->GetActorLocation());
   const bool Lit=B->bLightsOn&&B->Headlight->IsVisible()&&B->TailLight->IsVisible();
   UE_LOG(LogTemp,Display,TEXT("FallenBikeLights: attached=%d lamp_only_zone=%d lit=%d"),Attached,Probe,Lit);
   if(!Attached||!Probe||!Lit){Finish(false,TEXT("Fallen lamp attachment or darkness probe failed"));return;}
   Shot(TEXT("fallen-lights.png"));S.LightZone->Destroy();S.LightChecked=true;
  }
  auto* Person=Cast<ABattleRider>(PC->GetPawn());if(!Person||!IsValid(B->PlayerCrash)||!IsValid(B->PlayerCrash->Fallen)){Finish(false,TEXT("Fallen bike missing"));return;}
  const float MountDistance=FVector::Dist(B->PlayerCrash->Fallen->GetActorLocation(),Person->GetActorLocation());
  FVector Target=B->PlayerCrash->Fallen->GetActorLocation();Target.Z=Person->GetActorLocation().Z;PC->SetControlRotation((Target-Person->GetActorLocation()).Rotation());
  if(MountDistance>190)Key(EKeys::W,true);
  else{Key(EKeys::W,false);Key(EKeys::E,true);Key(EKeys::E,false);S.Stage=4;S.CrashTime=S.Clock;}
 }
 if(S.Stage==4&&S.Clock-S.CrashTime>(S.LightChecked?2.f:.5f)){
  int32 Crashes=0,Fallen=0;for(TActorIterator<ABattlePlayerCrash> It(PC->GetWorld());It;++It)Crashes++;for(TActorIterator<ABattleFallenBike> It(PC->GetWorld());It;++It)Fallen++;
  const bool Restored=B->Headlight->GetAttachParent()==S.HeadParent.Get()&&B->TailLight->GetAttachParent()==S.TailParent.Get()&&B->Headlight->GetRelativeTransform().Equals(S.HeadRelative,.001f)&&B->TailLight->GetRelativeTransform().Equals(S.TailRelative,.001f);
  if(S.LightChecked)UE_LOG(LogTemp,Display,TEXT("FallenBikeLightsRestored: mount=%d daylight_off=%d"),Restored,!B->bLightsOn&&!B->Headlight->IsVisible()&&!B->TailLight->IsVisible());
  const bool Pass=Restored&&(!S.LightChecked||(!B->bLightsOn&&!B->Headlight->IsVisible()&&!B->TailLight->IsVisible()))&&PC->GetPawn()==B&&!B->bParked&&!B->bCrashActive&&!IsValid(B->PlayerCrash)&&B->Ride->IsMovingOnGround()&&B->PistolAmmo==S.Ammo&&B->RiderHealth==S.ExpectedHealth&&B->Ride->Wipeouts==S.Wipeouts+1&&Crashes==0&&Fallen==0;
  UE_LOG(LogTemp,Display,TEXT("PlayerCrashCycle: cycle=%d passed=%d crash_actors=%d fallen_bikes=%d"),S.Cycles+1,Pass,Crashes,Fallen);Shot(TEXT("live-remounted.png"));
  if(Pass&&S.Cycles==0&&FParse::Param(FCommandLine::Get(),TEXT("BattleCrashRepeat"))){S.Cycles++;S.Stage=0;return;}
  Finish(Pass,S.Cycles?TEXT("Two keyboard collisions, recoveries and E remounts; no leftover crash actors"):S.Taser?TEXT("Warned police taser, -10 seconds, physical fall, on-foot recovery and E remount"):S.DroneMode?TEXT("Warned drone sweep, 15 harm, physical fall, on-foot recovery and E remount"):TEXT("Keyboard collision, on-foot recovery and E remount"));
 }
 if(S.Stage==5&&B->RiderHealth>0&&B->RespawnRemaining<=0){Shot(TEXT("live-death-return.png"));Finish(B->Headlight->GetAttachParent()==S.HeadParent.Get()&&B->TailLight->GetAttachParent()==S.TailParent.Get()&&B->Headlight->GetRelativeTransform().Equals(S.HeadRelative,.001f)&&B->TailLight->GetRelativeTransform().Equals(S.TailRelative,.001f)&&PC->GetPawn()==B&&!B->bCrashActive&&!IsValid(B->PlayerCrash)&&!B->bParked&&B->RiderHealth==100,FParse::Param(FCommandLine::Get(),TEXT("BattleCrashDeathOnFoot"))?TEXT("On-foot death cleaned up fallen bike and restored checkpoint control"):TEXT("Death interrupted crash and restored checkpoint control"));}
#endif
}
