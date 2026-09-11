#include "BattlePickup.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleCheckpoints.h"
#include "PiedmontPathSpline.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/CommandLine.h"

ABattleColaPickup::ABattleColaPickup(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("PickupRoot"));
 Can=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ColaCan"));Can->SetupAttachment(RootComponent);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));Can->SetStaticMesh(Cylinder.Object);Can->SetRelativeScale3D(FVector(.22,.22,.4));Can->SetCollisionEnabled(ECollisionEnabled::NoCollision);Can->SetCanEverAffectNavigation(false);
 Glow=CreateDefaultSubobject<UPointLightComponent>(TEXT("PickupGlow"));Glow->SetupAttachment(RootComponent);Glow->SetIntensity(120);Glow->SetAttenuationRadius(180);Glow->SetLightColor(FLinearColor(1,.12,.04));Glow->SetCastShadows(false);
 for(int32 I=0;I<2;I++){
  auto* Label=CreateDefaultSubobject<UTextRenderComponent>(*FString::Printf(TEXT("ColaLabel%d"),I));Label->SetupAttachment(RootComponent);Label->SetRelativeLocation(FVector(I?-12:12,0,3));Label->SetRelativeRotation(FRotator(0,I?180:0,0));Label->SetWorldSize(5);Label->SetText(FText::FromString(TEXT("Coca-Cola")));Label->SetHorizontalAlignment(EHTA_Center);Label->SetTextRenderColor(FColor::White);Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);Label->SetCanEverAffectNavigation(false);
 }
 Tags.Add(TEXT("BattleHealthPickup"));
}
void ABattleColaPickup::BeginPlay(){Super::BeginPlay();RestLocation=GetActorLocation();Can->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_ColaRed.M_ColaRed")));}
void ABattleColaPickup::Tick(float Dt){
 Super::Tick(Dt);if(bConsumed)return;Clock+=Dt;SetActorLocation(RestLocation+FVector(0,0,FMath::Sin(Clock*2.5f)*5));AddActorWorldRotation(FRotator(0,45*Dt,0));TryCollect(UGameplayStatics::GetPlayerPawn(this,0));
}
bool ABattleColaPickup::TryCollect(APawn* Pawn){
 if(bConsumed||!Pawn||!Pawn->IsPlayerControlled()||FVector::DistSquared(Pawn->GetActorLocation(),GetActorLocation())>FMath::Square(150.f))return false;
 auto* Bike=Cast<ABattleBike>(Pawn);if(auto* Person=Cast<ABattleRider>(Pawn))Bike=Person->ParkedBike;if(!Bike)return false;
 FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(ColaPickupSight),false,Pawn);Q.AddIgnoredActor(this);
 if(GetWorld()->LineTraceSingleByChannel(Hit,Pawn->GetActorLocation(),GetActorLocation(),ECC_Visibility,Q))return false;
 const float Applied=Bike->RestoreRiderHealth(HealAmount);if(Applied<=0)return false;
 bConsumed=true;Bike->HealthPickups++;Bike->LastHealAmount=Applied;Bike->PickupNoticeRemaining=2;
 if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_ColaOpen.S_ColaOpen")))UGameplayStatics::PlaySoundAtLocation(this,Sound,GetActorLocation(),.7f);
 Destroy();return true;
}
ABattlePickupDirector::ABattlePickupDirector(){PrimaryActorTick.bCanEverTick=false;}
bool ABattlePickupDirector::SpawnCola(FVector Surface,bool Trail,float Heal,int32 WeaponSlot){
 for(const FVector& P:Locations)if(FVector::DistSquared2D(P,Surface)<FMath::Square(1600.f))return false;
 auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);if(!Pawn)return false;
 FHitResult Ground;FCollisionQueryParams Q(SCENE_QUERY_STAT(ColaPlacement),false,Pawn);
 if(!GetWorld()->LineTraceSingleByChannel(Ground,Surface+FVector(0,0,150),Surface-FVector(0,0,150),ECC_Visibility,Q)||!Ground.GetActor())return false;
 const auto* Floor=Ground.GetActor();if(!Floor->ActorHasTag(TEXT("RidePath"))&&!Floor->ActorHasTag(TEXT("RideDirt"))&&!Floor->ActorHasTag(TEXT("RideBridge")))return false;
 const FVector Spot=Ground.ImpactPoint+FVector(0,0,65);
 if(!Floor->ActorHasTag(TEXT("RideBridge")))for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->ContainsBike(Ground.ImpactPoint+FVector(0,0,98)))return false;
 if(GetWorld()->OverlapBlockingTestByChannel(Spot,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeSphere(28),Q))return false;
 auto* Route=UNavigationSystemV1::FindPathToLocationSynchronously(this,Pawn->GetActorLocation(),Ground.ImpactPoint,Pawn);
 if(!Route||!Route->IsValid()||Route->IsPartial())return false;
 if(WeaponSlot>=0){
  if(auto* Crate=GetWorld()->SpawnActorDeferred<ABattleWeaponCrate>(ABattleWeaponCrate::StaticClass(),FTransform(Spot),this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn)){Crate->WeaponSlot=WeaponSlot;Crate->FinishSpawning(FTransform(Spot));Locations.Add(Spot);WeaponCrates++;return true;}return false;
 }
 if(auto* Pickup=GetWorld()->SpawnActorDeferred<ABattleColaPickup>(ABattleColaPickup::StaticClass(),FTransform(Spot),this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn)){
  Pickup->HealAmount=Heal;Pickup->bTrailPickup=Trail;Pickup->FinishSpawning(FTransform(Spot));Locations.Add(Spot);Spawned++;if(Trail)TrailPickups++;else ParkPickups++;return true;
 }return false;
}
void ABattlePickupDirector::BeginPlay(){
 Super::BeginPlay();
#if !UE_BUILD_SHIPPING
 for(const TCHAR* Flag:{TEXT("BattleMeleeAudit"),TEXT("BattleAudit"),TEXT("BattleHealthAudit"),TEXT("BattleGeographyAudit"),TEXT("BattleConnectorAudit"),TEXT("BattleEastsideAudit"),TEXT("BattleKrogAudit"),TEXT("BattleZombieAudit"),TEXT("BattleZombiePopulationAudit")})if(FParse::Param(FCommandLine::Get(),Flag))return;
#endif
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!Mode)return;
 const int32 Desired=Mode->Difficulty.HealthPickups;const int32 TrailGoal=FMath::Min(Desired,FMath::Max(2,Desired/3));const float Heal=Mode->Difficulty.ColaHealAmount;
 for(const auto& A:BattleCheckpoints::Anchors)if(TrailPickups<TrailGoal)SpawnCola(FVector(A.X,A.Y,A.Z),true,Heal);
 TArray<FVector> Park,Trail;
 for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It){auto* S=It->Centerline.Get();auto& Candidates=It->bArtifactEligible?Park:Trail;
  for(float D=FMath::Min(500.f,S->GetSplineLength()*.5f);D<S->GetSplineLength();D+=1800)Candidates.Add(S->GetLocationAtDistanceAlongSpline(D,ESplineCoordinateSpace::World));
 }
 auto Fill=[&](TArray<FVector>& Candidates,bool IsTrail,int32 Goal){
  while(!Candidates.IsEmpty()&&(IsTrail?TrailPickups:ParkPickups)<Goal){const int32 I=FMath::RandHelper(Candidates.Num());const FVector P=Candidates[I];Candidates.RemoveAtSwap(I);if(!SpawnCola(P,IsTrail,Heal))PlacementFailures++;}
 };
 Fill(Trail,true,TrailGoal);Fill(Park,false,Desired-TrailGoal);
#if !UE_BUILD_SHIPPING
 const bool ColaAudit=FParse::Param(FCommandLine::Get(),TEXT("BattlePickupAudit"));
#else
 const bool ColaAudit=false;
#endif
 if(!ColaAudit){
  TArray<FVector> Supplies=Park;Supplies.Append(Trail);
  while(!Supplies.IsEmpty()&&WeaponCrates<Mode->Difficulty.WeaponCrates){const int32 I=FMath::RandHelper(Supplies.Num());const FVector P=Supplies[I];Supplies.RemoveAtSwap(I);SpawnCola(P,false,0,1+WeaponCrates%2);}
  UE_LOG(LogTemp,Display,TEXT("BattleCrates: spawned=%d desired=%d"),WeaponCrates,Mode->Difficulty.WeaponCrates);
 }
 UE_LOG(LogTemp,Display,TEXT("BattlePickups: spawned=%d park=%d trail=%d desired=%d"),Spawned,ParkPickups,TrailPickups,Desired);
}
