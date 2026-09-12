#include "BattleFallenBike.h"
#include "BattleBike.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
ABattleFallenBike::ABattleFallenBike(){
 PrimaryActorTick.bCanEverTick=false;Frame=CreateDefaultSubobject<UBoxComponent>(TEXT("FallenFrame"));SetRootComponent(Frame);
 Frame->SetBoxExtent(FVector(45,7,28));Frame->SetCollisionProfileName(TEXT("PhysicsActor"));Frame->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);Frame->SetLinearDamping(.4f);Frame->SetAngularDamping(1.5f);Frame->BodyInstance.bUseCCD=true;
}
bool ABattleFallenBike::InitializeFrom(ABattleBike* Bike,const FVector& Velocity){
 if(!Bike)return false;
 const FTransform Source=Bike->Visual->GetComponentTransform();SetActorLocationAndRotation(Source.TransformPosition(FVector(0,0,60)),Source.GetRotation(),false,nullptr,ETeleportType::TeleportPhysics);
 Frame->SetSimulatePhysics(true);
 auto Hull=[&](const TCHAR* Name,FVector Position,FVector Extent){
  UBoxComponent* Box=NewObject<UBoxComponent>(this,Name);AddInstanceComponent(Box);Box->SetBoxExtent(Extent);Box->SetCollisionProfileName(TEXT("PhysicsActor"));Box->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);Box->SetWorldTransform(FTransform(Source.GetRotation(),Source.TransformPosition(Position)));Box->RegisterComponent();Box->AttachToComponent(Frame,FAttachmentTransformRules(EAttachmentRule::KeepWorld,true));
 };
 // Thin wheel, bar and saddle hulls support a fallen bike without a full-width
 // bounding box that would leave the frame visibly hovering over the road.
 Hull(TEXT("FrontTireHull"),FVector(60,0,35),FVector(35,3,35));Hull(TEXT("RearTireHull"),FVector(-60,0,35),FVector(35,3,35));
 Hull(TEXT("HandlebarHull"),FVector(40,0,112),FVector(4,31,3));Hull(TEXT("SaddleHull"),FVector(-23,0,98),FVector(15,10,3));
 TArray<UStaticMeshComponent*> Parts;Bike->GetComponents(Parts);
 for(UStaticMeshComponent* Original:Parts){
  if(Original==Bike->Pistol||!Original->GetStaticMesh()||Original->GetAttachParent()!=Bike->Visual)continue;
  UStaticMeshComponent* Copy=NewObject<UStaticMeshComponent>(this);AddInstanceComponent(Copy);Copy->SetStaticMesh(Original->GetStaticMesh());Copy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  for(int32 I=0;I<Original->GetNumMaterials();I++)Copy->SetMaterial(I,Original->GetMaterial(I));
  Copy->SetWorldTransform(Original->GetComponentTransform());Copy->RegisterComponent();Copy->AttachToComponent(Frame,FAttachmentTransformRules::KeepWorldTransform);Original->SetVisibility(false);PartCount++;
 }
 Frame->SetMassOverrideInKg(NAME_None,22,true);Frame->SetPhysicsLinearVelocity(Velocity);Frame->SetPhysicsAngularVelocityInDegrees(Source.TransformVectorNoScale(FVector(130,0,20)));return PartCount>0;
}
