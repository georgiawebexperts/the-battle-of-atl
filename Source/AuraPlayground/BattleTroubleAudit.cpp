#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattlePlayerCrash.h"
#include "BattleFallenBike.h"
#include "BattlePolice.h"
#include "BattleZombie.h"
#include "BattleQuest.h"
#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "AIController.h"
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
  if(TActorIterator<ABattlePolice> It(GetWorld());It)Officer=*It;
  CHECK_TROUBLE(Officer&&Mode->PoliceSpawned==1,"No police spawned on reachable world navigation");
  TroubleOfficer=Officer;TroublePoliceStart=Officer->GetActorLocation();Officer->Cooldown=100;Next();
 }else if(TroubleStage==2&&TroubleClock>1.6f){
  const float Travel=FVector::Dist2D(Officer->GetActorLocation(),TroublePoliceStart);CHECK_TROUBLE(Travel>120,"Officer failed actual navigation pursuit");UE_LOG(LogTemp,Display,TEXT("PolicePursuit: travelled=%.1f"),Travel);
  Officer->SetActorTickEnabled(false);Officer->GetCharacterMovement()->DisableMovement();if(auto* AI=Cast<AAIController>(Officer->GetController()))AI->StopMovement();
  CHECK_TROUBLE(Bike->Dismount(),"Dismount failed");Person=Cast<ABattleRider>(GetPawn());CHECK_TROUBLE(Person&&Person->ToggleDrawWeapon(),"Could not draw pistol");Officer->SetActorLocation(Person->GetActorLocation()+FVector(300,0,0));Next();
 }else if(TroubleStage==3){Aim();if(TroubleClock>.4f){const float Time=Mode->TimeRemaining,Heat=Mode->Trouble;CHECK_TROUBLE(Person->Fire()&&FMath::IsNearlyEqual(Mode->TimeRemaining-Time,-60.f,.01f)&&Officer->Health<100,"Real police shot penalty failed");CHECK_TROUBLE(Mode->Trouble>Heat,"Gunshot did not attract trouble");
  Mode->Enemies->Tick(0);const int32 Before=Mode->Enemies->DesiredZombies;Mode->Quest->bCollected=true;Mode->Enemies->Tick(0);CHECK_TROUBLE(Mode->Enemies->DesiredZombies==Before+6,"Artifact did not escalate population");Mode->Quest->bCollected=false;
  CHECK_TROUBLE(Person->MountBike(),"Remount failed");Officer->SetActorLocation(Bike->GetActorLocation()+FVector(350,0,0));
  auto* Wall=GetWorld()->SpawnActor<AActor>();auto* Box=NewObject<UStaticMeshComponent>(Wall);Wall->SetRootComponent(Box);Box->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Box->SetWorldScale3D(FVector(.3,4,4));Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);Box->SetCollisionResponseToAllChannels(ECR_Block);Box->RegisterComponent();Wall->SetActorLocation(Bike->GetActorLocation()+FVector(175,0,0));
  CHECK_TROUBLE(!Officer->FireTaser()&&Bike->TaserHits==0,"Taser passed through wall");Wall->Destroy();Next();
 }}else if(TroubleStage==4&&TroubleClock>.2f){
  Officer->Cooldown=0;Officer->SetActorTickEnabled(true);Next();
 }else if(TroubleStage==5&&TroubleClock>.4f){CHECK_TROUBLE(Officer->bWarning&&Bike->TaserHits==0,"Taser windup absent or fired early");TroubleTime=Mode->TimeRemaining;Next();}
 else if(TroubleStage==6&&Bike->TaserHits>0){
  Person=Cast<ABattleRider>(GetPawn());CHECK_TROUBLE(Bike->bCrashActive&&Bike->bParked&&Bike->StunRemaining>0&&Bike->Deaths==0&&Bike->RiderHealth==100,"Taser did not knock rider off locally");
  CHECK_TROUBLE(!Bike->FirePistol()&&!Bike->Dismount()&&!Bike->ApplyTaser()&&Bike->TaserHits==1,"Stun actions or repeat-hit guard failed");
  CHECK_TROUBLE(Mode->LastTimeDelta==-10&&Mode->TimeNotice==TEXT("TASED")&&Mode->TimeRemaining<TroubleTime-10,"Taser time penalty missing");Officer->SetActorTickEnabled(false);Next();
 }else if(TroubleStage==7&&TroubleClock>3.3f){
  if(Bike->bCrashActive||!Person){if(TroubleClock>18)Finish(false,TEXT("Taser physical recovery failed"));return;}
  const bool Mounted=MountRecovered();if(!Mounted){if(TroubleClock>18)Finish(false,TEXT("Taser remount approach failed"));return;}
  CHECK_TROUBLE(Bike->StunRemaining==0&&GetPawn()==Bike&&Bike->Ride->IsMovingOnGround()&&Bike->Deaths==0,"Recovery/remount failed");
  CHECK_TROUBLE(!Officer->FireTaser()&&Bike->TaserHits==1,"Taser grace failed");Bike->TaserGrace=0;CHECK_TROUBLE(Officer->FireTaser()&&Bike->ApplyRiderDamage(1000)>0,"Death during stun fixture failed");Next();
 }
 else if(TroubleStage==8&&TroubleClock>2.4f){CHECK_TROUBLE(Bike->Deaths==1&&GetPawn()==Bike&&!Bike->bParked&&Bike->StunRemaining==0,"Checkpoint retained stale stun");Finish(true,TEXT("Impact routing, three people, police spawn/pursuit, real shot penalty, escalation, blocked taser, windup, knockoff, recovery and death during stun pass"));}
 if(TroubleClock>18&&TroubleStage!=99)Finish(false,TEXT("Trouble audit timeout"));
#undef CHECK_TROUBLE
#endif
}
