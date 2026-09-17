#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattlePickup.h"
#include "BattleZombie.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "InputKeyEventArgs.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
void ABattleMacController::TickHornAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5||HornStage==99)return;HornAuditClock+=Dt;
 auto* Rider=Cast<ABattleRider>(GetPawn());auto* Bike=Rider?Rider->ParkedBike.Get():Cast<ABattleBike>(GetPawn());auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!Bike||!Mode)return;
 auto Finish=[&](bool Pass,const TCHAR* Why){UE_LOG(LogTemp,Display,TEXT("BattleHornAudit: {\"passed\":%s,\"stage\":%d,\"reason\":\"%s\",\"uses\":%d,\"sounded\":%d}"),Pass?TEXT("true"):TEXT("false"),HornStage,Why,Bike->HornUses,Bike->HornCount);HornStage=99;ConsoleCommand(TEXT("quit"));};
#define HCHECK(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Next=[&](){HornStage++;HornAuditClock=0;};
 auto Press=[&](){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::H,IE_Pressed,1.f,false,0));InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::H,IE_Released,0.f,false,0));};
 auto Supply=[&](APawn* P){auto* H=GetWorld()->SpawnActor<ABattleHornPickup>(P->GetActorLocation()+P->GetActorForwardVector()*80,FRotator::ZeroRotator);if(H)H->SetActorTickEnabled(false);return H;};
 if(HornStage==0){
  int Count=0;for(TActorIterator<ABattleHornPickup> It(GetWorld());It;++It)Count++;HCHECK(Count==6&&Bike->HornUses==2&&Bike->HornCount==0,"Missing six pickups or two starting uses");
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);if(Mode->Enemies)Mode->Enemies->bFreezeSpawns=true;
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)It->Destroy();
  Bike->SetActorLocationAndRotation(FVector(41600,74168,580),FRotator(0,180,0),false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->Ride->Recovery=0;Bike->Ride->bForceNextFloorCheck=true;Next();
 }else if(HornStage==1&&HornAuditClock>.8f){Press();HornPresses++;Next();}
 else if(HornStage==2&&HornAuditClock>.1f){
  UE_LOG(LogTemp,Display,TEXT("HornPress: presses=%d uses=%d sounded=%d recovery=%.2f health=%.1f stun=%.1f parked=%d"),HornPresses,Bike->HornUses,Bike->HornCount,Bike->Ride->Recovery,Bike->RiderHealth,Bike->StunRemaining,Bike->bParked);
  HCHECK(Bike->HornUses==2-HornPresses&&Bike->HornCount==HornPresses,"H did not spend exactly one use");Bike->Horn();HCHECK(Bike->HornUses==2-HornPresses&&Bike->HornCount==HornPresses,"Rapid repeat bypassed cooldown");
  if(Bike->HornUses>0){HornStage=1;HornAuditClock=0;}else Next();
 }else if(HornStage==3&&HornAuditClock>.8f){Press();Next();}
 else if(HornStage==4&&HornAuditClock>.1f){
  HCHECK(Bike->HornCount==2&&Bike->HornUses==0&&Bike->HornNoticeRemaining>0,"Empty horn sounded or lacked feedback");
  auto* A=Supply(Bike);HCHECK(A&&A->TryCollect(Bike)&&Bike->HornUses==3&&!A->TryCollect(Bike),"Horn pickup did not restore three uses once");
  auto* B=Supply(Bike);HCHECK(B&&B->TryCollect(Bike)&&Bike->HornUses==5,"Horn capacity exceeded five");
  auto* C=Supply(Bike);HCHECK(C&&!C->TryCollect(Bike)&&!C->bConsumed,"Full horn consumed pickup");C->Destroy();Next();
 }else if(HornStage==5&&HornAuditClock>.8f){Press();Next();}
 else if(HornStage==6&&HornAuditClock>.1f){HCHECK(Bike->HornUses==4&&Bike->Dismount(),"Cannot spend horn/dismount");Next();}
 else if(HornStage==7&&HornAuditClock>.8f){
  HCHECK(Rider&&Bike->HornUses==4,"Dismount changed horn inventory");Bike->Horn();HCHECK(Bike->HornUses==4,"Parked bike sounded horn");
  auto* H=Supply(Rider);HCHECK(H&&H->TryCollect(Rider)&&Bike->HornUses==5,"On-foot refill failed");HCHECK(Rider->MountBike()&&Bike->HornUses==5,"Remount lost horn inventory");Next();
 }else if(HornStage==8&&HornAuditClock>.8f){
  const int Sounds=Bike->HornCount;Bike->RiderHealth=0;Bike->Horn();HCHECK(Bike->HornCount==Sounds&&Bike->AddHornUses(3)==0,"Dead rider used horn");Bike->RiderHealth=100;
  Mode->StartCountdown=1;Bike->Horn();HCHECK(Bike->HornCount==Sounds,"Countdown used horn");Mode->StartCountdown=0;
  SetPause(true);Bike->Horn();HCHECK(Bike->HornCount==Sounds,"Paused game used horn");SetPause(false);
  Mode->bRunEnded=true;Bike->Horn();HCHECK(Bike->HornCount==Sounds&&Bike->AddHornUses(3)==0,"Ended run used horn");Mode->bRunEnded=false;
  Finish(true,TEXT("Two starting charges, six world pickups, cooldown, empty feedback, +3 refill/cap five, full pickup retention, foot refill/remount and invalid-state guards pass"));
 }
 if(HornAuditClock>8&&HornStage!=99)Finish(false,TEXT("Horn phase timed out"));
#undef HCHECK
#endif
}
