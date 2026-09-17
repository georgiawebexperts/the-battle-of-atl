#include "BattleMurderK.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "BattleBenchFire.h"

ABattleMurderK::ABattleMurderK(){
 SceneRoot=CreateDefaultSubobject<USceneComponent>(TEXT("MurderKRoot"));RootComponent=SceneRoot;
 Tags.Add(TEXT("MurderKLandmark"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 auto Part=[&](const TCHAR* Name,FVector Loc,FVector Scale,TArray<TObjectPtr<UStaticMeshComponent>>& Group,bool Round=false,bool Collision=true){
  auto* C=CreateDefaultSubobject<UStaticMeshComponent>(Name);C->SetupAttachment(SceneRoot);C->SetStaticMesh(Round?Cylinder.Object:Cube.Object);C->SetRelativeLocation(Loc);C->SetRelativeScale3D(Scale);C->SetCollisionEnabled(Collision?ECollisionEnabled::QueryAndPhysics:ECollisionEnabled::NoCollision);C->SetCollisionProfileName(Collision?TEXT("BlockAll"):TEXT("NoCollision"));Group.Add(C);return C;
 };
 // 725 Ponce is east of the northbound BeltLine, opposite Ponce City Market.
 // Its grocery level sits below a tall raw-concrete office block with a glass wing.
 // The real trail frontage opens into a broad plaza, so the riding line stays wide.
 TrailApron=Part(TEXT("TrailApron"),FVector(0,0,8),FVector(42,14,.16),ConcreteParts);
 StoreMass=Part(TEXT("StoreMass"),FVector(300,-1550,350),FVector(30,10,7),BrickParts);
 OfficeTower=Part(TEXT("OfficeTower"),FVector(450,-1700,1650),FVector(28,8.5,25),ConcreteParts);
 Part(TEXT("RoofCrown"),FVector(450,-1700,2940),FVector(29,9,.65),DarkParts);
 GlassWing=Part(TEXT("GlassWing"),FVector(1320,-1180,1680),FVector(6.5,5.2,20),GlassParts);
 Part(TEXT("StorefrontApron"),FVector(250,-720,7),FVector(36,5,.14),ConcreteParts);
 Part(TEXT("Awning"),FVector(300,-1015,520),FVector(12,.9,.22),RedParts);
 Part(TEXT("StorefrontGlass"),FVector(300,-1025,275),FVector(12,.12,2.4),GlassParts,false,false);
 for(int X=-1150;X<=1850;X+=600)Part(*FString::Printf(TEXT("FacadeColumn%d"),X),FVector(X,-930,650),FVector(.38,.38,6.5),ConcreteParts);
 for(int X=-900;X<=1800;X+=300)for(int Z=900;Z<=2500;Z+=320)Part(*FString::Printf(TEXT("OfficeWindow%d_%d"),X,Z),FVector(X,-1268,Z),FVector(1.05,.10,1.05),GlassParts,false,false);
 for(int X:{-1300,1800}){
  Part(*FString::Printf(TEXT("StickerPole%d"),X),FVector(X,-760,170),FVector(.10,.10,3.4),DarkParts,true);
  for(int Z:{70,135,205})Part(*FString::Printf(TEXT("PoleSticker%d_%d"),X,Z),FVector(X,-760,Z),FVector(.13,.13,.09),RedParts,true,false);
 }
 Part(TEXT("BurnBarrelA"),FVector(-1050,-790,48),FVector(.45,.45,.75),DarkParts,true);
 Part(TEXT("BurnBarrelB"),FVector(1450,-820,48),FVector(.45,.45,.75),DarkParts,true);
 StoreSign=CreateDefaultSubobject<UTextRenderComponent>(TEXT("MurderKSign"));StoreSign->SetupAttachment(SceneRoot);StoreSign->SetRelativeLocation(FVector(300,-995,535));StoreSign->SetRelativeRotation(FRotator(0,90,0));StoreSign->SetHorizontalAlignment(EHTA_Center);StoreSign->SetText(FText::FromString(TEXT("MURDER K")));StoreSign->SetWorldSize(128);StoreSign->SetTextRenderColor(FColor(245,236,216));
 GraffitiSign=CreateDefaultSubobject<UTextRenderComponent>(TEXT("MurderGraffiti"));GraffitiSign->SetupAttachment(SceneRoot);GraffitiSign->SetRelativeLocation(FVector(500,-1260,2660));GraffitiSign->SetRelativeRotation(FRotator(-5,90,-7));GraffitiSign->SetHorizontalAlignment(EHTA_Center);GraffitiSign->SetText(FText::FromString(TEXT("MURDER")));GraffitiSign->SetWorldSize(105);GraffitiSign->SetTextRenderColor(FColor(180,28,35));
}
void ABattleMurderK::BeginPlay(){
 Super::BeginPlay();
 auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
 auto Paint=[&](TArray<TObjectPtr<UStaticMeshComponent>>& Parts,FLinearColor Color){
  auto* Mat=UMaterialInstanceDynamic::Create(Base,this);if(Mat)Mat->SetVectorParameterValue(TEXT("Color"),Color);for(auto& Part:Parts)if(Part)Part->SetMaterial(0,Mat);
 };
 Paint(BrickParts,FLinearColor(.20f,.10f,.065f));Paint(DarkParts,FLinearColor(.025f,.035f,.045f));Paint(RedParts,FLinearColor(.55f,.018f,.025f));Paint(GlassParts,FLinearColor(.035f,.13f,.18f));Paint(ConcreteParts,FLinearColor(.30f,.31f,.30f));
 if(auto* Mat=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Environment/FancyRoachMotel/M_Brick.M_Brick")))for(auto& Part:BrickParts)Part->SetMaterial(0,Mat);
 if(auto* Mat=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Environment/FancyRoachMotel/M_Glass.M_Glass")))for(auto& Part:GlassParts)Part->SetMaterial(0,Mat);
 if(auto* Mat=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Environment/KrogApproach/M_KrogApproachConcrete.M_KrogApproachConcrete")))for(auto& Part:ConcreteParts)Part->SetMaterial(0,Mat);
 for(const FVector Local:{FVector(-1050,-790,20),FVector(1450,-820,20)}){
  const FTransform T(FRotator::ZeroRotator,GetActorTransform().TransformPosition(Local));
  if(auto* Fire=GetWorld()->SpawnActorDeferred<ABattleBenchFire>(ABattleBenchFire::StaticClass(),T,this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn)){Fire->Duration=3600;Fire->FinishSpawning(T);}
 }
}
