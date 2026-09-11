#include "BattleMacController.h"
#include "BattleFrisbee.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "PiedmontTrafficDirector.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
void ABattleMacController::TickFrisbeeAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5)return;
 auto* Rider=Cast<ABattleRider>(GetPawn());auto* Bike=Rider?Rider->ParkedBike.Get():Cast<ABattleBike>(GetPawn());auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!Bike||!Mode)return;
 auto* Group=Cast<ABattleFrisbeeGroup>(FrisbeeAuditGroup.Get());FrisbeeClock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BattleFrisbeeAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\",\"groups\":%d,\"throws\":%d,\"catches\":%d,\"misses\":%d,\"retrievals\":%d,\"ammo_given\":%d,\"supply\":%d}"),Pass?TEXT("true"):TEXT("false"),FrisbeePhase,Reason,Mode->Difficulty.FrisbeeGroups,Group?Group->Throws:0,Group?Group->Catches:0,Group?Group->Misses:0,Group?Group->Retrievals:0,Group?Group->AmmoGiven:0,Group?Group->Supply:0);UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);};
#define FRISBEE_CHECK(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 if(FrisbeePhase==0){
  int32 Count=0;for(TActorIterator<ABattleFrisbeeGroup> It(GetWorld());It;++It){Count++;FRISBEE_CHECK(It->bReady&&It->Players.Num()==2,"Incomplete group");for(auto P:It->Players)FRISBEE_CHECK(P->GetCharacterMovement()->IsMovingOnGround(),"Frisbee visitor is not grounded");if(!Group)Group=*It;}
  FRISBEE_CHECK(Count==Mode->Difficulty.FrisbeeGroups&&Group,"Difficulty population mismatch");FrisbeeAuditGroup=Group;
  for(ECollisionChannel Channel:{ECC_Visibility,ECC_Pawn}){FHitResult H;FCollisionQueryParams Q(SCENE_QUERY_STAT(FrisbeeFloorAudit),false,Bike);GetWorld()->LineTraceSingleByChannel(H,Group->GetActorLocation()+FVector(0,0,500),Group->GetActorLocation()-FVector(0,0,500),Channel,Q);UE_LOG(LogTemp,Display,TEXT("FrisbeeFloor: channel=%d actor=%s point=%s pawnResponse=%d"),int(Channel),*GetNameSafe(H.GetActor()),*H.ImpactPoint.ToString(),H.GetComponent()?int(H.GetComponent()->GetCollisionResponseToChannel(ECC_Pawn)):-1);}
  UE_LOG(LogTemp,Display,TEXT("FrisbeeActors: bike=%s playerA=%s playerB=%s"),*Bike->GetActorLocation().ToString(),*Group->Players[0]->GetActorLocation().ToString(),*Group->Players[1]->GetActorLocation().ToString());
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  Bike->SetActorLocation(Group->GetActorLocation()+FVector(0,0,110),false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->bForceNextFloorCheck=true;
  FRISBEE_CHECK(!Bike->Inventory[3].Owned&&!Group->TrySupply(Bike)&&Group->Supply==24,"Supply incorrectly unlocks launcher");
  FRISBEE_CHECK(Bike->GiveWeapon(3,8),"Launcher fixture failed");
  Mode->StartCountdown=1;FRISBEE_CHECK(!Group->TrySupply(Bike),"Countdown allowed supply");Mode->StartCountdown=0;
  UGameplayStatics::SetGamePaused(this,true);const bool PausedPickup=Group->TrySupply(Bike);UGameplayStatics::SetGamePaused(this,false);FRISBEE_CHECK(!PausedPickup,"Pause allowed supply");
  Bike->RiderHealth=0;const bool DeadPickup=Group->TrySupply(Bike);Bike->RiderHealth=100;FRISBEE_CHECK(!DeadPickup,"Dead rider received supply");
  Bike->SetActorLocation(Group->GetActorLocation()+FVector(100,0,110),false,nullptr,ETeleportType::TeleportPhysics);
  auto* Wall=GetWorld()->SpawnActor<AStaticMeshActor>(Group->GetActorLocation()+FVector(50,0,60),FRotator::ZeroRotator);FRISBEE_CHECK(Wall,"Wall fixture failed");auto* Mesh=Wall->GetStaticMeshComponent();Mesh->SetMobility(EComponentMobility::Movable);Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Wall->SetActorScale3D(FVector(.1,2,2));Mesh->SetCollisionProfileName(TEXT("BlockAll"));const bool WallPickup=Group->TrySupply(Bike);Wall->Destroy();FRISBEE_CHECK(!WallPickup&&Group->Supply==24,"Supply passed through wall");
  Bike->SetActorLocation(Group->GetActorLocation()+FVector(0,0,110),false,nullptr,ETeleportType::TeleportPhysics);FrisbeePhase=1;FrisbeeClock=0;
 }else if(FrisbeePhase==1&&FrisbeeClock>.3f){
  FRISBEE_CHECK(Group&&Bike->Inventory[3].Reserve==8&&Group->Supply==16&&Group->AmmoGiven==8,"Automatic bike supply failed");FRISBEE_CHECK(!Group->TrySupply(Bike),"Cooldown did not block repeated supply");Bike->Inventory[3].Reserve=32;FrisbeePhase=2;FrisbeeClock=0;
 }else if(FrisbeePhase==2&&FrisbeeClock>8.2f){
  FRISBEE_CHECK(Group->Supply==16&&!Group->TrySupply(Bike),"Full ammo consumed supply");
  UE_LOG(LogTemp,Display,TEXT("FrisbeeDismount: recovery=%f parked=%d health=%f location=%s"),Bike->Ride->Recovery,Bike->bParked,Bike->RiderHealth,*Bike->GetActorLocation().ToString());
  FRISBEE_CHECK(Bike->Dismount(),"Cannot dismount at group");Rider=Cast<ABattleRider>(GetPawn());FRISBEE_CHECK(Rider,"Missing on-foot rider");
  Rider->SetActorLocation(Group->GetActorLocation()+FVector(0,90,120),false,nullptr,ETeleportType::TeleportPhysics);Bike->Inventory[3].Reserve=30;
  FRISBEE_CHECK(Group->TrySupply(Rider)&&Bike->Inventory[3].Reserve==32&&Group->Supply==14&&Group->AmmoGiven==10,"On-foot supply did not conserve cap-limited ammo");FrisbeePhase=3;FrisbeeClock=0;
 }else if(FrisbeePhase==3){
  FRISBEE_CHECK(Group&&Group->Players.Num()==2&&!Group->Players[0]->bDead&&!Group->Players[1]->bDead,"Group disappeared");
  if(Group->Retrievals>0&&Group->Catches>=3&&Group->Misses>0&&Group->Throws>=5){
   Rider->SetActorLocation(Bike->GetActorLocation()+Bike->GetActorRightVector()*145+FVector(0,0,20),false,nullptr,ETeleportType::TeleportPhysics);FRISBEE_CHECK(Bike->Remount(Rider),"Remount after supply failed");
   FVector Surface;const FVector Direction=Group->FieldDirection.GetSafeNormal2D();const FVector Side=FVector::CrossProduct(FVector::UpVector,Direction);
   FRISBEE_CHECK(ABattleFrisbeeGroup::Ground(GetWorld(),Group->GetActorLocation()+Direction*450+Side*600,Surface,true),"No grass riding start");
   Bike->SetActorLocationAndRotation(Surface+FVector(0,0,130),FRotator(0,Direction.Rotation().Yaw,0),false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->StopMovementImmediately();Bike->Ride->SetMovementMode(MOVE_Walking);Bike->Ride->bForceNextFloorCheck=true;Bike->Ride->Gear=5;
   FrisbeeGrassStart=Bike->GetActorLocation();FrisbeeInitialWipeouts=Bike->Ride->Wipeouts;InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::W,IE_Pressed,1.f,false,0));FrisbeePhase=4;FrisbeeClock=0;
  }
 }
 if(FrisbeePhase==4){
  if(Bike->Ride->IsMovingOnGround()&&Bike->Ride->bGrass)FrisbeeGrassSeconds+=Dt;FrisbeePeakSpeed=FMath::Max(FrisbeePeakSpeed,Bike->Ride->Speed);
  if(FrisbeeClock>5){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::W,IE_Released,0.f,false,0));
   const float Distance=FVector::Dist2D(FrisbeeGrassStart,Bike->GetActorLocation());UE_LOG(LogTemp,Display,TEXT("FrisbeeGrassRide: distance=%.3f groundedGrassSeconds=%.3f peakSpeed=%.3f wipeouts=%d"),Distance,FrisbeeGrassSeconds,FrisbeePeakSpeed,Bike->Ride->Wipeouts-FrisbeeInitialWipeouts);
   FRISBEE_CHECK(Distance>1800&&FrisbeeGrassSeconds>3&&FrisbeePeakSpeed>1000&&Bike->Ride->Wipeouts==FrisbeeInitialWipeouts&&FMath::Abs(Bike->GetActorRotation().Roll)<.1f,"Grass riding failed");Finish(true,TEXT("Groups, complete disc cycle, guarded bike/foot supply, remount and fifth-gear grass ride pass"));return;
  }
 }
 if(FrisbeeClock>40)Finish(false,TEXT("Frisbee audit timed out"));
#undef FRISBEE_CHECK
#endif
}
