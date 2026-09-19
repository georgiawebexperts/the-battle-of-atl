#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattlePlayerCrash.h"
#include "BattleFallenBike.h"
#include "BattlePolice.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "BattleZombie.h"
#include "BattleQuest.h"
#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
#include "Components/CapsuleComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "AIController.h"
#include "Camera/CameraActor.h"
#include "GameFramework/HUD.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
void ABattleMacController::TickTroubleAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5||TroubleStage==99)return;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* Person=Cast<ABattleRider>(GetPawn());auto* Bike=Person?Person->ParkedBike.Get():Cast<ABattleBike>(GetPawn());if(!Mode||!Bike)return;TroubleClock+=Dt;
 auto* Officer=Cast<ABattlePolice>(TroubleOfficer.Get());
 auto Finish=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BattleTroubleAudit: {\"passed\":%s,\"stage\":%d,\"reason\":\"%s\",\"people\":%d,\"police_spawned\":%d,\"taser_hits\":%d}"),Pass?TEXT("true"):TEXT("false"),TroubleStage,Reason,Mode->PeopleHit,Mode->PoliceSpawned,Bike->TaserHits);TroubleStage=99;ConsoleCommand(TEXT("quit"));};
#define CHECK_TROUBLE(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Next=[&](){TroubleStage++;TroubleClock=0;};
 auto Civilian=[&](){auto* P=GetWorld()->SpawnActor<APiedmontPedestrian>(Bike->GetActorLocation()+FVector(1200,Mode->PeopleHit*140,0),FRotator::ZeroRotator);if(P){P->SetActorTickEnabled(false);P->GetCharacterMovement()->DisableMovement();}return P;};
 auto MountRecovered=[&](){
  Person=Cast<ABattleRider>(GetPawn());if(!Person)return GetPawn()==Bike&&!Bike->bCrashActive;
  if(IsValid(Bike->PlayerCrash)&&IsValid(Bike->PlayerCrash->Fallen)&&FVector::Dist(Person->GetActorLocation(),Bike->PlayerCrash->Fallen->GetActorLocation())>190){Person->AddMovementInput((Bike->PlayerCrash->Fallen->GetActorLocation()-Person->GetActorLocation()).GetSafeNormal2D());return false;}
  return Person->MountBike();
 };
 auto Aim=[&](){FVector Eye;FRotator View;GetPlayerViewPoint(Eye,View);if(Officer)SetControlRotation((Officer->GetActorLocation()-Eye).Rotation());};
 if(TroubleStage==0){
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  CHECK_TROUBLE(Mode->PeopleHit==0&&!Mode->bPoliceAlert&&Mode->PoliceSpawned==0,"Police active before incident");
  auto* P=Civilian();CHECK_TROUBLE(P,"Civilian fixture failed");
  FHitResult Impact(P,P->GetCapsuleComponent(),P->GetActorLocation(),-Bike->GetActorForwardVector());Impact.bBlockingHit=true;Bike->Ride->Speed=650;Bike->Ride->HandleImpact(Impact,.016f,Bike->GetActorForwardVector()*10);
  CHECK_TROUBLE(Mode->PeopleHit==1&&!Mode->bPoliceAlert&&!Mode->RecordAssault(P)&&Mode->PeopleHit==1,"Impact routing or repeated-person guard failed");Next();
 }else if(TroubleStage==1&&TroubleClock>2.4f){
  if(!MountRecovered()){if(TroubleClock>18)Finish(false,TEXT("Initial impact recovery failed"));return;}
  CHECK_TROUBLE(Mode->RecordAssault(Civilian())&&!Mode->bPoliceAlert&&Mode->PeopleHit==2,"Second person alerted police early");
  CHECK_TROUBLE(Mode->RecordAssault(Civilian())&&Mode->bPoliceAlert&&Mode->PeopleHit==3,"Third person did not alert police");
  CHECK_TROUBLE(Mode->Enemies&&Mode->Quest,"Directors missing");
  Mode->Enemies->bFreezeSpawns=false;Mode->PoliceDelay=0;Mode->TickTrouble(.01f);Mode->Enemies->bFreezeSpawns=true;
  // APD really answers on the next trouble tick, and the runtime navmesh can
  // still be building this early in a run, so wait for the officer instead of
  // demanding one in the same frame the alert fires.
  TroubleStage=12;TroubleClock=0;return;
 }else if(TroubleStage==12){
  Mode->Enemies->bFreezeSpawns=false;Mode->PoliceDelay=0;Mode->TickTrouble(.03f);Mode->Enemies->bFreezeSpawns=true;
  if(TActorIterator<ABattlePolice> It(GetWorld());It)Officer=*It;
  // 40 s, not 16: the officer comes from GetRandomReachablePointInRadius on the
  // runtime navmesh, and on a machine busy enough to be running something else
  // the build-and-query can miss every 0.03 s retry for a quarter of a minute.
  // This audit failed that way once in the build-136 sweep and passed twice
  // standalone on the same binary, so the wait was measuring the machine.
  if(!Officer){if(TroubleClock>40)Finish(false,TEXT("No police spawned on reachable world navigation"));return;}
  UE_LOG(LogTemp,Display,TEXT("PoliceResponseWait: seconds=%.2f spawned=%d"),TroubleClock,Mode->PoliceSpawned);
  CHECK_TROUBLE(Mode->PoliceSpawned==1,"Police actor appeared without counting a spawn");
  TroubleOfficer=Officer;TroublePoliceStart=Officer->GetActorLocation();Officer->Cooldown=100;TroubleStage=2;TroubleClock=0;return;
 }else if(TroubleStage==2&&TroubleClock>1.6f){
  const float Travel=FVector::Dist2D(Officer->GetActorLocation(),TroublePoliceStart);UE_LOG(LogTemp,Display,TEXT("PolicePursuitDiagnostic: travel=%.1f location=%s start=%s target=%s speed=%.1f movement=%d controller=%s"),Travel,*Officer->GetActorLocation().ToString(),*TroublePoliceStart.ToString(),*GetPawn()->GetActorLocation().ToString(),Officer->GetVelocity().Size(),int32(Officer->GetCharacterMovement()->MovementMode),*GetNameSafe(Officer->GetController()));CHECK_TROUBLE(Travel>120,"Officer failed actual navigation pursuit");UE_LOG(LogTemp,Display,TEXT("PolicePursuit: travelled=%.1f"),Travel);
  Officer->SetActorTickEnabled(false);Officer->GetCharacterMovement()->DisableMovement();if(auto* AI=Cast<AAIController>(Officer->GetController()))AI->StopMovement();
  CHECK_TROUBLE(Bike->Dismount(),"Dismount failed");Person=Cast<ABattleRider>(GetPawn());CHECK_TROUBLE(Person&&Person->ToggleDrawWeapon(),"Could not draw pistol");Officer->SetActorLocation(Person->GetActorLocation()+FVector(300,0,0));Next();
 }else if(TroubleStage==3){Aim();if(TroubleClock>.4f){const float Time=Mode->TimeRemaining,Heat=Mode->Trouble;const bool Fired=Person->Fire();UE_LOG(LogTemp,Display,TEXT("PoliceShotDiagnostic: fired=%d delta=%.3f officer_health=%.1f draw=%.2f shooter=%s officer=%s view=%s"),Fired,Mode->TimeRemaining-Time,Officer->Health,Person->DrawRemaining,*Person->GetActorLocation().ToString(),*Officer->GetActorLocation().ToString(),*GetControlRotation().ToString());CHECK_TROUBLE(Fired&&FMath::IsNearlyEqual(Mode->TimeRemaining-Time,-60.f,.01f)&&Officer->Health<100,"Real police shot penalty failed");CHECK_TROUBLE(Mode->Trouble>Heat,"Gunshot did not attract trouble");
  Mode->Enemies->Tick(0);const int32 Before=Mode->Enemies->DesiredZombies;Mode->Quest->bCollected=true;Mode->Enemies->Tick(0);CHECK_TROUBLE(Mode->Enemies->DesiredZombies==Before+6,"Artifact did not escalate population");Mode->Quest->bCollected=false;
  CHECK_TROUBLE(Person->MountBike(),"Remount failed");Officer->SetActorLocation(Bike->GetActorLocation()+FVector(350,0,0));
  auto* Wall=GetWorld()->SpawnActor<AActor>();auto* Box=NewObject<UStaticMeshComponent>(Wall);Wall->SetRootComponent(Box);Box->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Box->SetWorldScale3D(FVector(.3,4,4));Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);Box->SetCollisionResponseToAllChannels(ECR_Block);Box->RegisterComponent();Wall->SetActorLocation(Bike->GetActorLocation()+FVector(175,0,0));
  CHECK_TROUBLE(!Officer->FireTaser()&&Bike->TaserHits==0,"Taser passed through wall");Wall->Destroy();Next();
 }}else if(TroubleStage==4&&TroubleClock>.2f){
  FString Dir;if(FParse::Value(FCommandLine::Get(),TEXT("BattlePoliceReviewDir="),Dir)){
   const FVector Focus=Officer->GetActorLocation()+FVector(0,0,45),Eye=Officer->GetActorLocation()+(FParse::Param(FCommandLine::Get(),TEXT("BattlePoliceCloseup"))?FVector(-120,-190,90):FVector(-350,-450,150));
   if(auto* ReviewCamera=GetWorld()->SpawnActor<ACameraActor>(Eye,(Focus-Eye).Rotation()))SetViewTarget(ReviewCamera);
   if(FParse::Param(FCommandLine::Get(),TEXT("BattlePoliceCloseup"))&&GetHUD())GetHUD()->bShowHUD=false;
  }
  Officer->Cooldown=0;Officer->SetActorTickEnabled(true);Next();
 }else if(TroubleStage==5&&TroubleClock>.4f){CHECK_TROUBLE(Officer->bWarning&&Officer->WarningRemaining>ABattlePolice::TaserAimLockSeconds&&Officer->TaserDrawBlend>.7f&&Officer->Weapon->GetComponentLocation().Z>Officer->GetActorLocation().Z&&Officer->Weapon->IsVisible()&&Officer->Weapon->GetStaticMesh()->GetName()==TEXT("Taser")&&Officer->AimBeam->IsVisible()&&Bike->TaserHits==0,"Taser windup, visible aim line or reaction window missing");// Put cover behind the muzzle: a muzzle-only ray would incorrectly fire past it.
  CHECK_TROUBLE(Officer->WarningVoice->Sound&&Officer->WarningVoice->Sound->GetDuration()>1.f&&Officer->WarningVoice->Sound->GetDuration()<2.f&&Officer->WarningVoiceStarts==1,"Warning voice missing, repeated or too long");
  if(FParse::Param(FCommandLine::Get(),TEXT("BattlePoliceAudioAudit"))){
   CHECK_TROUBLE(Officer->WarningVoice->IsPlaying(),"Warning voice did not start on audio device");
   UE_LOG(LogTemp,Display,TEXT("PoliceVoiceAudit: playing=1 duration=%.2f starts=%d"),Officer->WarningVoice->Sound->GetDuration(),Officer->WarningVoiceStarts);
  }
  auto* NearWall=GetWorld()->SpawnActor<AActor>();CHECK_TROUBLE(NearWall,"Near-cover fixture failed");
  auto* NearBox=NewObject<UStaticMeshComponent>(NearWall);NearWall->SetRootComponent(NearBox);NearBox->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));NearBox->SetWorldScale3D(FVector(.05,.8,1));NearBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);NearBox->SetCollisionResponseToAllChannels(ECR_Block);NearBox->SetCanEverAffectNavigation(false);NearBox->RegisterComponent();
  NearWall->SetActorLocationAndRotation((Officer->GetActorLocation()+FVector(0,0,40)+Officer->TaserMuzzle())*.5f,Officer->GetActorRotation());
  FHitResult Probe;FCollisionQueryParams ProbeQuery(SCENE_QUERY_STAT(TaserNearCoverAudit),false,Officer);GetWorld()->LineTraceSingleByChannel(Probe,Officer->TaserMuzzle(),Bike->GetActorLocation(),ECC_Visibility,ProbeQuery);
  CHECK_TROUBLE(Probe.GetActor()!=NearWall,"Near-cover fixture did not isolate muzzle protrusion");
  CHECK_TROUBLE(!Officer->FireTaser()&&Bike->TaserHits==0,"Protruding taser bypassed nearby cover");NearWall->Destroy();
  TroubleTime=Mode->TimeRemaining;FString Dir;if(FParse::Value(FCommandLine::Get(),TEXT("BattlePoliceReviewDir="),Dir))FScreenshotRequest::RequestScreenshot(Dir/TEXT("police-warning.png"),false,false);Next();}
 else if(TroubleStage==6&&Bike->TaserHits>0){
  FString Dir;if(FParse::Value(FCommandLine::Get(),TEXT("BattlePoliceReviewDir="),Dir))FScreenshotRequest::RequestScreenshot(Dir/TEXT("police-discharge.png"),false,false);
  CHECK_TROUBLE(Officer->LastTaserOrigin.Z>Officer->GetActorLocation().Z&&FVector::Dist(Officer->LastTaserOrigin,Officer->Weapon->GetComponentLocation())<40,"Taser discharge did not originate at raised device");
  Person=Cast<ABattleRider>(GetPawn());CHECK_TROUBLE(Bike->bCrashActive&&Bike->bParked&&Bike->StunRemaining>0&&Bike->Deaths==0&&Bike->RiderHealth==100,"Taser did not knock rider off locally");
  CHECK_TROUBLE(!Bike->FirePistol()&&!Bike->Dismount()&&!Bike->ApplyTaser()&&Bike->TaserHits==1,"Stun actions or repeat-hit guard failed");
  CHECK_TROUBLE(Mode->LastTimeDelta==-10&&Mode->TimeNotice==TEXT("TASED")&&Mode->TimeRemaining<TroubleTime-10,"Taser time penalty missing");Officer->SetActorTickEnabled(false);Next();
 }else if(TroubleStage==7&&TroubleClock>3.3f){
  if(Bike->bCrashActive||!Person){if(TroubleClock>18)Finish(false,TEXT("Taser physical recovery failed"));return;}
  const bool Mounted=MountRecovered();if(!Mounted){if(TroubleClock>18)Finish(false,TEXT("Taser remount approach failed"));return;}
  CHECK_TROUBLE(Bike->StunRemaining==0&&GetPawn()==Bike&&Bike->Ride->IsMovingOnGround()&&Bike->Deaths==0,"Recovery/remount failed");
  CHECK_TROUBLE(!Officer->FireTaser()&&Bike->TaserHits==1,"Taser grace failed");Bike->TaserGrace=0;CHECK_TROUBLE(Officer->FireTaser()&&Bike->ApplyRiderDamage(1000)>0,"Death during stun fixture failed");Next();
 }
 else if(TroubleStage==8&&TroubleClock>2.4f){
  CHECK_TROUBLE(Bike->Deaths==1&&GetPawn()==Bike&&!Bike->bParked&&Bike->StunRemaining==0,"Checkpoint retained stale stun");
  Mode->PeopleHit=0;Mode->AssaultVictims.Empty();Mode->bPoliceAlert=false;Mode->PoliceDelay=0;
  auto* Witness=Civilian();auto* Sleeper=Civilian();CHECK_TROUBLE(Witness&&Sleeper,"Panic fixtures missing");Sleeper->bAmbientSleeper=true;
  Mode->RecordGunfire();CHECK_TROUBLE(Witness->PanicRemaining==18&&!Mode->bPoliceAlert,"Gunfire did not panic civilians without summoning police");
  Mode->RecordPlayerShotHit(Sleeper);CHECK_TROUBLE(!Mode->bPoliceAlert&&Mode->PeopleHit==0,"Shooting a sleeper summoned police");
  Mode->RecordPlayerShotHit(Witness);CHECK_TROUBLE(Mode->bPoliceAlert&&Mode->PeopleHit==1&&Mode->PoliceDelay==15,"Civilian shooting did not start delayed police response");
  auto* DodgeOfficer=GetWorld()->SpawnActor<ABattlePolice>(Bike->GetActorLocation()-Bike->GetActorForwardVector()*350,GetControlRotation());
  auto* WingOfficer=GetWorld()->SpawnActor<ABattlePolice>(Bike->GetActorLocation()-Bike->GetActorForwardVector()*420+Bike->GetActorRightVector()*80,GetControlRotation());
  CHECK_TROUBLE(DodgeOfficer&&WingOfficer,"Dodge squad fixture failed");const int32 HitsBeforeDodge=Bike->TaserHits;
  DodgeOfficer->bWarning=true;DodgeOfficer->WarningRemaining=ABattlePolice::TaserAimLockSeconds;DodgeOfficer->WarningAimPoint=Bike->GetActorLocation()+Bike->GetActorRightVector()*60;WingOfficer->Cooldown=0;WingOfficer->Tick(.01f);
  CHECK_TROUBLE(!WingOfficer->bWarning,"Two officers aimed tasers at once");
  CHECK_TROUBLE(!DodgeOfficer->FireTaser()&&DodgeOfficer->TaserShots==1&&Bike->TaserHits==HitsBeforeDodge,"Committed taser aim still snapped onto dodging rider");
  CHECK_TROUBLE(WingOfficer->Cooldown>=ABattlePolice::SquadRecoverySeconds-.1f&&!WingOfficer->bWarning,"Squad taser recovery window missing");DodgeOfficer->Destroy();WingOfficer->Destroy();
  Mode->bPoliceAlert=false;Mode->Trouble=0;Mode->QuietTime=0;Bike->TaserGrace=0;CHECK_TROUBLE(Bike->Dismount(),"Airborne taser dismount failed");
  Person=Cast<ABattleRider>(GetPawn());CHECK_TROUBLE(Person,"Airborne taser rider missing");Person->LaunchCharacter(FVector(0,0,560),false,true);Next();
 }else if(TroubleStage==9&&TroubleClock>.12f){
  Person=Cast<ABattleRider>(GetPawn());CHECK_TROUBLE(Person&&Person->GetCharacterMovement()->IsFalling(),"Airborne taser fixture did not leave ground");TroubleAirStartZ=Person->GetActorLocation().Z;
  CHECK_TROUBLE(Bike->ApplyTaser()&&Person->GetCharacterMovement()->IsFalling()&&Person->GetCharacterMovement()->MovementMode!=MOVE_None,"Taser froze airborne rider");Next();
 }else if(TroubleStage==10){
  Person=Cast<ABattleRider>(GetPawn());CHECK_TROUBLE(Person,"Airborne taser rider vanished");
  if(TroubleClock>.2f&&TroubleAirStartZ>-MAX_flt*.5f){CHECK_TROUBLE(Person->GetCharacterMovement()->MovementMode!=MOVE_None&&FMath::Abs(Person->GetActorLocation().Z-TroubleAirStartZ)>2,"Airborne taser stopped gravity");TroubleAirStartZ=-MAX_flt;}
  if(TroubleClock>3.3f){CHECK_TROUBLE(Bike->StunRemaining==0&&Person->GetCharacterMovement()->MovementMode!=MOVE_None,"Airborne taser did not restore movement");Mode->QuietTime=44;Mode->TickTrouble(2);CHECK_TROUBLE(!Mode->bPoliceAlert&&Mode->Trouble==0,"Police alert did not expire after quiet interval");for(TActorIterator<ABattlePolice> It(GetWorld());It;++It)CHECK_TROUBLE(It->bDead||It->IsActorBeingDestroyed(),"Live police remained after alert expired");Finish(true,TEXT("Readable taser windup, narrow dodge corridor, coordinated recovery, airborne gravity and police expiry pass"));}
 }
 if(TroubleClock>18&&TroubleStage!=99)Finish(false,TEXT("Trouble audit timeout"));
#undef CHECK_TROUBLE
#endif
}
