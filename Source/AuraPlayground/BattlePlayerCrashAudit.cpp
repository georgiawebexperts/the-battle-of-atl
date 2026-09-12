#include "BattleBike.h"
#include "BattleRider.h"
#include "BattlePlayerCrash.h"
#include "BattleFallenBike.h"
#include "BattleRoadCar.h"
#include "BattlePolice.h"
#include "BattleDrone.h"
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
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ABattleBike> Bike;TWeakObjectPtr<ABattleRoadCar> Car;TWeakObjectPtr<ABattlePolice> Officer;TWeakObjectPtr<ABattleDrone> Drone;float ExpectedHealth=70;bool Taser=false,DroneMode=false,WarningSeen=false;float Clock=0,CrashTime=0,StartTime=0,Elapsed=0;int Stage=0,Wipeouts=0,Ammo=0,Cycles=0;bool Done=false,FallShot=false,FootShot=false,Death=false;};static FState S;
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
  auto* B=S.Bike.Get();B->Ride->StopMovementImmediately();B->Ride->Speed=B->Ride->Recovery=0;B->Ride->Gear=4;B->SetActorLocationAndRotation(Ground.ImpactPoint+FVector(0,0,98),FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);B->Ride->SetMovementMode(MOVE_Walking);B->Ride->bForceNextFloorCheck=true;
  B->RiderHealth=70;B->HurtCooldown=100;B->DamageGrace=100;S.Wipeouts=B->Ride->Wipeouts;S.Ammo=B->PistolAmmo;S.Death=FParse::Param(FCommandLine::Get(),TEXT("BattleCrashDeathInterrupt"));S.Stage=1;S.Clock=0;
  if(S.Taser||S.DroneMode){
   Car->Destroy();B->DamageGrace=S.Taser?100:0;B->RiderHealth=100;S.ExpectedHealth=S.Taser?100:85;
   if(S.Taser){S.Officer=PC->GetWorld()->SpawnActor<ABattlePolice>(B->GetActorLocation()+FVector(350,0,0),FRotator::ZeroRotator);if(!S.Officer.IsValid()){Finish(false,TEXT("Officer spawn failed"));return;}S.Officer->Cooldown=0;}
   else S.Drone=PC->GetWorld()->SpawnActor<ABattleDrone>(B->GetActorLocation()+FVector(-600,0,400),FRotator::ZeroRotator);
  }else Key(EKeys::W,true);return;
 }
 auto* B=S.Bike.Get();if(!B){Finish(false,TEXT("Bike lost"));return;}
 auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(B));
 if(S.Stage==1){if(S.Taser&&S.Officer.IsValid()&&S.Officer->bWarning)S.WarningSeen=true;if(S.DroneMode&&S.Drone.IsValid()&&S.Drone->bWarning)S.WarningSeen=true;}
 if(S.Stage==1&&B->bCrashActive){
  if(S.Taser||S.DroneMode){
   const bool Hit=S.Taser?B->TaserHits==1&&Mode&&Mode->LastTimeDelta==-10&&Mode->TimeNotice==TEXT("TASED"):S.Drone.IsValid()&&S.Drone->RiderHits==1;
   const bool RepeatBlocked=S.Taser?!B->ApplyTaser():!B->ApplyDroneStrike();
   if(!S.WarningSeen||!Hit||!RepeatBlocked||B->RiderHealth!=S.ExpectedHealth||B->StunRemaining<=0||B->Dismount()||B->FirePistol()){Finish(false,TEXT("Hazard warning, harm, penalty or knockdown guards failed"));return;}
   B->DamageGrace=100;B->HurtCooldown=100;
   if(S.Officer.IsValid())S.Officer->SetActorTickEnabled(false);
  }
  Key(EKeys::W,false);S.Stage=2;S.CrashTime=S.Clock;S.StartTime=Mode?Mode->TimeRemaining:0;S.Elapsed=0;}
 if(S.Stage==2){
  S.Elapsed+=Dt;
  if(!S.FallShot&&S.Clock-S.CrashTime>1){Shot(TEXT("live-fall.png"));S.FallShot=true;}
  if(S.Death&&!FParse::Param(FCommandLine::Get(),TEXT("BattleCrashDeathOnFoot"))&&S.Clock-S.CrashTime>1.5f){B->DamageGrace=0;B->ApplyRiderDamage(1000);S.Stage=5;return;}
  if(auto* Person=Cast<ABattleRider>(PC->GetPawn())){
   const bool State=B->bParked&&!B->bCrashActive&&IsValid(B->PlayerCrash)&&B->PlayerCrash->bRecovered&&Person->ParkedBike==B&&!Person->bWeaponDrawn&&Person->Health==S.ExpectedHealth&&B->PistolAmmo==S.Ammo;
   const float Lost=Mode?S.StartTime-Mode->TimeRemaining:0;const bool Timer=Mode&&FMath::Abs(Lost-S.Elapsed*Mode->FootTimeMultiplier)<.2f;
   UE_LOG(LogTemp,Display,TEXT("PlayerCrashFoot: state=%d timer=%d elapsed=%.3f lost=%.3f"),State,Timer,S.Elapsed,Lost);
   if(!State||!Timer){Finish(false,TEXT("Foot state or timer mismatch"));return;}Shot(TEXT("live-foot.png"));
   if(S.Death){B->DamageGrace=0;Person->TakeDamage(1000,FDamageEvent(),PC,nullptr);S.Stage=5;return;}
   S.Stage=3;
  }
 }
 if(S.Stage==3){
  auto* Person=Cast<ABattleRider>(PC->GetPawn());if(!Person||!IsValid(B->PlayerCrash)||!IsValid(B->PlayerCrash->Fallen)){Finish(false,TEXT("Fallen bike missing"));return;}
  const float MountDistance=FVector::Dist(B->PlayerCrash->Fallen->GetActorLocation(),Person->GetActorLocation());
  FVector Target=B->PlayerCrash->Fallen->GetActorLocation();Target.Z=Person->GetActorLocation().Z;PC->SetControlRotation((Target-Person->GetActorLocation()).Rotation());
  if(MountDistance>190)Key(EKeys::W,true);
  else{Key(EKeys::W,false);Key(EKeys::E,true);Key(EKeys::E,false);S.Stage=4;S.CrashTime=S.Clock;}
 }
 if(S.Stage==4&&S.Clock-S.CrashTime>.5f){
  int32 Crashes=0,Fallen=0;for(TActorIterator<ABattlePlayerCrash> It(PC->GetWorld());It;++It)Crashes++;for(TActorIterator<ABattleFallenBike> It(PC->GetWorld());It;++It)Fallen++;
  const bool Pass=PC->GetPawn()==B&&!B->bParked&&!B->bCrashActive&&!IsValid(B->PlayerCrash)&&B->Ride->IsMovingOnGround()&&B->PistolAmmo==S.Ammo&&B->RiderHealth==S.ExpectedHealth&&B->Ride->Wipeouts==S.Wipeouts+1&&Crashes==0&&Fallen==0;
  UE_LOG(LogTemp,Display,TEXT("PlayerCrashCycle: cycle=%d passed=%d crash_actors=%d fallen_bikes=%d"),S.Cycles+1,Pass,Crashes,Fallen);Shot(TEXT("live-remounted.png"));
  if(Pass&&S.Cycles==0&&FParse::Param(FCommandLine::Get(),TEXT("BattleCrashRepeat"))){S.Cycles++;S.Stage=0;return;}
  Finish(Pass,S.Cycles?TEXT("Two keyboard collisions, recoveries and E remounts; no leftover crash actors"):S.Taser?TEXT("Warned police taser, -10 seconds, physical fall, on-foot recovery and E remount"):S.DroneMode?TEXT("Warned drone sweep, 15 harm, physical fall, on-foot recovery and E remount"):TEXT("Keyboard collision, on-foot recovery and E remount"));
 }
 if(S.Stage==5&&B->RiderHealth>0&&B->RespawnRemaining<=0){Shot(TEXT("live-death-return.png"));Finish(PC->GetPawn()==B&&!B->bCrashActive&&!IsValid(B->PlayerCrash)&&!B->bParked&&B->RiderHealth==100,FParse::Param(FCommandLine::Get(),TEXT("BattleCrashDeathOnFoot"))?TEXT("On-foot death cleaned up fallen bike and restored checkpoint control"):TEXT("Death interrupted crash and restored checkpoint control"));}
#endif
}
