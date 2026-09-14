#include "BattlePickup.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "UObject/ConstructorHelpers.h"
ABattleWeaponCrate::ABattleWeaponCrate(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("CrateRoot"));
 Box=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponCrate"));Box->SetupAttachment(RootComponent);static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));Box->SetStaticMesh(Cube.Object);Box->SetRelativeScale3D(FVector(.65,.38,.3));Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);Box->SetCanEverAffectNavigation(false);
 Glow=CreateDefaultSubobject<UPointLightComponent>(TEXT("CrateGlow"));Glow->SetupAttachment(RootComponent);Glow->SetLightColor(FLinearColor(.05,.7,1));Glow->SetIntensity(250);Glow->SetAttenuationRadius(240);Glow->SetCastShadows(false);
}
void ABattleWeaponCrate::BeginPlay(){
 Super::BeginPlay();
 // Keep the pickup interaction centered at riding height while seating the
 // visible case on its supporting surface.
 FHitResult Floor;FCollisionQueryParams GroundQuery(SCENE_QUERY_STAT(WeaponCaseGround),true,this);
 GroundQuery.AddIgnoredActor(UGameplayStatics::GetPlayerPawn(this,0));
 if(GetWorld()->LineTraceSingleByChannel(Floor,GetActorLocation()+FVector(0,0,30),GetActorLocation()-FVector(0,0,180),ECC_Visibility,GroundQuery)
  &&Floor.GetActor()&&Floor.ImpactNormal.Z>.65f
  &&(Floor.GetActor()->ActorHasTag(TEXT("RidePath"))||Floor.GetActor()->ActorHasTag(TEXT("RideDirt"))||Floor.GetActor()->ActorHasTag(TEXT("RideGrass"))||Floor.GetActor()->ActorHasTag(TEXT("RideBridge")))){
  Box->SetRelativeLocation(FVector(0,0,Floor.ImpactPoint.Z-GetActorLocation().Z+15));
 }
 Box->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_LockSteel.M_LockSteel")));
 auto* Text=NewObject<UTextRenderComponent>(this);AddInstanceComponent(Text);Text->SetupAttachment(RootComponent);Text->SetRelativeLocation(FVector(0,0,35));Text->SetText(FText::FromString(WeaponSlot==0?TEXT("PISTOL AMMO +17"):BattleWeapons::Name(WeaponSlot)));Text->SetWorldSize(12);Text->SetHorizontalAlignment(EHTA_Center);Text->SetTextRenderColor(FColor(255,239,204));Text->SetTextMaterial(LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/EngineMaterials/UnlitText.UnlitText")));Text->SetCollisionEnabled(ECollisionEnabled::NoCollision);Text->SetCanEverAffectNavigation(false);Text->RegisterComponent();
}
void ABattleWeaponCrate::Tick(float Dt){Super::Tick(Dt);if(auto* Camera=UGameplayStatics::GetPlayerCameraManager(this,0)){TInlineComponentArray<UTextRenderComponent*> Labels(this);for(auto* Label:Labels)Label->SetWorldRotation((Camera->GetCameraLocation()-Label->GetComponentLocation()).Rotation());}Glow->SetIntensity(200+70*FMath::Sin(GetWorld()->GetTimeSeconds()*3));TryCollect(UGameplayStatics::GetPlayerPawn(this,0));}
bool ABattleWeaponCrate::TryCollect(APawn* Pawn){
 if(bConsumed||!Pawn||!Pawn->IsPlayerControlled()||FVector::DistSquared(GetActorLocation(),Pawn->GetActorLocation())>FMath::Square(150.f))return false;
 auto* Bike=Cast<ABattleBike>(Pawn);auto* Person=Cast<ABattleRider>(Pawn);if(Person)Bike=Person->ParkedBike;if(!Bike)return false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(WeaponPickup),false,Pawn);Q.AddIgnoredActor(this);FHitResult Hit;if(GetWorld()->LineTraceSingleByChannel(Hit,Pawn->GetActorLocation(),GetActorLocation(),ECC_Visibility,Q))return false;
 if(!Bike->GiveWeapon(WeaponSlot,WeaponSlot==0?17:WeaponSlot==1?18:WeaponSlot==2?90:WeaponSlot==4?60:16))return false;bConsumed=true;Destroy();return true;
}
