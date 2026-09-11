#include "BattlePickup.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
ABattleWeaponCrate::ABattleWeaponCrate(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("CrateRoot"));
 Box=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponCrate"));Box->SetupAttachment(RootComponent);static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));Box->SetStaticMesh(Cube.Object);Box->SetRelativeScale3D(FVector(.65,.38,.3));Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);Box->SetCanEverAffectNavigation(false);
 Glow=CreateDefaultSubobject<UPointLightComponent>(TEXT("CrateGlow"));Glow->SetupAttachment(RootComponent);Glow->SetLightColor(FLinearColor(.05,.7,1));Glow->SetIntensity(250);Glow->SetAttenuationRadius(240);Glow->SetCastShadows(false);
}
void ABattleWeaponCrate::BeginPlay(){
 Super::BeginPlay();Box->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_LockSteel.M_LockSteel")));
 for(int32 I=0;I<2;I++){auto* Text=NewObject<UTextRenderComponent>(this);Text->SetupAttachment(RootComponent);Text->SetRelativeLocation(FVector(I?-34:34,0,20));Text->SetRelativeRotation(FRotator(0,I?180:0,0));Text->SetText(FText::FromString(BattleWeapons::Name(WeaponSlot)));Text->SetWorldSize(10);Text->SetHorizontalAlignment(EHTA_Center);Text->SetTextRenderColor(FColor::Cyan);Text->SetCollisionEnabled(ECollisionEnabled::NoCollision);Text->SetCanEverAffectNavigation(false);Text->RegisterComponent();}
}
void ABattleWeaponCrate::Tick(float Dt){Super::Tick(Dt);Glow->SetIntensity(200+70*FMath::Sin(GetWorld()->GetTimeSeconds()*3));TryCollect(UGameplayStatics::GetPlayerPawn(this,0));}
bool ABattleWeaponCrate::TryCollect(APawn* Pawn){
 if(bConsumed||!Pawn||!Pawn->IsPlayerControlled()||FVector::DistSquared(GetActorLocation(),Pawn->GetActorLocation())>FMath::Square(150.f))return false;
 auto* Bike=Cast<ABattleBike>(Pawn);auto* Person=Cast<ABattleRider>(Pawn);if(Person)Bike=Person->ParkedBike;if(!Bike)return false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(WeaponPickup),false,Pawn);Q.AddIgnoredActor(this);FHitResult Hit;if(GetWorld()->LineTraceSingleByChannel(Hit,Pawn->GetActorLocation(),GetActorLocation(),ECC_Visibility,Q))return false;
 if(!Bike->GiveWeapon(WeaponSlot,WeaponSlot==1?18:90))return false;bConsumed=true;Destroy();return true;
}
