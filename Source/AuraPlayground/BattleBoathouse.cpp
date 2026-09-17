#include "BattleBoathouse.h"
#include "PiedmontBike.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace {
// The guitarist already stands at the mapped dockside, so anchor the boathouse
// to the shore nearest that spot instead of inventing a second location.
const FVector DocksideAnchor(-15520,3150,0);
}

ABattleBoathouse::ABattleBoathouse(){
 PrimaryActorTick.bCanEverTick=false;
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("BoathouseRoot"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> StoneMat(TEXT("/Game/PiedmontRide/Materials/M_Concrete.M_Concrete"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> SidingMat(TEXT("/Game/BattleForTheA/Environment/TutorialHouse/M_Siding.M_Siding"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> TrimMat(TEXT("/Game/BattleForTheA/Environment/TutorialHouse/M_Trim.M_Trim"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> DoorMat(TEXT("/Game/BattleForTheA/Environment/TutorialHouse/M_Door.M_Door"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> DeckMat(TEXT("/Game/PiedmontRide/Materials/M_BridgeDeck.M_BridgeDeck"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> RailMat(TEXT("/Game/PiedmontRide/Materials/M_BridgeRails.M_BridgeRails"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> WoodMat(TEXT("/Game/PiedmontRide/Materials/M_BridgeWood.M_BridgeWood"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> RoofMat(TEXT("/Game/BattleForTheA/Furniture/M_BenchFrame.M_BenchFrame"));
 auto Group=[&](const TCHAR* Name,UMaterialInterface* Material,bool bCollide){
  auto* M=CreateDefaultSubobject<UInstancedStaticMeshComponent>(Name);
  M->SetupAttachment(RootComponent);M->SetStaticMesh(Cube.Object);M->SetMaterial(0,Material);
  M->SetCollisionProfileName(bCollide?TEXT("BlockAll"):TEXT("NoCollision"));
  M->SetCanEverAffectNavigation(false);
  return M;
 };
 Stone=Group(TEXT("BoathouseStone"),StoneMat.Object,true);
 Siding=Group(TEXT("BoathouseSiding"),SidingMat.Object,true);
 Trim=Group(TEXT("BoathouseTrim"),TrimMat.Object,true);
 Openings=Group(TEXT("BoathouseBays"),DoorMat.Object,false);
 Deck=Group(TEXT("BoathouseDeck"),DeckMat.Object,true);
 Railings=Group(TEXT("BoathouseRailings"),RailMat.Object,true);
 Piles=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BoathousePiles"));
 Piles->SetupAttachment(RootComponent);Piles->SetStaticMesh(Cylinder.Object);Piles->SetMaterial(0,WoodMat.Object);
 Piles->SetCollisionProfileName(TEXT("NoCollision"));Piles->SetCanEverAffectNavigation(false);
 Roof=Group(TEXT("BoathouseRoof"),RoofMat.Object,false);
 Sign=CreateDefaultSubobject<UTextRenderComponent>(TEXT("BoathouseSign"));Sign->SetupAttachment(RootComponent);
 Sign->SetText(FText::FromString(TEXT("LAKE CLARA MEER\nBOATHOUSE")));Sign->SetWorldSize(34);
 Sign->SetHorizontalAlignment(EHTA_Center);Sign->SetVerticalAlignment(EVRTA_TextCenter);
 Sign->SetTextRenderColor(FColor(245,235,207));
 if(auto* Unlit=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/EngineMaterials/UnlitText.UnlitText")))Sign->SetTextMaterial(Unlit);
 Sign->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Tags={TEXT("BattleBoathouse")};
}

bool ABattleBoathouse::FindShore(FVector& OutShore,FVector& OutToWater,float& OutWaterZ) const {
 const APiedmontWaterHazard* Hazard=nullptr;
 for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->Polygon.Num()>=3){Hazard=*It;break;}
 if(!Hazard)return false;
 const FTransform Xf=Hazard->GetActorTransform();
 TArray<FVector> Ring;Ring.Reserve(Hazard->Polygon.Num());
 FVector Centre=FVector::ZeroVector;
 for(const FVector& Local:Hazard->Polygon){const FVector World=Xf.TransformPosition(Local);Ring.Add(World);Centre+=World;}
 Centre/=Ring.Num();
 // Walk the outline outward from the mapped dockside and take the first stretch
 // of shore where the hull really is on dry land and the dock really reaches
 // water. The lake outline is irregular, so the nearest vertex is not always a
 // usable building site.
 TArray<int32> Order;Order.Reserve(Ring.Num());
 for(int32 I=0;I<Ring.Num();++I)Order.Add(I);
 Order.Sort([&](int32 A,int32 B){return FVector::Dist2D(Ring[A],DocksideAnchor)<FVector::Dist2D(Ring[B],DocksideAnchor);});
 const float Site=Hazard->GetActorLocation().Z;
 auto OverWater=[&](const FVector& Point){return Hazard->ContainsBike(FVector(Point.X,Point.Y,Site));};
 for(int32 Index:Order){
  const FVector Point=Ring[Index];
  if(FVector::Dist2D(Point,DocksideAnchor)>9000.f)break;
  const FVector Previous=Ring[(Index+Ring.Num()-1)%Ring.Num()],Next=Ring[(Index+1)%Ring.Num()];
  FVector Normal=FVector::ZeroVector;
  const FVector Edges[]={(Next-Point).GetSafeNormal2D(),(Point-Previous).GetSafeNormal2D()};
  for(const FVector& Edge:Edges){
   FVector Candidate=FVector::CrossProduct(FVector::UpVector,Edge).GetSafeNormal();
   if(FVector::DotProduct(Candidate,Centre-Point)<0)Candidate=-Candidate;
   Normal+=Candidate;
  }
  Normal=Normal.GetSafeNormal2D();
  if(Normal.IsNearlyZero()||!OverWater(Point+Normal*2400.f))continue;
  if(OverWater(Point-Normal*1500.f))continue;
  OutShore=FVector(Point.X,Point.Y,0);OutToWater=Normal;
  OutWaterZ=Site;
  return true;
 }
 return false;
}

void ABattleBoathouse::BeginPlay(){
 Super::BeginPlay();
 FVector Shore,ToWater;float LakeZ=0;
 if(!FindShore(Shore,ToWater,LakeZ)){UE_LOG(LogTemp,Warning,TEXT("BattleBoathouse: no lake hazard found"));return;}
 // The hull sits inland of the water's edge, the dock reaches out over it.
 const FVector Hull=Shore-ToWater*1500.f;
 FHitResult Hit;FCollisionQueryParams Params(SCENE_QUERY_STAT(BoathouseGround),false,this);
 const bool bGround=GetWorld()->LineTraceSingleByChannel(Hit,Hull+FVector(0,0,400),Hull-FVector(0,0,900),ECC_Visibility,Params);
 const float Ground=bGround?Hit.ImpactPoint.Z:Shore.Z;
 SetActorLocation(FVector(Shore.X,Shore.Y,Ground));
 SetActorRotation(ToWater.Rotation());
 ShorePoint=Shore;WaterZ=LakeZ;GroundZ=Ground;bPlaced=true;
 Build();
 APiedmontWaterHazard* Test=nullptr;
 for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(!Test&&It->Polygon.Num()>=3)Test=*It;
 // The deck stands above the swim detection height, so containment is tested
 // horizontally at the water surface: that is what "the dock is out over the
 // lake" means here.
 auto OverWater=[&](const FVector& Point){return Test&&Test->ContainsBike(FVector(Point.X,Point.Y,WaterZ));};
 bDockOverWater=OverWater(DockTip);
 // The actor sits on the water's edge; the hull is the inland part of it.
 const FVector HullCentre=GetActorTransform().TransformPosition(FVector(-1150,0,0));
 bHullOnLand=Test&&!OverWater(HullCentre);
 UE_LOG(LogTemp,Display,TEXT("BattleBoathouse: shore=%s water_z=%.1f ground_z=%.1f dock_tip=%s dock_over_water=%s hull_on_land=%s"),
  *ShorePoint.ToString(),WaterZ,GroundZ,*DockTip.ToString(),bDockOverWater?TEXT("true"):TEXT("false"),bHullOnLand?TEXT("true"):TEXT("false"));
}

void ABattleBoathouse::Build(){
 // Local space: +X points out over the lake, +Y runs along the shore. The park
 // terrain is authored at one third of real scale, so the hull is sized to the
 // world rather than to the real boathouse.
 auto Box=[&](UInstancedStaticMeshComponent* Group,const FVector& Centre,const FVector& Size,const FRotator& Rotation=FRotator::ZeroRotator){
  Group->AddInstance(FTransform(Rotation,Centre,Size/100.f));
 };
 Box(Stone,FVector(-700,0,45),FVector(1150,850,90));
 Box(Siding,FVector(-700,-400,215),FVector(1150,45,300));
 Box(Siding,FVector(-700,400,215),FVector(1150,45,300));
 Box(Siding,FVector(-1250,0,215),FVector(45,850,300));
 for(float Y:{-400.f,0.f,400.f})Box(Siding,FVector(-200,Y,215),FVector(45,340,300));
 for(float Y:{-190.f,190.f})Box(Openings,FVector(-215,Y,180),FVector(35,300,225));
 // Gable ridge runs along X, so the slopes roll about X. Pitching them made the
 // roof read as detached slabs hovering over the hull.
 Box(Roof,FVector(-700,-210,450),FVector(1240,470,28),FRotator(0,0,28));
 Box(Roof,FVector(-700,210,450),FVector(1240,470,28),FRotator(0,0,-28));
 Box(Roof,FVector(-700,0,565),FVector(1260,80,24));
 Box(Trim,FVector(-700,0,350),FVector(1150,880,30));
 Box(Deck,FVector(-60,0,15),FVector(520,1250,30));
 Box(Deck,FVector(620,0,0),FVector(900,380,28));
 Box(Deck,FVector(1350,0,-25),FVector(620,700,28));
 for(float X:{450.f,900.f,1350.f})for(float Y:{-330.f,330.f})Box(Railings,FVector(X,Y,70),FVector(45,45,140));
 for(int Side:{-1,1})Box(Railings,FVector(900,Side*205,140),FVector(1000,30,30));
 for(int Side:{-1,1})Box(Railings,FVector(1350,Side*350,140),FVector(30,700,30));
 Box(Railings,FVector(1750,0,140),FVector(30,700,30));
 Sign->SetRelativeLocation(FVector(-720,0,585));Sign->SetRelativeRotation(FRotator(0,90,0));Sign->SetWorldSize(26);
 DockTip=GetActorLocation()+GetActorForwardVector()*2400.f;
}
