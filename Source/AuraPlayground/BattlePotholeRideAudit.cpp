#include "BattlePothole.h"
#include "BattleBike.h"
#include "BattleRoadCar.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
// Opt-in native test: starting teleports only; traversal uses the normal W key.
void TickBattlePotholeRideAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ABattlePothole> Contact;FVector Site;int Phase=0,Wipeouts=0;float Clock=0;bool Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;
 auto Key=[&](bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::W,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Finish=[&](bool Pass,const TCHAR* Why){Key(false);S.Done=true;UE_LOG(LogTemp,Display,TEXT("PotholeRideAudit: {\"passed\":%s,\"reason\":\"%s\",\"phase\":%d,\"contacts\":%d}"),Pass?TEXT("true"):TEXT("false"),Why,S.Phase,S.Contact.IsValid()?S.Contact->Contacts:-1);PC->ConsoleCommand(TEXT("quit"));};
 auto* Bike=Cast<ABattleBike>(PC->GetPawn());if(!Bike){Finish(false,TEXT("Missing mounted bike"));return;}auto* Move=Bike->Ride.Get();
 if(S.Phase==0){
  bool Found=false;for(TActorIterator<ABattlePothole> It(PC->GetWorld());It;++It)if(It->ActorHasTag(TEXT("PotholeReview"))||It->ActorHasTag(TEXT("AuthoredPothole"))){S.Site=It->GetActorLocation();It->SetActorTickEnabled(false);Found=true;break;}
  if(!Found){Finish(false,TEXT("Missing authored candidate"));return;}
  for(TActorIterator<ABattleRoadCar> It(PC->GetWorld());It;++It)It->Destroy();S.Phase=1;
 }
 if(S.Clock==0){
  Key(false);if(S.Contact.IsValid())S.Contact->Destroy();
  FVector Start=S.Site+FVector(-800,S.Phase==1?130:0,0);FHitResult Hit;FCollisionQueryParams Q;Q.bTraceComplex=true;Q.AddIgnoredActor(Bike);
  if(!PC->GetWorld()->LineTraceSingleByChannel(Hit,Start+FVector(0,0,300),Start-FVector(0,0,300),ECC_WorldStatic,Q)){Finish(false,TEXT("Missing starting road"));return;}
  Move->StopMovementImmediately();Move->Speed=0;Move->Recovery=0;Move->Gear=3;Bike->SetActorLocationAndRotation(Hit.ImpactPoint+FVector(0,0,98),FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);Move->SetMovementMode(MOVE_Walking);Move->bForceNextFloorCheck=true;
  S.Wipeouts=Move->Wipeouts;S.Contact=PC->GetWorld()->SpawnActor<ABattlePothole>(S.Site,FRotator::ZeroRotator);S.Clock=.001f;return;
 }
 S.Clock+=Dt;if(S.Clock<.6f)return;Key(true);
 if(Move->Wipeouts!=S.Wipeouts){Finish(false,TEXT("Unexpected shallow wipeout"));return;}
 if(S.Clock>15){Finish(false,TEXT("Traversal timed out"));return;}
 if(Bike->GetActorLocation().X>S.Site.X+350){
  Key(false);const int Expected=S.Phase==1?0:1;
  if(S.Contact->Contacts!=Expected||!Move->IsMovingOnGround()){Finish(false,TEXT("Contact count or grounded endpoint incorrect"));return;}
  if(S.Phase==2){Finish(true,TEXT("Keyboard near miss and shallow crossing passed with one contact and no wipeout"));return;}
  S.Phase=2;S.Clock=0;
 }
#endif
}
