#include "PiedmontBlood.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "UObject/ConstructorHelpers.h"
APiedmontBlood::APiedmontBlood(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("BloodOrigin"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Plane(TEXT("/Engine/BasicShapes/Plane.Plane"));
 for(int I=0;I<14;++I){auto* Drop=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Drop%d"),I));Drop->SetupAttachment(RootComponent);Drop->SetStaticMesh(Plane.Object);Drop->SetCollisionEnabled(ECollisionEnabled::NoCollision);Drop->SetCanEverAffectNavigation(false);Drop->SetCastShadow(false);Drops.Add(Drop);}
 SetActorEnableCollision(false);InitialLifeSpan=.55f;
}
void APiedmontBlood::BeginPlay(){
 Super::BeginPlay();auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/PiedmontRide/Materials/M_BloodSpray.M_BloodSpray"));
 SprayMaterial=Base?UMaterialInstanceDynamic::Create(Base,this):nullptr;
 const FVector Axis=SprayDirection.GetSafeNormal(UE_SMALL_NUMBER,FVector::UpVector);
 for(int I=0;I<Drops.Num();I++){
  Drops[I]->SetMaterial(0,SprayMaterial);Sizes.Add(FVector(FMath::FRandRange(3.f,7.f),FMath::FRandRange(.6f,1.4f),1));
  Velocities.Add(Axis*(I<10?-1.f:1.f)*FMath::FRandRange(90.f,190.f)+FMath::VRand()*90.f+FVector(0,0,35));
 }
 UpdateSpray(0);
}
void APiedmontBlood::Tick(float Dt){Super::Tick(Dt);Age+=Dt;UpdateSpray(Age);}
void APiedmontBlood::UpdateSpray(float Time){
 auto* Camera=UGameplayStatics::GetPlayerCameraManager(this,0);
 if(SprayMaterial)SprayMaterial->SetScalarParameterValue(TEXT("Opacity"),FMath::Square(FMath::Clamp(1.f-Time/.55f,0.f,1.f)));
 for(int I=0;I<Drops.Num();I++){
  const FVector Offset=Velocities[I]*Time+FVector(0,0,-490.f*Time*Time);
  Drops[I]->SetRelativeLocation(Offset);
  const FVector Facing=Camera?(Camera->GetCameraLocation()-GetActorLocation()-Offset).GetSafeNormal():FVector::ForwardVector;
  FVector Along=FVector::VectorPlaneProject(Velocities[I]+FVector(0,0,-980.f*Time),Facing).GetSafeNormal();
  if(Along.IsNearlyZero())Along=FVector::VectorPlaneProject(FVector::UpVector,Facing).GetSafeNormal();
  Drops[I]->SetWorldRotation(FRotationMatrix::MakeFromXZ(Along,Facing).Rotator());
  Drops[I]->SetRelativeScale3D(FVector(Sizes[I].X/100.f,Sizes[I].Y/100.f,1));
 }
}
void APiedmontBlood::Burst(UWorld* World,FVector Location,FVector Direction){
 if(!World)return;
 // A shotgun's pellet hits share one local spray; damage is handled separately.
 int32 Active=0;for(TActorIterator<APiedmontBlood> It(World);It;++It){if(It->IsActorBeingDestroyed())continue;Active++;if(It->Age<.06f&&FVector::DistSquared(It->GetActorLocation(),Location)<FMath::Square(45.f))return;}
 if(Active>=16)return;
 FTransform Transform(Location);
 if(auto* Blood=World->SpawnActorDeferred<APiedmontBlood>(StaticClass(),Transform,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn)){Blood->SprayDirection=Direction;Blood->FinishSpawning(Transform);}
}
