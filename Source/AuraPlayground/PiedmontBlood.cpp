#include "PiedmontBlood.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
APiedmontBlood::APiedmontBlood(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("BloodOrigin"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
 for(int I=0;I<12;++I){auto* Drop=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Drop%d"),I));Drop->SetupAttachment(RootComponent);Drop->SetStaticMesh(Sphere.Object);Drop->SetCollisionEnabled(ECollisionEnabled::NoCollision);Drop->SetCastShadow(false);Drops.Add(Drop);}
 SetActorEnableCollision(false);InitialLifeSpan=1.2f;
}
void APiedmontBlood::BeginPlay(){
 Super::BeginPlay();auto* Material=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/PiedmontRide/Materials/M_Blood.M_Blood"),nullptr,LOAD_NoWarn);
 for(const auto& Drop:Drops){Drop->SetMaterial(0,Material);Drop->SetRelativeScale3D(FVector(FMath::FRandRange(.025f,.065f)));Velocities.Add(SprayDirection*FMath::FRandRange(60.f,140.f)+FMath::VRand()*100+FVector(0,0,90));}
}
void APiedmontBlood::Tick(float Dt){Super::Tick(Dt);Dt=FMath::Min(Dt,.05f);for(int I=0;I<Drops.Num();++I){Velocities[I].Z-=650*Dt;Drops[I]->AddRelativeLocation(Velocities[I]*Dt);}}
void APiedmontBlood::Burst(UWorld* World,FVector Location,FVector Direction){
 if(!World)return;FTransform Transform(Location);
 if(auto* Blood=World->SpawnActorDeferred<APiedmontBlood>(StaticClass(),Transform,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn)){Blood->SprayDirection=Direction;Blood->FinishSpawning(Transform);}
}
