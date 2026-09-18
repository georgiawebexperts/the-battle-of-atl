#include "BattleScooterTraffic.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "PiedmontPathSpline.h"
#include "Components/SplineComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ABattleScooterTraffic::ABattleScooterTraffic(){
 PrimaryActorTick.bCanEverTick=true;
 static ConstructorHelpers::FClassFinder<AActor> Scooter(TEXT("/Game/BeltLineGlide/BP_ScooterRider"));
 ScooterClass=Scooter.Class;
}
void ABattleScooterTraffic::BeginPlay(){
 Super::BeginPlay();auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!Mode||!ScooterClass)return;
 TArray<APiedmontPathSpline*> Paths;for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It)if(It->bArtifactEligible&&!It->bBridge&&It->Centerline&&It->Centerline->GetSplineLength()>1800)Paths.Add(*It);
 Paths.Sort([](const APiedmontPathSpline& A,const APiedmontPathSpline& B){return A.OsmWayId<B.OsmWayId;});
 // Scooters are part of the furniture of the whole ride, not a rarity: Elliott
 // asked for them by name after riding the junctions. The difficulty table sets
 // the requested count; this raises the floor so they are always around.
 FRandomStream Random(9602);const int32 Count=FMath::Min(FMath::Max(Mode->Difficulty.Scooters,8),Paths.Num()*2);
 for(int32 I=0;I<Count;I++){
  auto* Path=Paths[I%Paths.Num()];const float Length=Path->Centerline->GetSplineLength();const float Distance=Random.FRandRange(250.f,Length-250.f);
  const bool Reverse=Random.FRand()<Mode->Difficulty.ScooterWrongWayFraction;FVector Location=Path->Centerline->GetLocationAtDistanceAlongSpline(Distance,ESplineCoordinateSpace::World)+FVector(0,0,92);
  FVector Direction=Path->Centerline->GetDirectionAtDistanceAlongSpline(Distance,ESplineCoordinateSpace::World)*(Reverse?-1.f:1.f);
  FActorSpawnParameters Params;Params.Owner=this;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
  if(auto* Rider=GetWorld()->SpawnActor<AActor>(ScooterClass,Location,Direction.Rotation(),Params)){
   Rider->SetActorTickEnabled(false);Rider->Tags.Add(TEXT("BattleScooterRider"));
   const float Speeds[]={Mode->Difficulty.ScooterSlowSpeed,Mode->Difficulty.ScooterMediumSpeed,Mode->Difficulty.ScooterFastSpeed};
   Scooters.Add({Rider,Path,Distance,Speeds[I%3],Reverse});SpawnedScooters++;
  }
 }
 UE_LOG(LogTemp,Display,TEXT("BattleScooters: visible moving riders=%d requested=%d"),SpawnedScooters,Mode->Difficulty.Scooters);
}
void ABattleScooterTraffic::Tick(float Dt){
 Super::Tick(Dt);ContactCooldown=FMath::Max(0.f,ContactCooldown-Dt);auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);auto* Bike=Cast<ABattleBike>(Pawn);if(auto* Foot=Cast<ABattleRider>(Pawn))Bike=Foot->ParkedBike;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!Mode||Mode->bRunEnded)return;
 for(auto& Moving:Scooters){auto* Rider=Moving.Actor.Get();auto* Path=Moving.Path.Get();if(!Rider||!Path||!Path->Centerline)continue;const float Length=Path->Centerline->GetSplineLength();Moving.Distance+=Moving.Speed*Dt*(Moving.Reverse?-1.f:1.f);while(Moving.Distance>Length)Moving.Distance-=Length;while(Moving.Distance<0)Moving.Distance+=Length;
  const FVector Location=Path->Centerline->GetLocationAtDistanceAlongSpline(Moving.Distance,ESplineCoordinateSpace::World)+FVector(0,0,92);FVector Direction=Path->Centerline->GetDirectionAtDistanceAlongSpline(Moving.Distance,ESplineCoordinateSpace::World)*(Moving.Reverse?-1.f:1.f);Rider->SetActorLocationAndRotation(Location,Direction.Rotation(),false,nullptr,ETeleportType::TeleportPhysics);
  if(Pawn&&Bike&&ContactCooldown<=0&&FVector::DistSquared(Pawn->GetActorLocation(),Location)<FMath::Square(125.f)){PlayerContacts++;ContactCooldown=2;Bike->Ride->Wipeout(TEXT("Scooter rider clipped the bike"));}
 }
}
