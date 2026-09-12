#include "BattleRoadCrossing.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
ABattleRoadCrossing::ABattleRoadCrossing(){
 PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=.05f;
 CrossingArea=CreateDefaultSubobject<UBoxComponent>(TEXT("CrossingArea"));SetRootComponent(CrossingArea);
 CrossingArea->SetBoxExtent(FVector(150,650,130));CrossingArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
bool ABattleRoadCrossing::CanEnter(const AActor* Vehicle) const {
 if(!IsValid(Vehicle)||Vehicle->IsActorBeingDestroyed()||Vehicle->GetWorld()!=GetWorld())return false;
 if(ReservedVehicle.IsValid())return ReservedVehicle.Get()==Vehicle;
 if(!bVehicleGreen)return false;
 FCollisionObjectQueryParams Objects;Objects.AddObjectTypesToQuery(ECC_Pawn);Objects.AddObjectTypesToQuery(ECC_WorldDynamic);
 FCollisionQueryParams Q(SCENE_QUERY_STAT(RoadCrossingOccupants),false,this);Q.AddIgnoredActor(Vehicle);
 TArray<FOverlapResult> Occupants;
 GetWorld()->OverlapMultiByObjectType(Occupants,CrossingArea->GetComponentLocation(),CrossingArea->GetComponentQuat(),Objects,FCollisionShape::MakeBox(CrossingArea->GetScaledBoxExtent()),Q);
 for(const auto& O:Occupants)if(IsValid(O.GetActor())&&O.GetComponent()&&O.GetComponent()->GetCollisionEnabled()!=ECollisionEnabled::NoCollision)return false;
 return true;
}

bool ABattleRoadCrossing::TryReserve(AActor* Vehicle){
 if(!CanEnter(Vehicle))return false;
 if(ReservedVehicle.Get()!=Vehicle){ReservedVehicle=Vehicle;bOwnerEntered=false;}
 return true;
}
void ABattleRoadCrossing::ReleaseVehicle(const AActor* Vehicle){
 if(ReservedVehicle.Get()==Vehicle){ReservedVehicle.Reset();bOwnerEntered=false;}
}
void ABattleRoadCrossing::Tick(float Dt){
 Super::Tick(Dt);
 auto* Owner=ReservedVehicle.Get();
 if(!IsValid(Owner)||Owner->IsActorBeingDestroyed()){ReservedVehicle.Reset();bOwnerEntered=false;return;}
 const bool InCrossing=CrossingArea->Bounds.GetBox().Intersect(Owner->GetComponentsBoundingBox());
 if(InCrossing)bOwnerEntered=true;
 else if(bOwnerEntered)ReleaseVehicle(Owner);
}
