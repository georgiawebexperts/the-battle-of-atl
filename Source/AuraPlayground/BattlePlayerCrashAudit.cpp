#include "BattleBike.h"
#include "BattleRider.h"
#include "BattlePlayerCrash.h"
#include "BattleFallenBike.h"
#include "BattleRoadCar.h"
#include "Components/PoseableMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
void TickBattlePlayerCrashAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ABattleBike> Bike;TWeakObjectPtr<ABattleRoadCar> Car;float Clock=0,CrashTime=0,StartTime=0,Elapsed=0;int Stage=0,Wipeouts=0,Ammo=0;bool Done=false,FallShot=false,FootShot=false,Death=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;
 auto Key=[&](FKey K,bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Shot=[&](const TCHAR* Name){FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);FScreenshotRequest::RequestScreenshot(Folder/Name,false,false);};
 auto Finish=[&](bool Pass,const TCHAR* Why){Key(EKeys::W,false);S.Done=true;UE_LOG(LogTemp,Display,TEXT("PlayerCrashAudit: {\"passed\":%s,\"reason\":\"%s\",\"stage\":%d}"),Pass?TEXT("true"):TEXT("false"),Why,S.Stage);PC->ConsoleCommand(TEXT("quit"));};
 S.Clock+=Dt;if(S.Clock>35){Finish(false,TEXT("Live crash flow timed out"));return;}
 if(S.Stage==0){
  S.Bike=Cast<ABattleBike>(PC->GetPawn());if(!S.Bike.IsValid()){Finish(false,TEXT("No bike"));return;}
  auto* Car=PC->GetWorld()->SpawnActor<ABattleRoadCar>();S.Car=Car;Car->Route={FVector(-18000,13048,350),FVector(-17000,13030,350)};if(!Car->StartRoute()){Finish(false,TEXT("Car setup failed"));return;}Car->SetActorTickEnabled(false);
  FHitResult Ground;FCollisionQueryParams Q;Q.AddIgnoredActor(S.Bike.Get());if(!PC->GetWorld()->LineTraceSingleByChannel(Ground,FVector(-19800,13075,2000),FVector(-19800,13075,-2000),ECC_Visibility,Q)){Finish(false,TEXT("No road"));return;}
  auto* B=S.Bike.Get();B->Ride->StopMovementImmediately();B->Ride->Speed=B->Ride->Recovery=0;B->Ride->Gear=4;B->SetActorLocationAndRotation(Ground.ImpactPoint+FVector(0,0,98),FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);B->Ride->SetMovementMode(MOVE_Walking);B->Ride->bForceNextFloorCheck=true;
  B->RiderHealth=70;B->HurtCooldown=100;S.Wipeouts=B->Ride->Wipeouts;S.Ammo=B->PistolAmmo;S.Death=FParse::Param(FCommandLine::Get(),TEXT("BattleCrashDeathInterrupt"));S.Stage=1;S.Clock=0;Key(EKeys::W,true);return;
 }
 auto* B=S.Bike.Get();if(!B){Finish(false,TEXT("Bike lost"));return;}
 auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(B));
 if(S.Stage==1&&B->bCrashActive){Key(EKeys::W,false);S.Stage=2;S.CrashTime=S.Clock;S.StartTime=Mode?Mode->TimeRemaining:0;S.Elapsed=0;}
 if(S.Stage==2){
  S.Elapsed+=Dt;
  if(!S.FallShot&&S.Clock-S.CrashTime>1){Shot(TEXT("live-fall.png"));S.FallShot=true;}
  if(S.Death&&S.Clock-S.CrashTime>1.5f){B->DamageGrace=0;B->ApplyRiderDamage(1000);S.Stage=5;return;}
  if(auto* Person=Cast<ABattleRider>(PC->GetPawn())){
   const bool State=B->bParked&&!B->bCrashActive&&IsValid(B->PlayerCrash)&&B->PlayerCrash->bRecovered&&Person->ParkedBike==B&&!Person->bWeaponDrawn&&Person->Health==70&&B->PistolAmmo==S.Ammo;
   const float Lost=Mode?S.StartTime-Mode->TimeRemaining:0;const bool Timer=Mode&&FMath::Abs(Lost-S.Elapsed*Mode->FootTimeMultiplier)<.2f;
   UE_LOG(LogTemp,Display,TEXT("PlayerCrashFoot: state=%d timer=%d elapsed=%.3f lost=%.3f"),State,Timer,S.Elapsed,Lost);
   if(!State||!Timer){Finish(false,TEXT("Foot state or timer mismatch"));return;}Shot(TEXT("live-foot.png"));S.Stage=3;
  }
 }
 if(S.Stage==3){
  auto* Person=Cast<ABattleRider>(PC->GetPawn());if(!Person||!IsValid(B->PlayerCrash)||!IsValid(B->PlayerCrash->Fallen)){Finish(false,TEXT("Fallen bike missing"));return;}
  FVector Target=B->PlayerCrash->Fallen->GetActorLocation();Target.Z=Person->GetActorLocation().Z;PC->SetControlRotation((Target-Person->GetActorLocation()).Rotation());
  if(FVector::Dist2D(Target,Person->GetActorLocation())>150)Key(EKeys::W,true);
  else{Key(EKeys::W,false);Key(EKeys::E,true);Key(EKeys::E,false);S.Stage=4;S.CrashTime=S.Clock;}
 }
 if(S.Stage==4&&S.Clock-S.CrashTime>.5f){UE_LOG(LogTemp,Display,TEXT("PlayerCrashRemountState: pawn=%s parked=%d active=%d crash=%d ground=%d ammo=%d health=%.1f wipes=%d"),*GetNameSafe(PC->GetPawn()),B->bParked,B->bCrashActive,IsValid(B->PlayerCrash),B->Ride->IsMovingOnGround(),B->PistolAmmo,B->RiderHealth,B->Ride->Wipeouts-S.Wipeouts);Shot(TEXT("live-remounted.png"));Finish(PC->GetPawn()==B&&!B->bParked&&!B->bCrashActive&&!IsValid(B->PlayerCrash)&&B->Ride->IsMovingOnGround()&&B->PistolAmmo==S.Ammo&&B->RiderHealth==70&&B->Ride->Wipeouts==S.Wipeouts+1,TEXT("Keyboard collision, on-foot recovery and E remount"));}
 if(S.Stage==5&&B->RiderHealth>0&&B->RespawnRemaining<=0){Shot(TEXT("live-death-return.png"));Finish(PC->GetPawn()==B&&!B->bCrashActive&&!IsValid(B->PlayerCrash)&&!B->bParked&&B->RiderHealth==100,TEXT("Death interrupted crash and restored checkpoint control"));}
#endif
}
