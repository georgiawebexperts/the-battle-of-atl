#include "BattleMacController.h"
#include "BattleBike.h"
#include "PiedmontPedestrian.h"
#include "BattleRider.h"
#include "BattlePickup.h"
#include "PiedmontTrafficDirector.h"
#include "BattleZombie.h"
#include "EngineUtils.h"
#include "Engine/OverlapResult.h"
#include "Components/StaticMeshComponent.h"
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
 // Dismount refuses on four guards, and separately when none of the exit
 // directions has a clear spot. "Dismount failed" and "Reload interruption
 // setup failed" both hid that, and the audit failed at both stages on
 // different runs. Re-run the same read-only checks and name the cause.
 auto DismountWhy=[&]()->FString{
  int32 Grounded=0,WallBlocked=0,Occupied=0;
  FCollisionQueryParams DQ(SCENE_QUERY_STAT(AmmoDismountDiag),false,Bike);
  for(const FVector Direction:{Bike->GetActorRightVector(),-Bike->GetActorRightVector(),-Bike->GetActorForwardVector(),Bike->GetActorForwardVector()}){
   const FVector Candidate=Bike->GetActorLocation()+Direction*145;FHitResult Ground;
   if(!GetWorld()->LineTraceSingleByChannel(Ground,Candidate+FVector(0,0,100),Candidate-FVector(0,0,220),ECC_Visibility,DQ)||Ground.ImpactNormal.Z<.65f)continue;
   ++Grounded;
   const float Clearance=88.f+30.f*(1.f/FMath::Max(.65f,Ground.ImpactNormal.Z)-1.f)+2.f;
   const FVector Exit=Ground.ImpactPoint+FVector(0,0,Clearance);
   FHitResult Wall;if(GetWorld()->SweepSingleByChannel(Wall,Bike->GetActorLocation(),Exit,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,88),DQ)){++WallBlocked;continue;}
   if(GetWorld()->OverlapBlockingTestByChannel(Exit,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,88),DQ)){++Occupied;continue;}
  }
  return FString::Printf(TEXT("controller=%d parked=%d health=%.1f recovery=%.1f grounded=%d wall=%d occupied=%d speed=%.1f at=%s"),
   Bike->GetController()?1:0,Bike->bParked?1:0,Bike->RiderHealth,Bike->Ride?Bike->Ride->Recovery:-1.f,Grounded,WallBlocked,Occupied,
   Bike->GetVelocity().Size(),*Bike->GetActorLocation().ToCompactString());
 };
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
   for(auto Weak:Bins){
    AActor* Bin=Weak.Get();auto* Mesh=Bin->FindComponentByClass<UStaticMeshComponent>();CHECK_AMMO(Mesh,"Bin mesh missing");
    const FBox Bounds=Mesh->Bounds.GetBox();const FVector Base(Bounds.GetCenter().X,Bounds.GetCenter().Y,Bounds.Min.Z);
    FHitResult Ground;FCollisionQueryParams Query(SCENE_QUERY_STAT(BinGroundAudit),true,Bin);Query.AddIgnoredActor(Bike);
    const bool Hit=GetWorld()->LineTraceSingleByChannel(Ground,Base+FVector(0,0,10),Base-FVector(0,0,300),ECC_Visibility,Query);
    UE_LOG(LogTemp,Display,TEXT("BinGroundAudit: bin=%s base=%s gap_cm=%.3f floor=%s component=%s"),*Bin->GetName(),*Base.ToString(),Hit?Base.Z-Ground.ImpactPoint.Z:999.f,*GetNameSafe(Ground.GetActor()),*GetNameSafe(Ground.GetComponent()));
    CHECK_AMMO(Hit&&Base.Z-Ground.ImpactPoint.Z>=-6.f&&Base.Z-Ground.ImpactPoint.Z<=3.f,"Visible ammo bin is not seated on ground");
   }
   const FVector Focus=(Supplies[0]->GetActorLocation()+Bins[0]->GetActorLocation())*.5f;
   const FVector Eye=Focus+FVector(280,-360,160);
   Bike->SetActorLocation(Bike->GetActorLocation()+FVector(0,0,1000));Bike->GetCharacterMovement()->DisableMovement();
   if(auto* Camera=GetWorld()->SpawnActor<ACameraActor>(Eye,(Focus-Eye).Rotation()))SetViewTarget(Camera);
   Next();
  }else if(AmmoStage==1&&AmmoClock>2){
   FString Dir;if(FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Dir))FScreenshotRequest::RequestScreenshot(Dir/TEXT("ammo-bin.png"),false,false);
   Next();
  }else if(AmmoStage==2&&AmmoClock>1){
   for(auto Weak:Supplies){
    auto* Crate=Weak.Get();auto* Mesh=Crate->FindComponentByClass<UStaticMeshComponent>();CHECK_AMMO(Mesh,"Visible ammo case missing");
    const FBox Bounds=Mesh->Bounds.GetBox();const FVector Base(Bounds.GetCenter().X,Bounds.GetCenter().Y,Bounds.Min.Z);
    FHitResult Ground;FCollisionQueryParams Query(SCENE_QUERY_STAT(AmmoCaseGroundAudit),true,Crate);Query.AddIgnoredActor(Bike);
    const bool Hit=GetWorld()->LineTraceSingleByChannel(Ground,Base+FVector(0,0,10),Base-FVector(0,0,180),ECC_Visibility,Query);
    UE_LOG(LogTemp,Display,TEXT("AmmoCaseGroundAudit: gap_cm=%.3f"),Hit?Base.Z-Ground.ImpactPoint.Z:999.f);
    CHECK_AMMO(Hit&&FMath::Abs(Base.Z-Ground.ImpactPoint.Z)<1.f,"Visible ammo case floats above its pickup surface");
   }
   TSet<AActor*> Matched;
   for(auto Weak:Supplies){
    auto* Crate=Weak.Get();CHECK_AMMO(Crate,"Map ammo disappeared before collection");AActor* Closest=nullptr;float Distance=MAX_flt;
    for(auto B:Bins)if(B.IsValid()){const float D=FVector::Dist2D(B->GetActorLocation(),Crate->GetActorLocation());if(D<Distance){Distance=D;Closest=B.Get();}}
    CHECK_AMMO(Closest&&Distance>=100&&Distance<=120&&!Matched.Contains(Closest),"Ammo not paired with a unique nearby bin");Matched.Add(Closest);
    const FVector Approach=Crate->GetActorLocation()+FVector(0,0,33);FCollisionQueryParams Q(SCENE_QUERY_STAT(AmmoBinApproach),false,Bike);
    TArray<FOverlapResult> Blocks;GetWorld()->OverlapMultiByChannel(Blocks,Approach,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,96),Q);
    for(const auto& B:Blocks)if(B.bBlockingHit)UE_LOG(LogTemp,Warning,TEXT("BattleAmmoBinBlocked: crate=%s pos=%s actor=%s component=%s"),*Crate->GetName(),*Approach.ToString(),*GetNameSafe(B.GetActor()),*GetNameSafe(B.GetComponent()));
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
 else if(AmmoStage==3&&AmmoClock>2){
  CHECK_AMMO(Bike->PistolAmmo==17&&Bike->Inventory[0].Reserve==0,"Bike reload failed conservation");
  if(!Bike->Dismount()){
   const FString Why=FString::Printf(TEXT("Dismount failed: %s"),*DismountWhy());
   Finish(false,*Why);return;
  }
  Person=Cast<ABattleRider>(GetPawn());CHECK_AMMO(Person&&Person->Ammo==17,"Possession lost ammunition");CHECK_AMMO(Person->ToggleDrawWeapon(),"Could not draw pistol after dismount");Person->Reload();CHECK_AMMO(Person->ReloadRemaining==0,"Foot reload accepted empty reserve");CHECK_AMMO(Collect(Person)&&Bike->Inventory[0].Reserve==17,"Foot pickup failed");CHECK_AMMO(Collect(Person)&&Bike->Inventory[0].Reserve==34,"Second pickup did not accumulate beyond seventeen");Next();
 }
 else if(AmmoStage==4&&AmmoClock>.4f){CHECK_AMMO(Person->Fire()&&Person->Ammo==16,"Foot shot failed");Person->Reload();CHECK_AMMO(Person->ReloadRemaining>0,"Reload did not start");Next();}
 else if(AmmoStage==5&&AmmoClock>2){CHECK_AMMO(Person->Ammo==17&&Bike->Inventory[0].Reserve==33,"Foot reload created or lost rounds");CHECK_AMMO(Person->MountBike()&&Bike->PistolAmmo==17,"Remount lost rounds");CHECK_AMMO(Bike->GiveWeapon(0,100)&&Bike->Inventory[0].Reserve==102&&!Bike->GiveWeapon(0,10),"Reserve cap failed");AmmoShots=0;Next();}
 else if(AmmoStage==6&&AmmoClock>.3f){const bool Fired=Bike->FirePistol();if(!Fired)UE_LOG(LogTemp,Warning,TEXT("AmmoAudit shot blocked: ammo=%d reserve=%d health=%.1f parked=%d recovery=%.2f"),Bike->PistolAmmo,Bike->Inventory[0].Reserve,Bike->RiderHealth,Bike->bParked,Bike->Ride->Recovery);CHECK_AMMO(Fired,"Second magazine shot failed");AmmoShots++;AmmoClock=0;if(AmmoShots==17)Next();}
 else if(AmmoStage==7&&AmmoClock>.4f){
  const bool bDry=!Bike->FirePistol();
  if(!bDry||!Bike->Dismount()){
   const FString Why=FString::Printf(TEXT("Reload interruption setup failed: dry=%d %s"),bDry?1:0,*DismountWhy());
   Finish(false,*Why);return;
  }
  Next();
 }
 else if(AmmoStage==8&&AmmoClock>2){CHECK_AMMO(Person&&Person->Ammo==0&&Bike->Inventory[0].Reserve==102&&Bike->PistolAmmo==0,"Parked bike consumed rounds during foot possession");CHECK_AMMO(Person->ToggleDrawWeapon(),"Could not draw pistol for interrupted reload recovery");Person->DrawRemaining=0;Person->Reload();Next();}
 else if(AmmoStage==9&&AmmoClock>2){CHECK_AMMO(Person->Ammo==17&&Bike->Inventory[0].Reserve==85&&Person->MountBike()&&Bike->PistolAmmo==17,"Interrupted reload recovery lost rounds");Finish(true,TEXT("Seventeen initial shots, dry fire, pickups, bike/foot reload conservation, possession, interrupted reload and reserve cap pass"));}
 if(AmmoClock>10&&AmmoStage!=99)Finish(false,TEXT("Ammo audit timeout"));
#undef CHECK_AMMO
#endif
}
