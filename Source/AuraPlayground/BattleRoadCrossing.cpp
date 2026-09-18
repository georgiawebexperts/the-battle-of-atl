#include "BattleRoadCrossing.h"
#include "BattleRoadCar.h"
#include "PiedmontPedestrian.h"
#include "GameFramework/Pawn.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
ABattleRoadCrossing::ABattleRoadCrossing(){
 PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=.05f;
 CrossingArea=CreateDefaultSubobject<UBoxComponent>(TEXT("CrossingArea"));SetRootComponent(CrossingArea);
 CrossingArea->SetBoxExtent(FVector(150,650,130));CrossingArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
bool ABattleRoadCrossing::CanEnter(const AActor* Vehicle,bool AllowAmber) const {
 if(!IsValid(Vehicle)||Vehicle->IsActorBeingDestroyed()||Vehicle->GetWorld()!=GetWorld())return false;
 if(ReservedVehicle.IsValid())return ReservedVehicle.Get()==Vehicle;
 if(!bVehicleGreen&&!(AllowAmber&&bVehicleAmber))return false;
 FCollisionObjectQueryParams Objects;Objects.AddObjectTypesToQuery(ECC_Pawn);Objects.AddObjectTypesToQuery(ECC_WorldDynamic);
 FCollisionQueryParams Q(SCENE_QUERY_STAT(RoadCrossingOccupants),false,this);Q.AddIgnoredActor(Vehicle);
 TArray<FOverlapResult> Occupants;
 GetWorld()->OverlapMultiByObjectType(Occupants,CrossingArea->GetComponentLocation(),CrossingArea->GetComponentQuat(),Objects,FCollisionShape::MakeBox(CrossingArea->GetScaledBoxExtent()),Q);
 // Vehicles wait for vehicles, not for people. Counting the crowd as an
 // occupant looked right and behaved terribly: walkers cross here constantly,
 // so every car read the box as blocked, none of them ever entered, and the
 // intersection silted up with stationary traffic. Walkers are held at the
 // kerb while the light is green instead, below.
 for(const auto& O:Occupants){
  auto* Other=O.GetActor();
  if(!IsValid(Other)||!O.GetComponent()||O.GetComponent()->GetCollisionEnabled()==ECollisionEnabled::NoCollision)continue;
  if(Cast<ABattleRoadCar>(Other))return false;
  if(const APawn* P=Cast<APawn>(Other))if(P->IsPlayerControlled())return false;
 }
 return true;
}

bool ABattleRoadCrossing::TryReserve(AActor* Vehicle,bool AllowAmber){
 if(!CanEnter(Vehicle,AllowAmber))return false;
 if(ReservedVehicle.Get()!=Vehicle){ReservedVehicle=Vehicle;bOwnerEntered=false;}
 return true;
}
void ABattleRoadCrossing::ReleaseVehicle(const AActor* Vehicle){
 if(ReservedVehicle.Get()==Vehicle){ReservedVehicle.Reset();bOwnerEntered=false;}
}
void ABattleRoadCrossing::Tick(float Dt){
 Super::Tick(Dt);
 if(bAutoCycle){
  const float Green=FMath::Max(1.f,GreenSeconds),Amber=FMath::Max(1.f,AmberSeconds),Red=FMath::Max(1.f,RedSeconds);
  CycleClock=FMath::Fmod(CycleClock+FMath::Max(0.f,Dt),Green+Amber+Red);
  bVehicleGreen=CycleClock<Green;bVehicleAmber=!bVehicleGreen&&CycleClock<Green+Amber;
 }
 // Hold the crowd at the kerb while the cars have the light. Without this the
 // walkers wander into the carriageway the moment the light changes and the
 // traffic has to thread through them.
 if(bVehicleGreen||bVehicleAmber){
  FCollisionObjectQueryParams Objects;Objects.AddObjectTypesToQuery(ECC_Pawn);
  FCollisionQueryParams Q(SCENE_QUERY_STAT(RoadCrossingKerb),false,this);
  const FVector Kerb=FVector(CrossingArea->GetScaledBoxExtent().X+260.f,CrossingArea->GetScaledBoxExtent().Y+260.f,CrossingArea->GetScaledBoxExtent().Z);
  TArray<FOverlapResult> Nearby;
  if(GetWorld()->OverlapMultiByObjectType(Nearby,CrossingArea->GetComponentLocation(),CrossingArea->GetComponentQuat(),Objects,FCollisionShape::MakeBox(Kerb),Q)){
   for(const auto& O:Nearby){
    auto* P=Cast<APiedmontPedestrian>(O.GetActor());
    if(!P||P->bDead)continue;
    // Anyone already in the carriageway walks out; only the kerbside waits.
    if(CrossingArea->Bounds.GetBox().IsInside(P->GetActorLocation()))continue;
    P->PauseRemaining=FMath::Max(P->PauseRemaining,.25f);
   }
  }
 }
 auto* Owner=ReservedVehicle.Get();
 if(!IsValid(Owner)||Owner->IsActorBeingDestroyed()){ReservedVehicle.Reset();bOwnerEntered=false;return;}
 const bool InCrossing=CrossingArea->Bounds.GetBox().Intersect(Owner->GetComponentsBoundingBox());
 if(InCrossing)bOwnerEntered=true;
 else if(bOwnerEntered)ReleaseVehicle(Owner);
}
