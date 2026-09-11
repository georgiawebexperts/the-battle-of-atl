#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "PiedmontPathSpline.h"
#include "PiedmontBike.h"
#include "Components/SplineComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Engine/World.h"
APiedmontTrafficDirector::APiedmontTrafficDirector(){PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=.5f;}
void APiedmontTrafficDirector::BeginPlay(){
 Super::BeginPlay();
 for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It)if(!It->bBridge){
  for(int32 I=0;I<It->Centerline->GetNumberOfSplinePoints();I+=8)CandidatePoints.Add(It->Centerline->GetLocationAtSplinePoint(I,ESplineCoordinateSpace::World));
 }
}
APiedmontPedestrian* APiedmontTrafficDirector::SpawnVisitor(FVector Location,bool Jogger){
 auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());FNavLocation Point;
 if(!Nav||!Nav->ProjectPointToNavigation(Location,Point,FVector(180,180,200)))return nullptr;
 FActorSpawnParameters Params;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
 auto* Visitor=GetWorld()->SpawnActor<APiedmontPedestrian>(Point.Location+FVector(0,0,92),FRotator(0,FMath::FRandRange(-180.f,180.f),0),Params);
 if(Visitor){Visitor->Configure(Jogger?EPiedmontPedestrianKind::Jogger:EPiedmontPedestrianKind::Walker);Visitors.Add(Visitor);TotalSpawned++;}
 return Visitor;
}
void APiedmontTrafficDirector::Tick(float Dt){
 Super::Tick(Dt);auto* PC=UGameplayStatics::GetPlayerController(this,0);auto* Mode=Cast<APiedmontRideMode>(UGameplayStatics::GetGameMode(this));
 if(!PC||!PC->GetPawn()||!Mode||Mode->bRunEnded)return;
 const FVector Player=PC->GetPawn()->GetActorLocation();FVector Eye;FRotator View;PC->GetPlayerViewPoint(Eye,View);
 for(int32 I=Visitors.Num()-1;I>=0;I--){
  auto* Visitor=Visitors[I].Get();
  if(!IsValid(Visitor)){Visitors.RemoveAtSwap(I);continue;}
  if(FVector::Dist2D(Player,Visitor->GetActorLocation())>11000&&FVector::DotProduct((Visitor->GetActorLocation()-Eye).GetSafeNormal(),View.Vector())<.3f){Visitor->Destroy();Visitors.RemoveAtSwap(I);}
 }
 LivePopulation=Visitors.Num();if(LivePopulation>=DesiredPopulation||CandidatePoints.IsEmpty())return;
 TArray<FVector> Nearby;
 for(const FVector& P:CandidatePoints){
  const float Distance=FVector::Dist2D(Player,P);if(Distance<900||Distance>7500)continue;
  if(Mode->StartCountdown<=0&&Distance<6000&&FVector::DotProduct((P-Eye).GetSafeNormal(),View.Vector())>.3f)continue;
  Nearby.Add(P);
 }
 for(int32 Attempt=0;Attempt<16&&Visitors.Num()<DesiredPopulation&&!Nearby.IsEmpty();Attempt++){
  const int32 Index=FMath::RandRange(0,Nearby.Num()-1);const FVector P=Nearby[Index];Nearby.RemoveAtSwap(Index);
  bool TooClose=false;for(const auto& Other:Visitors)if(Other.IsValid()&&FVector::Dist2D(P,Other->GetActorLocation())<350){TooClose=true;break;}if(TooClose)continue;
  const bool Jogger=FMath::FRand()<.3f;
  if(auto* Leader=SpawnVisitor(P,Jogger))if(!Jogger&&Visitors.Num()<DesiredPopulation&&FMath::FRand()<.55f){
   if(auto* Friend=SpawnVisitor(Leader->GetActorLocation()+Leader->GetActorRightVector()*105,false)){Friend->GroupLeader=Leader;Friend->GroupSide=1;}
  }
 }
 LivePopulation=Visitors.Num();
}
APiedmontPedestrian* APiedmontTrafficDirector::SpawnVisitorForValidation(FVector Location,bool Jogger){
#if WITH_EDITOR
 if(GetWorld()->WorldType==EWorldType::PIE)return SpawnVisitor(Location,Jogger);
#endif
 return nullptr;
}
