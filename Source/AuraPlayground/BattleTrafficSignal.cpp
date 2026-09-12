#include "BattleTrafficSignal.h"
#include "Engine/StaticMesh.h"
#include "BattleRoadCrossing.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
ABattleTrafficSignal::ABattleTrafficSignal(){
 PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=.05f;
 SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("SignalRoot")));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube")),Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder")),Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Housing(TEXT("/Game/BattleForTheA/Traffic/M_SignalHousing.M_SignalHousing")),Steel(TEXT("/Game/BattleForTheA/Traffic/M_SignalPole.M_SignalPole")),Lens(TEXT("/Game/BattleForTheA/Traffic/M_SignalLens.M_SignalLens"));
 auto Part=[&](const TCHAR* Name,UStaticMesh* Mesh,UMaterialInterface* Material,FVector Position,FVector Scale){auto* C=CreateDefaultSubobject<UStaticMeshComponent>(Name);C->SetupAttachment(RootComponent);C->SetStaticMesh(Mesh);C->SetMaterial(0,Material);C->SetRelativeLocation(Position);C->SetRelativeScale3D(Scale);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);return C;};
 auto* Pole=Part(TEXT("Pole"),Cylinder.Object,Steel.Object,FVector(0,0,165),FVector(.09,.09,3.3));Pole->SetCollisionProfileName(TEXT("BlockAll"));
 Part(TEXT("Foot"),Cylinder.Object,Steel.Object,FVector(0,0,5),FVector(.3,.3,.1));
 Part(TEXT("Backplate"),Cube.Object,Housing.Object,FVector(0,0,370),FVector(.08,.62,1.6));
 Part(TEXT("Housing"),Cube.Object,Housing.Object,FVector(-8,0,370),FVector(.2,.46,1.4));
 for(int32 I=0;I<3;++I){const float Z=410-I*40;
  auto* Rim=Part(*FString::Printf(TEXT("Rim%d"),I),Cylinder.Object,Housing.Object,FVector(-20,0,Z),FVector(.34,.34,.06));Rim->SetRelativeRotation(FRotator(90,0,0));
  Lenses.Add(Part(*FString::Printf(TEXT("Lens%d"),I),Sphere.Object,Lens.Object,FVector(-24,0,Z),FVector(.09,.27,.27)));
  Part(*FString::Printf(TEXT("Visor%d"),I),Cube.Object,Housing.Object,FVector(-28,0,Z+18),FVector(.3,.36,.035));
 }
 Tags.Add(TEXT("RideBarrier"));
}
void ABattleTrafficSignal::BeginPlay(){
 Super::BeginPlay();for(auto* Lens:Lenses)LensMaterials.Add(Lens->CreateAndSetMaterialInstanceDynamic(0));UpdateLamps();
}
void ABattleTrafficSignal::Tick(float Dt){Super::Tick(Dt);UpdateLamps();}
void ABattleTrafficSignal::UpdateLamps(){
 const int32 Active=IsValid(Crossing)?(Crossing->bVehicleGreen?2:Crossing->bVehicleAmber?1:0):0;
 for(int32 I=0;I<LensMaterials.Num();++I)if(auto* Material=LensMaterials[I]){
  const FLinearColor Lit=I==0?FLinearColor(.3,.001,.001):I==1?FLinearColor(.4,.17,.001):FLinearColor(.002,.3,.018);
  Material->SetVectorParameterValue(TEXT("Color"),I==Active?Lit:FLinearColor(.009,.012,.009));Material->SetScalarParameterValue(TEXT("Strength"),I==Active?1.5f:0.f);
 }
}
