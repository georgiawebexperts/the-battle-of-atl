#include "BattleKrogCrash.h"
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
 static ConstructorHelpers::FClassFinder<AActor> Scooter(TEXT("/Game/BeltLineGlide/BP_ScooterRider"));
 ScooterClass=Scooter.Class;
}

void ABattleKrogCrash::SpawnWreck(){
 auto Mesh=[&](const TCHAR* Path,UStaticMesh*& Out){Out=LoadObject<UStaticMesh>(nullptr,Path);};
 UStaticMesh* Cube=nullptr;Mesh(TEXT("/Engine/BasicShapes/Cube.Cube"),Cube);
 auto* Wood=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Furniture/M_BenchWood.M_BenchWood"));
 auto* Metal=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/PiedmontRide/Materials/M_GunMetal.M_GunMetal"));
 // Two scooters: one down in the lane, one thrown onto the shoulder.
 if(ScooterClass){
  const FRotator Down(RoadDir.Rotation().Pitch+78.f,RoadDir.Rotation().Yaw,RoadDir.Rotation().Roll+112.f);
  if(auto* A=GetWorld()->SpawnActor<AActor>(ScooterClass,WreckSpot+RoadSide*40.f,Down))Spawned.Add(A);
  if(auto* B=GetWorld()->SpawnActor<AActor>(ScooterClass,WreckSpot+RoadSide*430.f+RoadDir*260.f,FRotator(0,RoadDir.Rotation().Yaw+58.f,96.f)))Spawned.Add(B);
 }
 // Debris field so the lane reads as wreckage rather than a parked scooter.
 // A car thrown onto its side is what makes the crash read from a distance.
 if(Cube){
  auto* Car=NewObject<UStaticMeshComponent>(this);
  Car->SetupAttachment(CrashRoot);Car->SetStaticMesh(Cube);Car->SetMaterial(0,Metal);
  Car->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  Car->SetCollisionResponseToAllChannels(ECR_Block);
  Car->SetCanEverAffectNavigation(false);
  Car->RegisterComponent();
  Car->SetWorldLocation(WreckSpot-RoadSide*300.f+FVector(0,0,110.f));
  Car->SetWorldScale3D(FVector(4.6f,2.0f,1.25f));
  Car->SetWorldRotation(FRotator(-7.f,RoadDir.Rotation().Yaw+14.f,84.f));
  auto* Cabin=NewObject<UStaticMeshComponent>(this);
  Cabin->SetupAttachment(CrashRoot);Cabin->SetStaticMesh(Cube);Cabin->SetMaterial(0,Wood);
  Cabin->SetCollisionEnabled(ECollisionEnabled::NoCollision);Cabin->SetCanEverAffectNavigation(false);
  Cabin->RegisterComponent();
  Cabin->SetWorldLocation(WreckSpot-RoadSide*300.f+FVector(0,0,210.f));
  Cabin->SetWorldScale3D(FVector(2.4f,1.8f,1.0f));
  Cabin->SetWorldRotation(FRotator(-7.f,RoadDir.Rotation().Yaw+14.f,84.f));
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
 UE_LOG(LogTemp,Display,TEXT("BattleKrogCrash: wreck=%s approach=%s tunnel_distance_cm=%.0f gap_cm=%.0f gap_open=%s fires=%d bodies=%d bystanders=%d brawlers=%d"),
  *WreckSpot.ToString(),*Approach.ToString(),DistanceToTunnelCm,GapClearanceCm,GapIsRideable()?TEXT("true"):TEXT("false"),Fires,Bodies,Bystanders,Brawlers);
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
