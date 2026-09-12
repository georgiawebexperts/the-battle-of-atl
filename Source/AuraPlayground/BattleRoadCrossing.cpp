#include "BattleRoadCrossing.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
ABattleRoadCrossing::ABattleRoadCrossing(){
 CrossingArea=CreateDefaultSubobject<UBoxComponent>(TEXT("CrossingArea"));SetRootComponent(CrossingArea);
 CrossingArea->SetBoxExtent(FVector(150,650,130));CrossingArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
bool ABattleRoadCrossing::CanEnter(const AActor* Vehicle) const {
 if(!bVehicleGreen||!Vehicle||Vehicle->GetWorld()!=GetWorld())return false;
 FCollisionObjectQueryParams Objects;Objects.AddObjectTypesToQuery(ECC_Pawn);Objects.AddObjectTypesToQuery(ECC_WorldDynamic);
 FCollisionQueryParams Q(SCENE_QUERY_STAT(RoadCrossingOccupants),false,this);Q.AddIgnoredActor(Vehicle);
 TArray<FOverlapResult> Occupants;
 GetWorld()->OverlapMultiByObjectType(Occupants,CrossingArea->GetComponentLocation(),CrossingArea->GetComponentQuat(),Objects,FCollisionShape::MakeBox(CrossingArea->GetScaledBoxExtent()),Q);
 for(const auto& O:Occupants)if(IsValid(O.GetActor())&&O.GetComponent()&&O.GetComponent()->GetCollisionEnabled()!=ECollisionEnabled::NoCollision)return false;
 return true;
}
