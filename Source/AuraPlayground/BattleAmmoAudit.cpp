#include "BattleMacController.h"
#include "BattleBike.h"
#include "PiedmontPedestrian.h"
#include "BattleRider.h"
#include "BattlePickup.h"
#include "PiedmontTrafficDirector.h"
#include "BattleZombie.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "Engine/GameViewportClient.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
void ABattleMacController::TickAmmoAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5||AmmoStage==99)return;
 auto* Person=Cast<ABattleRider>(GetPawn());auto* Bike=Person?Person->ParkedBike.Get():Cast<ABattleBike>(GetPawn());auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!Bike||!Mode)return;AmmoClock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BattleAmmoAudit: {\"passed\":%s,\"reason\":\"%s\",\"stage\":%d}"),Pass?TEXT("true"):TEXT("false"),Reason,AmmoStage);AmmoStage=99;ConsoleCommand(TEXT("quit"));};
#define CHECK_AMMO(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Next=[&](){AmmoStage++;AmmoClock=0;};
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleAmmoBinAudit"))){
  static TArray<TWeakObjectPtr<ABattleWeaponCrate>> Supplies;static TArray<TWeakObjectPtr<AActor>> Bins;
  if(AmmoStage==0){
   CHECK_AMMO(Mode->Pickups&&Mode->Pickups->AmmoPickups==12,"Incomplete bin ammo layout");
   for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
   for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
   for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)It->Destroy();
   Supplies.Reset();Bins.Reset();
   for(TActorIterator<AActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("BattleAmmoBin")))Bins.Add(*It);
   for(TActorIterator<ABattleWeaponCrate> It(GetWorld());It;++It)if(It->WeaponSlot==0){CHECK_AMMO(It->ActorHasTag(TEXT("AmmoByBin")),"Ammo missing bin association");Supplies.Add(*It);}
   CHECK_AMMO(Supplies.Num()==12&&Bins.Num()==12,"Expected twelve supplies and persistent bins");
   const FVector Focus=(Supplies[0]->GetActorLocation()+Bins[0]->GetActorLocation())*.5f;
   const FVector Eye=Focus+FVector(280,-360,160);
   Bike->SetActorLocation(Bike->GetActorLocation()+FVector(0,0,1000));Bike->GetCharacterMovement()->DisableMovement();
   if(auto* Camera=GetWorld()->SpawnActor<ACameraActor>(Eye,(Focus-Eye).Rotation()))SetViewTarget(Camera);
   Next();
  }else if(AmmoStage==1&&AmmoClock>2){
   FString Dir;if(FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Dir))FScreenshotRequest::RequestScreenshot(Dir/TEXT("ammo-bin.png"),false,false);
   Next();
  }else if(AmmoStage==2&&AmmoClock>1){
   TSet<AActor*> Matched;
   for(auto Weak:Supplies){
    auto* Crate=Weak.Get();CHECK_AMMO(Crate,"Map ammo disappeared before collection");AActor* Closest=nullptr;float Distance=MAX_flt;
    for(auto B:Bins)if(B.IsValid()){const float D=FVector::Dist2D(B->GetActorLocation(),Crate->GetActorLocation());if(D<Distance){Distance=D;Closest=B.Get();}}
    CHECK_AMMO(Closest&&Distance>=100&&Distance<=120&&!Matched.Contains(Closest),"Ammo not paired with a unique nearby bin");Matched.Add(Closest);
    const FVector Approach=Crate->GetActorLocation()+FVector(0,0,33);FCollisionQueryParams Q(SCENE_QUERY_STAT(AmmoBinApproach),false,Bike);
    CHECK_AMMO(!GetWorld()->OverlapBlockingTestByChannel(Approach,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,96),Q),"Bin ammo approach obstructed");
    Bike->SetActorLocation(Approach,false,nullptr,ETeleportType::TeleportPhysics);Bike->Inventory[0].Reserve=0;
    CHECK_AMMO(Crate->TryCollect(Bike)&&Bike->Inventory[0].Reserve==17,"Actual map ammo could not be collected for seventeen rounds");
    CHECK_AMMO(!Crate->TryCollect(Bike)&&IsValid(Closest),"Repeat collection or bin persistence failed");
   }
   UE_LOG(LogTemp,Display,TEXT("BattleAmmoBins: twelve unique pairs, twelve clear approaches, twelve real collections +17, twelve bins retained"));
   Finish(true,TEXT("Twelve actual map ammo pickups each have a reachable persistent bin and grant exactly seventeen rounds"));
  }
  return;
 }
 auto Collect=[&](APawn* Pawn){const FTransform T(Pawn->GetActorLocation()+FVector(70,0,-20));auto* Crate=GetWorld()->SpawnActorDeferred<ABattleWeaponCrate>(ABattleWeaponCrate::StaticClass(),T,this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);if(!Crate)return false;Crate->WeaponSlot=0;Crate->FinishSpawning(T);Crate->SetActorTickEnabled(false);return Crate->TryCollect(Pawn)&&!Crate->TryCollect(Pawn);};
 if(AmmoStage==0){
  CHECK_AMMO(Bike->PistolAmmo==17&&Bike->Inventory[0].Reserve==0,"Wrong starting ammunition");
  CHECK_AMMO(Mode->Pickups&&Mode->Pickups->AmmoPickups==12,"Incomplete ammo layout");
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)It->Destroy();Next();
 }else if(AmmoStage==1&&AmmoClock>.3f){CHECK_AMMO(Bike->FirePistol(),"Bike shot failed");AmmoShots++;AmmoClock=0;if(AmmoShots==17)Next();}
 else if(AmmoStage==2&&AmmoClock>2){CHECK_AMMO(Bike->PistolAmmo==0&&!Bike->FirePistol(),"Empty bike generated rounds");CHECK_AMMO(Collect(Bike)&&Bike->Inventory[0].Reserve==17,"Bike pickup or repeat guard failed");CHECK_AMMO(!Bike->FirePistol(),"Empty bike fired before reload");Next();}
 else if(AmmoStage==3&&AmmoClock>2){CHECK_AMMO(Bike->PistolAmmo==17&&Bike->Inventory[0].Reserve==0,"Bike reload failed conservation");CHECK_AMMO(Bike->Dismount(),"Dismount failed");Person=Cast<ABattleRider>(GetPawn());CHECK_AMMO(Person&&Person->Ammo==17,"Possession lost ammunition");CHECK_AMMO(Person->ToggleDrawWeapon(),"Could not draw pistol after dismount");Person->Reload();CHECK_AMMO(Person->ReloadRemaining==0,"Foot reload accepted empty reserve");CHECK_AMMO(Collect(Person)&&Bike->Inventory[0].Reserve==17,"Foot pickup failed");Next();}
 else if(AmmoStage==4&&AmmoClock>.4f){CHECK_AMMO(Person->Fire()&&Person->Ammo==16,"Foot shot failed");Person->Reload();CHECK_AMMO(Person->ReloadRemaining>0,"Reload did not start");Next();}
 else if(AmmoStage==5&&AmmoClock>2){CHECK_AMMO(Person->Ammo==17&&Bike->Inventory[0].Reserve==16,"Foot reload created or lost rounds");CHECK_AMMO(Person->MountBike()&&Bike->PistolAmmo==17,"Remount lost rounds");CHECK_AMMO(Bike->GiveWeapon(0,100)&&Bike->Inventory[0].Reserve==60&&!Bike->GiveWeapon(0,10),"Reserve cap failed");AmmoShots=0;Next();}
 else if(AmmoStage==6&&AmmoClock>.3f){const bool Fired=Bike->FirePistol();if(!Fired)UE_LOG(LogTemp,Warning,TEXT("AmmoAudit shot blocked: ammo=%d reserve=%d health=%.1f parked=%d recovery=%.2f"),Bike->PistolAmmo,Bike->Inventory[0].Reserve,Bike->RiderHealth,Bike->bParked,Bike->Ride->Recovery);CHECK_AMMO(Fired,"Second magazine shot failed");AmmoShots++;AmmoClock=0;if(AmmoShots==17)Next();}
 else if(AmmoStage==7&&AmmoClock>.4f){CHECK_AMMO(!Bike->FirePistol()&&Bike->Dismount(),"Reload interruption setup failed");Next();}
 else if(AmmoStage==8&&AmmoClock>2){CHECK_AMMO(Person&&Person->Ammo==0&&Bike->Inventory[0].Reserve==60&&Bike->PistolAmmo==0,"Parked bike consumed rounds during foot possession");CHECK_AMMO(Person->ToggleDrawWeapon(),"Could not draw pistol for interrupted reload recovery");Person->DrawRemaining=0;Person->Reload();Next();}
 else if(AmmoStage==9&&AmmoClock>2){CHECK_AMMO(Person->Ammo==17&&Bike->Inventory[0].Reserve==43&&Person->MountBike()&&Bike->PistolAmmo==17,"Interrupted reload recovery lost rounds");Finish(true,TEXT("Seventeen initial shots, dry fire, pickups, bike/foot reload conservation, possession, interrupted reload and reserve cap pass"));}
 if(AmmoClock>10&&AmmoStage!=99)Finish(false,TEXT("Ammo audit timeout"));
#undef CHECK_AMMO
#endif
}
