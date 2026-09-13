#include "BattleSpareBikes.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "PiedmontPathSpline.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SplineComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
namespace BattleSpareBikes {
namespace {
TWeakObjectPtr<UWorld> StationWorld;TArray<TWeakObjectPtr<AActor>> Stations;
AActor* Create(ABattleBike* Source,FTransform Placement){
 auto* Actor=Source->GetWorld()->SpawnActor<AActor>();if(!Actor)return nullptr;
 auto* Root=NewObject<UCapsuleComponent>(Actor,TEXT("SpareBikeCollision"));Actor->AddInstanceComponent(Root);Actor->SetRootComponent(Root);Root->SetCapsuleSize(32,95);Root->SetCollisionProfileName(TEXT("BlockAll"));Root->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);Root->SetCanEverAffectNavigation(false);Root->RegisterComponent();Actor->SetActorTransform(Placement);
 int Index=0;TInlineComponentArray<UStaticMeshComponent*> Parts(Source);
 for(auto* Part:Parts){
  if(Part->IsEditorOnly()||Part->bHiddenInGame||!Part->IsVisible()||!Part->GetStaticMesh()||!Part->IsAttachedTo(Source->Visual)||Part->IsAttachedTo(Source->Rider))continue;
  auto* Copy=NewObject<UStaticMeshComponent>(Actor,*FString::Printf(TEXT("BikePart%d"),Index++));Actor->AddInstanceComponent(Copy);Copy->SetupAttachment(Root);Copy->SetStaticMesh(Part->GetStaticMesh());Copy->SetRelativeTransform(Part->GetComponentTransform().GetRelativeTransform(Source->GetActorTransform()));
  for(int I=0;I<Part->GetNumMaterials();I++)Copy->SetMaterial(I,Part->GetMaterial(I));Copy->SetCollisionEnabled(ECollisionEnabled::NoCollision);Copy->SetCanEverAffectNavigation(false);Copy->RegisterComponent();
 }
 Actor->Tags.Add(TEXT("BattleSpareBike"));Stations.Add(Actor);return Actor;
}
}
void SpawnStations(AActor* Context){
 if(!Context)return;auto* Source=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(Context,0));if(!Source)return;
 StationWorld=Context->GetWorld();Stations.Reset();int Count=0,ParkCount=0,TrailCount=0;
 for(TActorIterator<AActor> It(Context->GetWorld());It&&Count<6;++It){
  if(!It->ActorHasTag(TEXT("BattleAmmoBin")))continue;
  const FVector Bin=It->GetActorLocation();FVector Along;float Best=MAX_flt;bool Park=false;
  for(TActorIterator<APiedmontPathSpline> Path(Context->GetWorld());Path;++Path){auto* S=Path->Centerline.Get();const float Key=S->FindInputKeyClosestToWorldLocation(Bin);float D=FVector::DistSquared2D(Bin,S->GetLocationAtSplineInputKey(Key,ESplineCoordinateSpace::World));if(D<Best){Best=D;Park=Path->bArtifactEligible;Along=S->GetDirectionAtSplineInputKey(Key,ESplineCoordinateSpace::World).GetSafeNormal2D();}}
  if(Along.IsNearlyZero()||(Park?ParkCount:TrailCount)>=3)continue;
  for(float Sign:{1.f,-1.f}){
   FVector Candidate=Bin+Along*(Sign*180);FHitResult Ground;FCollisionQueryParams Q(SCENE_QUERY_STAT(SpareBikeSite),false,Source);
   if(!Context->GetWorld()->LineTraceSingleByChannel(Ground,Candidate+FVector(0,0,400),Candidate-FVector(0,0,700),ECC_Visibility,Q)||Ground.ImpactNormal.Z<.95f)continue;
   Candidate=Ground.ImpactPoint+FVector(0,0,98);
   bool Wet=false;for(TActorIterator<APiedmontWaterHazard> Water(Context->GetWorld());Water&&!Wet;++Water)Wet=Water->ContainsBike(Candidate-FVector(0,0,98));if(Wet)continue;
   if(Context->GetWorld()->OverlapBlockingTestByChannel(Candidate,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(36,97),Q))continue;
   if(Create(Source,FTransform(Along.Rotation(),Candidate))){Count++;if(Park)ParkCount++;else TrailCount++;break;}
  }
 }
 UE_LOG(LogTemp,Display,TEXT("BattleSpareBikes: spawned=%d"),Count);
}
AActor* Nearest(const ABattleRider* Person,float Range){
 if(!Person)return nullptr;AActor* Best=nullptr;float Distance=Range*Range;
 if(StationWorld!=Person->GetWorld())return nullptr;
 for(auto Weak:Stations){auto* It=Weak.Get();if(!It)continue;const float D=FVector::DistSquared(It->GetActorLocation(),Person->GetActorLocation());if(D>Distance)continue;
  FCollisionQueryParams Q(SCENE_QUERY_STAT(SpareBikeUse),false,Person);Q.AddIgnoredActor(It);if(Person->ParkedBike)Q.AddIgnoredActor(Person->ParkedBike);FHitResult Hit;
  if(Person->GetWorld()->LineTraceSingleByChannel(Hit,Person->GetActorLocation(),It->GetActorLocation(),ECC_Visibility,Q))continue;Best=It;Distance=D;
 }
 return Best;
}
void Locations(const UObject* Context,TArray<FVector>& Out){if(!Context||StationWorld!=Context->GetWorld())return;for(auto Weak:Stations)if(auto* A=Weak.Get())Out.Add(A->GetActorLocation());}
bool Mount(ABattleRider* Person,AActor* Spare){
 if(!Person||!Spare||!Spare->ActorHasTag(TEXT("BattleSpareBike"))||Person->bSwimming||Person->Health<=0||!IsValid(Person->ParkedBike))return false;
 auto* Bike=Person->ParkedBike.Get();if(Bike->StunRemaining>0||Bike->bCrashActive||!Bike->bParked||Nearest(Person)!=Spare)return false;
 const FTransform Abandoned=Bike->GetActorTransform(),Destination=Spare->GetActorTransform();Spare->SetActorEnableCollision(false);
 Bike->SetActorTransform(Destination,false,nullptr,ETeleportType::TeleportPhysics);
 if(!Bike->Remount(Person)){Bike->SetActorTransform(Abandoned,false,nullptr,ETeleportType::TeleportPhysics);Spare->SetActorEnableCollision(true);return false;}
 // Keep the player's durable state on the possessed bike; exchange parked locations.
 Spare->SetActorTransform(Abandoned);Spare->SetActorEnableCollision(true);Bike->Ride->LastSafeLocation=Destination.GetLocation();Bike->Ride->LastDryLocation=Destination.GetLocation();Bike->Ride->bHasDryLocation=true;Bike->Ride->bForceNextFloorCheck=true;
 return true;
}
}
