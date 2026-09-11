#include "BattleMemorial.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
ABattleMemorial::ABattleMemorial(){
 auto* Base=CreateDefaultSubobject<UBoxComponent>(TEXT("ArrangementBoundary"));RootComponent=Base;
 Base->SetBoxExtent(FVector(42,32,20));Base->SetCollisionProfileName(TEXT("BlockAll"));Base->SetCanEverAffectNavigation(true);
 // The compact base protects the flowers, with its edge clear of through traffic.
 auto Part=[&](const TCHAR* Name,UStaticMesh* Mesh){auto* C=CreateDefaultSubobject<UStaticMeshComponent>(Name);C->SetupAttachment(RootComponent);C->SetStaticMesh(Mesh);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCanEverAffectNavigation(false);};
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Stems(TEXT("/Game/BattleForTheA/Spirit/SM_MemorialStems.SM_MemorialStems")),Petals(TEXT("/Game/BattleForTheA/Spirit/SM_MemorialPetals.SM_MemorialPetals")),Lavender(TEXT("/Game/BattleForTheA/Spirit/SM_MemorialLavender.SM_MemorialLavender")),Centers(TEXT("/Game/BattleForTheA/Spirit/SM_MemorialCenters.SM_MemorialCenters")),Stone(TEXT("/Game/BattleForTheA/Spirit/SM_MemorialStone.SM_MemorialStone")),Ribbon(TEXT("/Game/BattleForTheA/Spirit/SM_MemorialRibbon.SM_MemorialRibbon"));
 Part(TEXT("Stems"),Stems.Object);Part(TEXT("IvoryPetals"),Petals.Object);Part(TEXT("LavenderPetals"),Lavender.Object);Part(TEXT("FlowerCenters"),Centers.Object);Part(TEXT("SmallStone"),Stone.Object);Part(TEXT("ClothTie"),Ribbon.Object);
}
