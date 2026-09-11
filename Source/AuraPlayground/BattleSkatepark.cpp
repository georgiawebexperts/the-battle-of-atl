#include "BattleSkatepark.h"
#include "BattlePickup.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
ABattleSkatepark::ABattleSkatepark(){
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("SkateparkRoot"));
 Concrete=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RideableConcrete"));Berm=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GradedSurround"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> C(TEXT("/Game/BattleForTheA/Skatepark/SM_SkateConcrete.SM_SkateConcrete"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> B(TEXT("/Game/BattleForTheA/Skatepark/SM_SkateBerm.SM_SkateBerm"));
 Concrete->SetStaticMesh(C.Object);Berm->SetStaticMesh(B.Object);Berm->ComponentTags.Add(TEXT("RideGrass"));
 for(auto* M:{Concrete.Get(),Berm.Get()}){M->SetupAttachment(RootComponent);M->SetCollisionProfileName(TEXT("BlockAll"));M->SetCanEverAffectNavigation(true);}
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Iron(TEXT("/Game/BattleForTheA/Furniture/M_BenchFrame.M_BenchFrame"));
 auto* Panel=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SignBoard"));Panel->SetupAttachment(RootComponent);Panel->SetStaticMesh(Cube.Object);Panel->SetMaterial(0,Iron.Object);Panel->SetRelativeLocation(FVector(1815,780,850));Panel->SetRelativeScale3D(FVector(.12,6,1.8));Panel->SetCollisionProfileName(TEXT("BlockAll"));
 for(int I=0;I<2;I++){auto* Post=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("SignPost%d"),I));Post->SetupAttachment(RootComponent);Post->SetStaticMesh(Cube.Object);Post->SetMaterial(0,Iron.Object);Post->SetRelativeLocation(FVector(1810,600+I*360,740));Post->SetRelativeScale3D(FVector(.1,.1,2.5));Post->SetCollisionProfileName(TEXT("BlockAll"));}
 auto* Sign=CreateDefaultSubobject<UTextRenderComponent>(TEXT("SkateparkSign"));Sign->SetupAttachment(RootComponent);Sign->SetRelativeLocation(FVector(1823,780,850));Sign->SetText(FText::FromString(TEXT("FOURTH WARD\nSKATEPARK\nBOWLS / RAMPS / +30s")));Sign->SetWorldSize(38);Sign->SetHorizontalAlignment(EHTA_Center);Sign->SetVerticalAlignment(EVRTA_TextCenter);Sign->SetTextRenderColor(FColor(245,235,207));Sign->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Tags={TEXT("RidePath"),TEXT("RideRamp"),TEXT("BattleSkatepark")};
}
void ABattleSkatepark::BeginPlay(){
 Super::BeginPlay();
 for(FVector Local:{FVector(-950,-350,510),FVector(-950,400,550),FVector(1100,-500,990)}){
  const FTransform T(GetActorTransform().TransformPosition(Local));auto* P=GetWorld()->SpawnActorDeferred<ABattleColaPickup>(ABattleColaPickup::StaticClass(),T,this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  if(P){P->bTimeBonus=true;P->Tags.Add(TEXT("SkateparkBonus"));P->FinishSpawning(T);Bonuses.Add(P);}
 }
 UE_LOG(LogTemp,Display,TEXT("BattleSkatepark: bonuses=%d location=%s"),Bonuses.Num(),*GetActorLocation().ToString());
}
