#include "BattleZombie.h"
#include "BattleParkRegion.h"
#include "BattleBike.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformMisc.h"

void TickBattleVendorRegionAudit(ABattleEnemyDirector* Director,float Dt){
#if !UE_BUILD_SHIPPING
 auto* World=Director->GetWorld();if(World->GetTimeSeconds()<5)return;
 static int Stage=0;static float Clock=0;static TWeakObjectPtr<ABattleZombie> Vendor,Punk;static FVector Inside,Outside;
 auto Finish=[&](bool Pass,const TCHAR* Why){UE_LOG(LogTemp,Display,TEXT("VendorRegionAudit: {\"passed\":%s,\"stage\":%d,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Stage,Why);FPlatformMisc::RequestExit(false);};
 auto* Bike=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(World,0));if(!Bike)return;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(World));if(!Mode)return;Mode->bTutorialActive=false;Mode->StartCountdown=0;Bike->Ride->DisableMovement();Bike->DamageGrace=100;
 if(Stage==0){
  bool Found=false;
  for(int I=0;I<UE_ARRAY_COUNT(BattleParkRegion::Outline)&&!Found;I++){
   const FVector2D A=BattleParkRegion::Outline[I],B=BattleParkRegion::Outline[(I+1)%UE_ARRAY_COUNT(BattleParkRegion::Outline)];
   if(FVector2D::Distance(A,B)<400)continue;
   const FVector2D Middle=(A+B)*.5,Edge=(B-A).GetSafeNormal(),Normal(-Edge.Y,Edge.X);
   for(float Sign:{-1.f,1.f}){
    Inside=FVector(Middle+Normal*Sign*70,0);Outside=FVector(Middle-Normal*Sign*70,0);
    if(!BattleParkRegion::Contains(Inside)||BattleParkRegion::Contains(Outside))continue;
    FHitResult Floor;FCollisionQueryParams Q;Q.AddIgnoredActor(Bike);
    if(!World->LineTraceSingleByChannel(Floor,Inside+FVector(0,0,5000),Inside-FVector(0,0,5000),ECC_Visibility,Q)||Floor.ImpactNormal.Z<.8)continue;
    Inside.Z=Outside.Z=Floor.ImpactPoint.Z+90;Found=true;break;
   }
  }
  if(!Found){Finish(false,TEXT("No supported boundary fixture"));return;}
  auto Spawn=[&](FVector P,int Style){auto* Z=World->SpawnActorDeferred<ABattleZombie>(ABattleZombie::StaticClass(),FTransform(P),Director,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);Z->VisualStyle=Style;Z->FinishSpawning(FTransform(P));Z->Emergence=0;return Z;};
  auto* Exterior=Spawn(Outside,0);if(Exterior->VisualStyle!=1){Finish(false,TEXT("Exterior vendor was allowed"));return;}Exterior->Destroy();
  Vendor=Spawn(Inside,0);Punk=Spawn(Outside+FVector(0,0,250),1);
  if(Vendor->VisualStyle!=0||Punk->VisualStyle!=1){Finish(false,TEXT("Variant selection incorrect"));return;}
  // Identical forced native movement: only the vendor must remain inside.
  // Disable collision/ground effects to isolate the geographic movement guard.
  for(auto* Z:{Vendor.Get(),Punk.Get()}){
   Z->SetActorLocation(Inside,false,nullptr,ETeleportType::TeleportPhysics);Z->SetActorEnableCollision(false);
   auto* Movement=Z->GetCharacterMovement();Movement->SetMovementMode(MOVE_Flying);Movement->BrakingDecelerationFlying=0;
   Movement->Velocity=(Outside-Inside).GetSafeNormal()*1000;Movement->TickComponent(.5f,LEVELTICK_All,nullptr);
  }
  const bool Guarded=BattleParkRegion::Contains(Vendor->GetActorLocation());
  const bool Unrestricted=!BattleParkRegion::Contains(Punk->GetActorLocation());
  UE_LOG(LogTemp,Display,TEXT("VendorRegionMovement: vendor_inside=%d punk_outside=%d"),Guarded,Unrestricted);
  if(!Guarded||!Unrestricted){Finish(false,TEXT("Native movement did not distinguish vendor from unrestricted punk"));return;}
  Vendor->SetActorEnableCollision(true);Vendor->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
  Punk->Destroy();Bike->SetActorLocation(Outside,false,nullptr,ETeleportType::TeleportPhysics);
  Vendor->bTelegraphing=true;Vendor->SetActorLocation(Outside,false,nullptr,ETeleportType::TeleportPhysics);Vendor->Tick(.1);
  if(!BattleParkRegion::Contains(Vendor->GetActorLocation())||Vendor->bTelegraphing||Vendor->Attacks){Finish(false,TEXT("Outside correction or attack cancellation failed"));return;}
  Stage=1;
 }
 Clock+=Dt;
 if(Stage==1){
  if(!Vendor.IsValid()||!BattleParkRegion::Contains(Vendor->GetActorLocation())||Vendor->Attacks){Finish(false,TEXT("Vendor escaped or attacked across boundary"));return;}
  // Exercise the character movement delegate with sustained outward velocity.
  Vendor->GetCharacterMovement()->Velocity=(Outside-Inside).GetSafeNormal()*500;
  if(Clock>2){Finish(true,TEXT("Exterior selection, park vendor, outside attack cancellation and two-second outward movement containment pass"));Stage=2;}
 }
#endif
}
