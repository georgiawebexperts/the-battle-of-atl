#include "BattleSkyline.h"
#include "BattleTutorialData.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ABattleSkyline::ABattleSkyline(){
 PrimaryActorTick.bCanEverTick=false;Tags.Add(TEXT("BattleSkyline"));
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("SkylineRoot"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Haze(TEXT("/Game/PiedmontRide/Materials/M_Concrete.M_Concrete"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> CrownMat(TEXT("/Game/BattleForTheA/Furniture/M_BenchFrame.M_BenchFrame"));
 Blocks=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("SkylineBlocks"));
 Blocks->SetupAttachment(RootComponent);Blocks->SetStaticMesh(Cube.Object);Blocks->SetMaterial(0,Haze.Object);
 Blocks->SetCollisionEnabled(ECollisionEnabled::NoCollision);Blocks->SetCanEverAffectNavigation(false);
 Blocks->SetCastShadow(false);
 Crown=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("SkylineCrowns"));
 Crown->SetupAttachment(RootComponent);Crown->SetStaticMesh(Cone.Object);Crown->SetMaterial(0,CrownMat.Object);
 Crown->SetCollisionEnabled(ECollisionEnabled::NoCollision);Crown->SetCanEverAffectNavigation(false);
 Crown->SetCastShadow(false);
}

void ABattleSkyline::BeginPlay(){
 Super::BeginPlay();
 // Downtown sits south-west of Piedmont Park. Anchoring to the 14th Street gate
 // keeps the skyline behind the park no matter where the player is riding.
 const FVector Gate=BattleTutorialData::Gate;
 Centre=Gate+FVector(-13500.f,66000.f,0.f);
 // The cluster sits on a ridge that is ~7 m above the gate, so drop it onto the
 // local ground rather than the gate's height or half the towers read buried.
 FHitResult Ground;FCollisionQueryParams Q;Q.bIgnoreTouches=true;
 if(GetWorld()->LineTraceSingleByChannel(Ground,Centre+FVector(0,0,20000),Centre-FVector(0,0,20000),ECC_Visibility,Q))Centre.Z=Ground.ImpactPoint.Z;
 SetActorLocation(Centre);
 // A fixed seed keeps the silhouette identical between runs and reviews.
 FRandomStream Random(20260917);
 auto Tower=[&](float X,float Y,float Width,float Depth,float Height,float Yaw){
  Blocks->AddInstance(FTransform(FRotator(0,Yaw,0),FVector(X,Y,Height*.5f),FVector(Width/100.f,Depth/100.f,Height/100.f)));
  if(Height>4200.f)Crown->AddInstance(FTransform(FRotator(0,Yaw,0),FVector(X,Y,Height+120.f),FVector(Width*.35f/100.f,Depth*.35f/100.f,.75f)));
  TallestM=FMath::Max(TallestM,Height/100.f*3.f); // game cm -> authored metres at 1:3
 };
 // Core cluster: a few slabs and towers, tallest in the middle.
 for(int32 I=0;I<26;++I){
  const float Angle=Random.FRandRange(0.f,6.283f),Radius=Random.FRandRange(300.f,5200.f);
  const float Height=Random.FRandRange(2600.f,5600.f);
  Tower(FMath::Cos(Angle)*Radius,FMath::Sin(Angle)*Radius*.8f,Random.FRandRange(700.f,1500.f),Random.FRandRange(700.f,1500.f),Height,Random.FRandRange(-20.f,20.f));
  ++Towers;
 }
 // Landmark pair: one spire and one wide midtown slab, like the real skyline.
 Tower(-600.f,300.f,1600.f,1600.f,7400.f,12.f);
 Tower(1500.f,-900.f,1100.f,1400.f,4200.f,-8.f);
 Tower(-2400.f,1400.f,900.f,900.f,3100.f,24.f);
 Towers+=3;
 // A low ring of midtown blocks so the cluster does not float alone.
 for(int32 I=0;I<14;++I){
  const float Angle=I*6.283f/14.f,Radius=6400.f+Random.FRandRange(-600.f,600.f);
  Tower(FMath::Cos(Angle)*Radius,FMath::Sin(Angle)*Radius*.8f,Random.FRandRange(600.f,1200.f),Random.FRandRange(600.f,1200.f),Random.FRandRange(1600.f,3000.f),Random.FRandRange(-30.f,30.f));
  ++Towers;
 }
 UE_LOG(LogTemp,Display,TEXT("BattleSkyline: centre=%s towers=%d tallest_m=%.0f gate=%s"),*Centre.ToString(),Towers,TallestM,*Gate.ToString());
}
