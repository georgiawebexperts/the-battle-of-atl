#include "BattleRoadTrafficDirector.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
ABattleRoadTrafficDirector::ABattleRoadTrafficDirector(){PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=1.f;}
bool ABattleRoadTrafficDirector::IsUnobserved(FVector Point) const {
 bool HasObserver=false;
 for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It){auto* PC=It->Get();if(!PC)continue;HasObserver=true;
  FVector Eye;FRotator Facing;PC->GetPlayerViewPoint(Eye,Facing);
  if(FVector::Dist(Eye,Point)<3500||(PC->GetPawn()&&FVector::Dist2D(PC->GetPawn()->GetActorLocation(),Point)<3000))return false;
  if(FVector::DotProduct((Point-Eye).GetSafeNormal(),Facing.Vector())<-.1f)continue;
  // Only static cover can hide a population change. Test the whole car envelope.
  FCollisionObjectQueryParams Objects;Objects.AddObjectTypesToQuery(ECC_WorldStatic);
  FCollisionQueryParams Q(SCENE_QUERY_STAT(RoadTrafficVisibility),true,this);if(PC->GetPawn())Q.AddIgnoredActor(PC->GetPawn());
  for(float X:{-270.f,270.f})for(float Y:{-270.f,270.f})for(float Z:{0.f,170.f}){
   const FVector Target=Point+FVector(X,Y,Z);FHitResult Hit;
   if(!GetWorld()->LineTraceSingleByObjectType(Hit,Eye,Target,Objects,Q)||Hit.Distance>=FVector::Dist(Eye,Target)-30.f)return false;
  }
 }
 return HasObserver;
}
ABattleRoadCar* ABattleRoadTrafficDirector::TrySpawnLane(int32 Index){
 Cars.RemoveAll([](const FCar& C){return !C.Actor.IsValid();});LiveCars=Cars.Num();
 if(!Lanes.IsValidIndex(Index)||Lanes[Index].Points.Num()<2||LiveCars>=FMath::Clamp(MaxCars,0,32))return nullptr;
 int32 Count=0;for(const auto& C:Cars)if(C.Lane==Index)++Count;
 if(Count>=FMath::Max(0,MaxCarsPerLane)||!IsUnobserved(Lanes[Index].Points[0]))return nullptr;
 const FTransform Spawn(FRotator::ZeroRotator,Lanes[Index].Points[0]+FVector(0,0,73.3));
 auto* Car=GetWorld()->SpawnActorDeferred<ABattleRoadCar>(ABattleRoadCar::StaticClass(),Spawn,this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
 if(!Car)return nullptr;
 Car->PaintVariant=TotalSpawned%6;
 Car->SetActorHiddenInGame(true);Car->SetActorEnableCollision(false);Car->Route=Lanes[Index].Points;Car->Crossings=Lanes[Index].Crossings;Car->CruiseSpeed=FMath::Clamp(Lanes[Index].CruiseSpeed,100.f,1000.f);Car->FinishSpawning(Spawn);
 if(!Car->StartRoute()){Car->Destroy();return nullptr;}
 Car->SetActorEnableCollision(true);Car->SetActorHiddenInGame(false);Car->Tags.Add(TEXT("AmbientRoadCar"));Cars.Add({Car,Index});++TotalSpawned;LiveCars=Cars.Num();PeakCars=FMath::Max(PeakCars,LiveCars);return Car;
}
void ABattleRoadTrafficDirector::Tick(float Dt){
 Super::Tick(Dt);if(GetWorld()->GetTimeSeconds()<5)return;
 for(int32 I=Cars.Num()-1;I>=0;--I){auto* Car=Cars[I].Actor.Get();if(!IsValid(Car)){Cars.RemoveAtSwap(I);continue;}
  if(Car->bRouteFinished&&IsUnobserved(Car->GetActorLocation())){Car->Destroy();Cars.RemoveAtSwap(I);++TotalRemoved;}
 }
 LiveCars=Cars.Num();Cooldowns.SetNumZeroed(Lanes.Num());
 for(int32 I=0;I<Lanes.Num();++I){Cooldowns[I]-=Dt;if(Cooldowns[I]>0)continue;Cooldowns[I]=FMath::Max(2.f,SpawnInterval);TrySpawnLane(I);}
}
void ABattleRoadTrafficDirector::EndPlay(const EEndPlayReason::Type Reason){
 for(auto& C:Cars)if(C.Actor.IsValid())C.Actor->Destroy();Cars.Reset();LiveCars=0;Super::EndPlay(Reason);
}
