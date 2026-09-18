#include "BattleMemorial.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "BattleSpiritData.h"
#include "UObject/ConstructorHelpers.h"
ABattleMemorial::ABattleMemorial(){
 auto* Base=CreateDefaultSubobject<UBoxComponent>(TEXT("ArrangementBoundary"));RootComponent=Base;
 Base->SetBoxExtent(FVector(42,32,20));Base->SetCollisionProfileName(TEXT("BlockAll"));Base->SetCanEverAffectNavigation(true);
 // The compact base protects the flowers, with its edge clear of through traffic.
 auto Part=[&](const TCHAR* Name,UStaticMesh* Mesh)->UStaticMeshComponent*{auto* C=CreateDefaultSubobject<UStaticMeshComponent>(Name);C->SetupAttachment(RootComponent);C->SetStaticMesh(Mesh);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCanEverAffectNavigation(false);return C;};
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Stems(TEXT("/Game/BattleForTheA/Spirit/SM_MemorialStems.SM_MemorialStems")),Petals(TEXT("/Game/BattleForTheA/Spirit/SM_MemorialPetals.SM_MemorialPetals")),Lavender(TEXT("/Game/BattleForTheA/Spirit/SM_MemorialLavender.SM_MemorialLavender")),Centers(TEXT("/Game/BattleForTheA/Spirit/SM_MemorialCenters.SM_MemorialCenters")),StoneAsset(TEXT("/Game/BattleForTheA/Spirit/SM_MemorialStone.SM_MemorialStone")),Ribbon(TEXT("/Game/BattleForTheA/Spirit/SM_MemorialRibbon.SM_MemorialRibbon"));
 Part(TEXT("Stems"),Stems.Object);Part(TEXT("IvoryPetals"),Petals.Object);Part(TEXT("LavenderPetals"),Lavender.Object);Part(TEXT("FlowerCenters"),Centers.Object);Stone=Part(TEXT("SmallStone"),StoneAsset.Object);Part(TEXT("ClothTie"),Ribbon.Object);
 // A flat stone with flowers lying in long grass is invisible from a bike at
 // speed - Elliott rode past it twice and never saw it. Make the arrangement
 // read from the route, and stand a marker and words beside the flowers.
 Base->SetRelativeScale3D(FVector(1.5f));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Marker(TEXT("/Engine/BasicShapes/Cube.Cube"));
 if(Marker.Succeeded()){
  MarkerPart=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MemorialMarker"));MarkerPart->SetupAttachment(RootComponent);
  MarkerPart->SetStaticMesh(Marker.Object);MarkerPart->SetRelativeLocation(FVector(-66,0,40));MarkerPart->SetRelativeScale3D(FVector(.07f,.70f,.80f));
  MarkerPart->SetCollisionEnabled(ECollisionEnabled::NoCollision);MarkerPart->SetCanEverAffectNavigation(false);
 }
 Words=CreateDefaultSubobject<UTextRenderComponent>(TEXT("MemorialWords"));Words->SetupAttachment(RootComponent);
 Words->SetRelativeLocation(FVector(-88,0,52));Words->SetText(FText::FromString(TEXT("IN LOVING MEMORY")));
 Words->SetWorldSize(8);Words->SetHorizontalAlignment(EHTA_Center);Words->SetVerticalAlignment(EVRTA_TextCenter);
 // Dark engraved words on a pale stone; pale-on-pale was unreadable when the
 // marker faces away from the sun.
 Words->SetTextRenderColor(FColor(38,38,42));Words->SetCollisionEnabled(ECollisionEnabled::NoCollision);Words->SetCanEverAffectNavigation(false);
}
void ABattleMemorial::BeginPlay(){
 Super::BeginPlay();
 // Turn the marker and the words to face the ride line, so the memorial is
 // presented to the rider rather than pointing off into the grass. The flowers
 // and the stone stay where they are; only the upright parts swing round.
 FVector Best;float BestDistance=BIG_NUMBER;
 for(const FVector& P:BattleSpiritData::Ride){
  const float D=FVector::Dist2D(P,GetActorLocation());
  if(D<BestDistance){BestDistance=D;Best=P;}
 }
 if(BestDistance>=BIG_NUMBER)return;
 const float Face=(Best-GetActorLocation()).Rotation().Yaw;
 const FVector Toward=FRotationMatrix(FRotator(0,Face,0)).GetUnitAxis(EAxis::X);
 // The base carries a 1.5x scale, so the marker's world height is 85 cm * 1.5;
 // sitting its centre at half that puts it on the ground instead of hovering.
 const float MarkerCentre=80.f*1.5f*.5f;
 if(MarkerPart){
  // Pale stone rather than the stone mesh's own material: a vertical slab that
  // faces away from the sun goes near black with anything darker.
  if(auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"))){
   if(auto* Pale=UMaterialInstanceDynamic::Create(Base,this)){Pale->SetVectorParameterValue(TEXT("Color"),FLinearColor(1.f,.97f,.9f));MarkerPart->SetMaterial(0,Pale);}
  }else if(Stone&&Stone->GetMaterial(0))MarkerPart->SetMaterial(0,Stone->GetMaterial(0));
  MarkerPart->SetWorldLocation(GetActorLocation()-Toward*100+FVector(0,0,MarkerCentre));
  MarkerPart->SetWorldRotation(FRotator(0,Face,0));
 }
 if(Words){
  Words->SetWorldLocation(GetActorLocation()-Toward*94+FVector(0,0,MarkerCentre+14));
  Words->SetWorldRotation(FRotator(0,Face,0));
 }
}
