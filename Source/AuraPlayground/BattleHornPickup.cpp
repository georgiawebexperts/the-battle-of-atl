#include "BattlePickup.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
ABattleHornPickup::ABattleHornPickup(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("HornRoot"));Tags.Add(TEXT("BattleHornPickup"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube")),Cone(TEXT("/Engine/BasicShapes/Cone.Cone")),Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere")),Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Metal(TEXT("/Game/BattleForTheA/Materials/M_LockSteel.M_LockSteel")),Rubber(TEXT("/Game/BattleForTheA/Furniture/M_BenchFrame.M_BenchFrame")),TextMat(TEXT("/Engine/EngineMaterials/UnlitText.UnlitText"));
 auto Part=[&](const TCHAR* Name,UStaticMesh* Mesh,FVector P,FVector S,FRotator R,UMaterialInterface* Mat){auto* M=CreateDefaultSubobject<UStaticMeshComponent>(Name);M->SetupAttachment(RootComponent);M->SetStaticMesh(Mesh);M->SetMaterial(0,Mat);M->SetRelativeLocation(P);M->SetRelativeScale3D(S);M->SetRelativeRotation(R);M->SetCollisionEnabled(ECollisionEnabled::NoCollision);M->SetCanEverAffectNavigation(false);};
 Part(TEXT("RubberBulb"),Sphere.Object,FVector(-33,0,0),FVector(.30,.23,.23),FRotator::ZeroRotator,Rubber.Object);
 Part(TEXT("Stem"),Cylinder.Object,FVector(-15,0,0),FVector(.08,.08,.30),FRotator(90,0,0),Metal.Object);
 Part(TEXT("Bell"),Cone.Object,FVector(9,0,0),FVector(.36,.36,.45),FRotator(90,0,0),Metal.Object);
 Part(TEXT("BellOpening"),Cylinder.Object,FVector(31.8,0,0),FVector(.32,.32,.006),FRotator(90,0,0),Rubber.Object);
 Part(TEXT("LabelBacking"),Cube.Object,FVector(0,0,55),FVector(.025,1.1,.32),FRotator::ZeroRotator,Rubber.Object);
 auto* Glow=CreateDefaultSubobject<UPointLightComponent>(TEXT("HornGlow"));Glow->SetupAttachment(RootComponent);Glow->SetLightColor(FLinearColor(1,.65,.2));Glow->SetIntensity(160);Glow->SetAttenuationRadius(200);Glow->SetCastShadows(false);
 for(int I=0;I<2;I++){auto* Label=CreateDefaultSubobject<UTextRenderComponent>(*FString::Printf(TEXT("HornLabel%d"),I));Label->SetupAttachment(RootComponent);Label->SetRelativeLocation(FVector(I?-2:2,0,43));Label->SetRelativeRotation(FRotator(0,I?180:0,0));Label->SetWorldSize(24);Label->SetHorizontalAlignment(EHTA_Center);Label->SetText(FText::FromString(TEXT("HORN +3")));Label->SetTextRenderColor(FColor(255,211,139));Label->SetTextMaterial(TextMat.Object);Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);Label->SetCanEverAffectNavigation(false);}
}
void ABattleHornPickup::Tick(float Dt){Super::Tick(Dt);if(bConsumed)return;AddActorWorldRotation(FRotator(0,35*Dt,0));TryCollect(UGameplayStatics::GetPlayerPawn(this,0));}
bool ABattleHornPickup::TryCollect(APawn* Pawn){
 if(bConsumed||!Pawn||!Pawn->IsPlayerControlled()||FVector::DistSquared(Pawn->GetActorLocation(),GetActorLocation())>FMath::Square(150.f))return false;
 auto* Bike=Cast<ABattleBike>(Pawn);if(auto* Person=Cast<ABattleRider>(Pawn))Bike=Person->ParkedBike.Get();if(!Bike)return false;
 FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(HornPickupSight),false,Pawn);Q.AddIgnoredActor(this);if(GetWorld()->LineTraceSingleByChannel(Hit,Pawn->GetActorLocation(),GetActorLocation(),ECC_Visibility,Q))return false;
 if(Bike->AddHornUses(3)<=0)return false;bConsumed=true;
 if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_Boost.S_Boost")))UGameplayStatics::PlaySoundAtLocation(this,Sound,GetActorLocation(),.5f);
 Destroy();return true;
}
