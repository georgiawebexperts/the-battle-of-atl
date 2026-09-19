#include "BattleKrogCrash.h"
#include "BattleScooterProp.h"
#include "BattleBike.h"
#include "BattleHomeData.h"
#include "BattleCheckpoints.h"
#include "BattleBenchFire.h"
#include "BattleZombie.h"
#include "PiedmontPedestrian.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

namespace{
 const TCHAR* KrogShouts[]={TEXT("HELP!"),TEXT("SOMEONE CALL 911!"),TEXT("HE'S NOT MOVING!"),TEXT("PLEASE, HELP US!")};
}

ABattleKrogCrash::ABattleKrogCrash(){
 PrimaryActorTick.bCanEverTick=true;Tags.Add(TEXT("BattleKrogCrash"));
 CrashRoot=CreateDefaultSubobject<USceneComponent>(TEXT("CrashRoot"));RootComponent=CrashRoot;
 // The blocker covers the crown of the road only; the shoulder stays open so
 // the rider threads past instead of being stopped dead.
 RoadBlock=CreateDefaultSubobject<UBoxComponent>(TEXT("WreckBlock"));RoadBlock->SetupAttachment(CrashRoot);
 RoadBlock->SetBoxExtent(FVector(240.f,170.f,95.f));
 RoadBlock->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
 RoadBlock->SetCollisionResponseToAllChannels(ECR_Block);
 RoadBlock->SetCanEverAffectNavigation(false);RoadBlock->SetHiddenInGame(true);
}

void ABattleKrogCrash::SpawnWreck(){
 auto Mesh=[&](const TCHAR* Path,UStaticMesh*& Out){Out=LoadObject<UStaticMesh>(nullptr,Path);};
 UStaticMesh* Cube=nullptr;Mesh(TEXT("/Engine/BasicShapes/Cube.Cube"),Cube);
 auto* Wood=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Furniture/M_BenchWood.M_BenchWood"));
 auto* Metal=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/PiedmontRide/Materials/M_GunMetal.M_GunMetal"));
 // The scooters in the wreck.
 //
 // These used to be BP_ScooterRider, which is an untextured engine cylinder -
 // the same placeholder the moving traffic uses. Elliott rode this stretch and
 // asked where the scooters were, and he was right: there were six of them and
 // all six were a grey tube. They are authored from parts now, and they wear the
 // KrogIncident scooter materials that had been sitting unused in the project.
 const int32 Yaw=(int32)RoadDir.Rotation().Yaw;
 const FVector DownSpots[]={
  WreckSpot+RoadSide*-30.f+RoadDir*60.f,
  WreckSpot+RoadSide*430.f+RoadDir*260.f,
  WreckSpot+RoadSide*-260.f+RoadDir*300.f,
  WreckSpot+RoadSide*180.f+RoadDir*-380.f,
  WreckSpot+RoadSide*520.f+RoadDir*140.f,
  WreckSpot+RoadSide*-420.f+RoadDir*-180.f};
 for(int32 I=0;I<UE_ARRAY_COUNT(DownSpots);++I){
  // Laid over: 82-102 degrees of roll about the deck axis, and the matching lift
  // so the deck, not the wheels, is what touches the pavement. The angle used to
  // sit in Pitch, which stands a scooter on its nose instead of laying it down.
  const FRotator Lay(2.f+(I%3)*4.f,(float)(Yaw+(I%2?38:-31)),92.f+(I%2?-12:8));
  ScooterParts+=BuildBattleScooter(this,CrashRoot,DownSpots[I]+FVector(0,0,12.f),Lay,I,false);
  ++Scooters;
 }
 // One still on its wheels, slewed across the wheel line, as if it was just
 // clipped - the thing the rider is looking for when he asks about scooters.
ScooterParts+=BuildBattleScooter(this,CrashRoot,WreckSpot+RoadSide*-520.f+RoadDir*620.f+FVector(0,0,18.f),FRotator(6.f,(float)(Yaw+124),-7.f),2,false);
 ++Scooters;
 // Debris field so the lane reads as wreckage rather than a parked scooter.
 // A car thrown onto its side is what makes the crash read from a distance, and
 // it has to look like a car: the first version of this was a scaled engine
 // cube in gun metal, which on screen was a two-tonne grey slab with a bonfire
 // under it. This is the same body the road cars use, rolled onto its side.
 if(auto* Hull=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Vehicles/SportsCar/SM_SportsCar.SM_SportsCar"))){
  auto* Car=NewObject<UStaticMeshComponent>(this);
  Car->SetupAttachment(CrashRoot);Car->SetStaticMesh(Hull);
  Car->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  Car->SetCollisionResponseToAllChannels(ECR_Block);
  Car->SetCanEverAffectNavigation(false);
  Car->RegisterComponent();
  Car->SetWorldLocation(WreckSpot-RoadSide*300.f+FVector(0,0,118.f));
  Car->SetWorldRotation(FRotator(0,RoadDir.Rotation().Yaw+14.f,87.f));
  if(auto* Paint=Car->CreateDynamicMaterialInstance(0))Paint->SetVectorParameterValue(TEXT("Paint Tint"),FLinearColor(.72f,.18f,.03f));
  if(auto* GlassMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Vehicles/SportsCar/SM_SportsCar_Glass.SM_SportsCar_Glass"))){
   auto* Glass=NewObject<UStaticMeshComponent>(this);
   Glass->SetupAttachment(Car);Glass->SetStaticMesh(GlassMesh);
   // The glass carries the hull's own template offset, exactly as the road car
   // does it, so the two meshes stay aligned when the wreck is rolled over.
   Glass->SetRelativeLocation(FVector(-12,0,-59));
   Glass->SetCollisionEnabled(ECollisionEnabled::NoCollision);
   Glass->RegisterComponent();
  }
  // Wheels off the car and scattered, which is what a wreck looks like.
  if(auto* WheelMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Vehicles/SportsCar/SM_SportsCar_Wheel.SM_SportsCar_Wheel"))){
   for(int32 I=0;I<3;++I){
    auto* Loose=NewObject<UStaticMeshComponent>(this);
    Loose->SetupAttachment(CrashRoot);Loose->SetStaticMesh(WheelMesh);
    Loose->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Loose->SetCanEverAffectNavigation(false);
    Loose->RegisterComponent();
    Loose->SetWorldLocation(WreckSpot+RoadSide*FMath::Cos(I*2.1f)*430.f+RoadDir*FMath::Sin(I*2.1f)*560.f+FVector(0,0,38.f));
    Loose->SetWorldRotation(FRotator(90.f,I*57.f,0.f));
   }
  }
 }
 if(Cube){
  auto* Cabin=NewObject<UStaticMeshComponent>(this);
  Cabin->SetupAttachment(CrashRoot);Cabin->SetStaticMesh(Cube);Cabin->SetMaterial(0,Wood);
  Cabin->SetCollisionEnabled(ECollisionEnabled::NoCollision);Cabin->SetCanEverAffectNavigation(false);
  Cabin->RegisterComponent();
  Cabin->SetWorldLocation(WreckSpot-RoadSide*690.f+RoadDir*120.f+FVector(0,0,26.f));
  Cabin->SetWorldScale3D(FVector(1.9f,1.1f,.42f));
  Cabin->SetWorldRotation(FRotator(9.f,RoadDir.Rotation().Yaw+38.f,7.f));
 }
 for(int32 I=0;I<7;++I){
  const float A=I*0.9f;
  auto* Chunk=NewObject<UStaticMeshComponent>(this);
  Chunk->SetupAttachment(CrashRoot);Chunk->SetStaticMesh(Cube);
  Chunk->SetMaterial(0,I%2?Wood:Metal);
  Chunk->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  Chunk->RegisterComponent();
  Chunk->SetWorldLocation(WreckSpot+RoadSide*FMath::Cos(A)*330.f+RoadDir*FMath::Sin(A)*420.f+FVector(0,0,12.f+(I%3)*9.f));
  Chunk->SetWorldScale3D(FVector(0.7f+(I%3)*0.35f,0.45f,0.22f));
  Chunk->SetWorldRotation(FRotator(I*27.f,RoadDir.Rotation().Yaw+I*41.f,I*13.f));
 }
 // Fires burn for the whole run.
 for(int32 I=0;I<5;++I){
  const FVector Spot=WreckSpot+RoadSide*(I-1)*260.f+RoadDir*(I==1?180.f:-120.f);
  if(auto* Fire=GetWorld()->SpawnActor<ABattleBenchFire>(Spot+FVector(0,0,40.f),FRotator::ZeroRotator)){
   Fire->Duration=100000.f;Fire->SetLifeSpan(0.f);Fires++;
  }
 }
}

void ABattleKrogCrash::BeginPlay(){
 Super::BeginPlay();
 const FVector Entry=BattleHomeData::TunnelEntry;
 const auto& Market=BattleCheckpoints::Anchors[1];
 const FVector MarketXY((float)Market.X,(float)Market.Y,Entry.Z);
 RoadDir=(Entry-MarketXY).GetSafeNormal2D();
 RoadSide=FVector::CrossProduct(FVector::UpVector,RoadDir).GetSafeNormal();
 const FVector Try=Entry-RoadDir*2400.f;
 FHitResult Ground;FCollisionQueryParams Q;Q.bIgnoreTouches=true;
 WreckSpot=GetWorld()->LineTraceSingleByChannel(Ground,Try+FVector(0,0,1500),Try-FVector(0,0,2500),ECC_Visibility,Q)?Ground.ImpactPoint:Try;
 Approach=WreckSpot-RoadDir*2200.f;
 DistanceToTunnelCm=FVector::Dist2D(WreckSpot,Entry);
 SetActorLocation(WreckSpot);
 RoadBlock->SetWorldLocationAndRotation(WreckSpot+RoadSide*110.f+FVector(0,0,95.f),RoadDir.Rotation());
 // How much room is left on the open shoulder.
 FHitResult Gap;FCollisionQueryParams GQ;GQ.AddIgnoredActor(this);
 const FVector From=WreckSpot+RoadSide*330.f-RoadDir*900.f+FVector(0,0,70.f);
 const FVector To=WreckSpot+RoadSide*330.f+RoadDir*900.f+FVector(0,0,70.f);
 GapClearanceCm=GetWorld()->LineTraceSingleByChannel(Gap,From,To,ECC_Visibility,GQ)?FVector::Dist(From,Gap.ImpactPoint):FVector::Dist(From,To);
 SpawnWreck();
 TopUp();
 // Keep a shout live even before the rider arrives, so the scene is never mute.
 ShoutText=KrogShouts[0];ShoutRemaining=3.4f;
 UE_LOG(LogTemp,Display,TEXT("BattleKrogCrash: wreck=%s approach=%s tunnel_distance_cm=%.0f gap_cm=%.0f gap_open=%s fires=%d bodies=%d bystanders=%d brawlers=%d scooters=%d scooter_parts=%d"),
  *WreckSpot.ToString(),*Approach.ToString(),DistanceToTunnelCm,GapClearanceCm,GapIsRideable()?TEXT("true"):TEXT("false"),Fires,Bodies,Bystanders,Brawlers,Scooters,ScooterParts);
}

bool ABattleKrogCrash::GapIsRideable() const{
 if(!GetWorld())return false;
 FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(this);
 const FVector From=WreckSpot+RoadSide*330.f-RoadDir*900.f+FVector(0,0,70.f);
 const FVector To=WreckSpot+RoadSide*330.f+RoadDir*900.f+FVector(0,0,70.f);
 return !GetWorld()->LineTraceSingleByChannel(Hit,From,To,ECC_Visibility,Q);
}

void ABattleKrogCrash::TopUp(){
 auto Ground=[&](FVector P){FHitResult H;FCollisionQueryParams Q;Q.AddIgnoredActor(this);
  return GetWorld()->LineTraceSingleByChannel(H,P+FVector(0,0,900),P-FVector(0,0,1500),ECC_Visibility,Q)?H.ImpactPoint:P;};
 auto Ped=[&](FVector Spot,bool bBody){
  auto* P=GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),
   FTransform(FRotator(0,RoadDir.Rotation().Yaw+(bBody?90.f:0.f),0),Spot),this,nullptr,
   ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
  if(!P)return (APiedmontPedestrian*)nullptr;
  P->bAmbientSleeper=bBody;P->AmbientWakeChance=bBody?0.f:.35f;
  P->FinishSpawning(FTransform(FRotator(0,RoadDir.Rotation().Yaw+(bBody?90.f:0.f),0),Spot));
  return P;
 };
 // Bodies stay where they fell; they never wake.
 if(Bodies<3)for(int32 I=Bodies;I<3;++I){
  const FVector Spot=Ground(WreckSpot+RoadSide*(I-1)*230.f+RoadDir*(I==1?240.f:-260.f));
  if(Ped(Spot+FVector(0,0,40.f),true))++Bodies;
 }
 // Bystanders shout and run.
 while(Crowd.Num()<8){
  const float A=Crowd.Num()*0.79f;
  const FVector Spot=Ground(WreckSpot+RoadSide*FMath::Cos(A)*1150.f+RoadDir*FMath::Sin(A)*1350.f);
  auto* P=Ped(Spot+FVector(0,0,98.f),false);
  if(!P)break;
  P->HearGunfire(WreckSpot);Crowd.Add(P);++Bystanders;
 }
 // Two brawls, refreshed so the fight is always on when the rider arrives.
 if(Brawlers<4){
  TArray<ABattleZombie*> Fresh;
  for(int32 I=Brawlers;I<4;++I){
   const float A=I*1.9f;
   const FVector Spot=Ground(WreckSpot+RoadSide*FMath::Cos(A)*1650.f+RoadDir*FMath::Sin(A)*1750.f);
   if(auto* Z=GetWorld()->SpawnActorDeferred<ABattleZombie>(ABattleZombie::StaticClass(),
     FTransform((WreckSpot-Spot).Rotation(),Spot+FVector(0,0,90.f)),this,nullptr,
     ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn)){
    Z->VisualStyle=1;Z->bMurderKBrawler=true;Z->Emergence=0;
    Z->FinishSpawning(FTransform((WreckSpot-Spot).Rotation(),Spot+FVector(0,0,90.f)));
    Fresh.Add(Z);++Brawlers;
   }
  }
  for(int32 I=0;I+1<Fresh.Num();I+=2){Fresh[I]->BrawlPartner=Fresh[I+1];Fresh[I+1]->BrawlPartner=Fresh[I];}
 }
}

void ABattleKrogCrash::Tick(float Dt){
 Super::Tick(Dt);
 auto* Viewer=UGameplayStatics::GetPlayerPawn(this,0);
 if(!Viewer)return;
 const float Distance=FVector::Dist2D(Viewer->GetActorLocation(),WreckSpot);
 if(Distance>14000.f)return;
 if(Distance<9000.f)if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this)))Mode->PushHint(TEXT("krogcrash"),TEXT("CRASH AHEAD — PEOPLE IN THE ROAD, THREAD THE GAP"),5.f);
 // Keep the crowd panicked and inside the scene: strays are put back on the ring.
 for(int32 I=Crowd.Num()-1;I>=0;--I){
  auto* P=Crowd[I].Get();
  if(!P||P->IsActorBeingDestroyed()){Crowd.RemoveAt(I);Bystanders=FMath::Max(0,Bystanders-1);continue;}
  P->HearGunfire(WreckSpot);
  if(FVector::Dist2D(P->GetActorLocation(),WreckSpot)>5200.f){
   const float A=I*0.79f;
   P->SetActorLocation(WreckSpot+RoadSide*FMath::Cos(A)*1150.f+RoadDir*FMath::Sin(A)*1350.f+FVector(0,0,98.f),false,nullptr,ETeleportType::TeleportPhysics);
  }
 }
 ShoutClock-=Dt;
 if(ShoutClock<=0){
  ShoutClock=3.4f;ShoutIndex=(ShoutIndex+1)%(int32)UE_ARRAY_COUNT(KrogShouts);
  ShoutText=KrogShouts[ShoutIndex];ShoutRemaining=3.4f;
 }
 ShoutRemaining=FMath::Max(0.f,ShoutRemaining-Dt);
 TopUpClock-=Dt;
 if(TopUpClock<=0){TopUpClock=6.f;TopUp();}
}
