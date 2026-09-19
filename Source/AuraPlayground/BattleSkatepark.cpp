#include "BattleSkatepark.h"
#include "BattlePickup.h"
#include "BattleSkater.h"
#include "BattleParkFurniture.h"
#include "EngineUtils.h"
#include "Components/InstancedStaticMeshComponent.h"
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
 auto* Paint=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RampWayfindingPaint"));Paint->SetupAttachment(RootComponent);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> PaintAsset(TEXT("/Game/BattleForTheA/Skatepark/SM_SkateMarkings.SM_SkateMarkings"));
 Paint->SetStaticMesh(PaintAsset.Object);Paint->SetCollisionEnabled(ECollisionEnabled::NoCollision);Paint->SetCanEverAffectNavigation(false);Paint->SetCastShadow(false);
 for(auto* M:{Concrete.Get(),Berm.Get()}){M->SetupAttachment(RootComponent);M->SetCollisionProfileName(TEXT("BlockAll"));M->SetCanEverAffectNavigation(true);}
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Iron(TEXT("/Game/BattleForTheA/Furniture/M_BenchFrame.M_BenchFrame"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 Metal=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CopingAndRails"));Metal->SetupAttachment(RootComponent);Metal->SetStaticMesh(Cylinder.Object);Metal->SetMaterial(0,Iron.Object);Metal->SetCollisionProfileName(TEXT("BlockAll"));
 auto* Panel=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SignBoard"));Panel->SetupAttachment(RootComponent);Panel->SetStaticMesh(Cube.Object);Panel->SetMaterial(0,Iron.Object);Panel->SetRelativeLocation(FVector(1815,780,850));Panel->SetRelativeScale3D(FVector(.12,6,1.8));Panel->SetCollisionProfileName(TEXT("BlockAll"));
 for(int I=0;I<2;I++){auto* Post=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("SignPost%d"),I));Post->SetupAttachment(RootComponent);Post->SetStaticMesh(Cube.Object);Post->SetMaterial(0,Iron.Object);Post->SetRelativeLocation(FVector(1810,600+I*360,740));Post->SetRelativeScale3D(FVector(.1,.1,2.5));Post->SetCollisionProfileName(TEXT("BlockAll"));}
 auto* Sign=CreateDefaultSubobject<UTextRenderComponent>(TEXT("SkateparkSign"));Sign->SetupAttachment(RootComponent);Sign->SetRelativeLocation(FVector(1823,780,850));Sign->SetText(FText::FromString(TEXT("FOURTH WARD\nSKATEPARK\nBOWLS / QUARTER / PUMP TRACK / +30s")));Sign->SetWorldSize(38);Sign->SetHorizontalAlignment(EHTA_Center);Sign->SetVerticalAlignment(EVRTA_TextCenter);Sign->SetTextRenderColor(FColor(245,235,207));if(auto* Unlit=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/EngineMaterials/UnlitText.UnlitText")))Sign->SetTextMaterial(Unlit);Sign->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Tags={TEXT("RidePath"),TEXT("RideRamp"),TEXT("BattleSkatepark")};
}
void ABattleSkatepark::BeginPlay(){
 Super::BeginPlay();
 auto Tube=[&](FVector A,FVector B,float R){Metal->AddInstance(FTransform(FQuat::FindBetweenNormals(FVector::UpVector,(B-A).GetSafeNormal()),(A+B)*.5,FVector(R/50,R/50,(B-A).Size()/100)));};
 // Coping around every bowl rim. The five bowls are the shape in
 // Scripts/skatepark_geometry.py; an arc that would run across the deck into a
 // neighbouring bowl is skipped rather than laid through the other bowl's wall.
 const FVector Bowls[]={FVector(-950,-350,0),FVector(-950,400,0),FVector(-2600,-1500,0),FVector(-3000,-2100,0),FVector(-1900,-2050,0)};
 const FVector2D Radius[]={FVector2D(650,600),FVector2D(600,550),FVector2D(640,470),FVector2D(280,250),FVector2D(460,330)};
 const int32 BowlCount=UE_ARRAY_COUNT(Bowls);
 for(int Bowl=0;Bowl<BowlCount;Bowl++)for(int I=0;I<96;I++){
  const float A=I*2*PI/96,B=(I+1)*2*PI/96,M=(A+B)*.5;
  const FVector Mid=Bowls[Bowl]+FVector(Radius[Bowl].X*FMath::Cos(M),Radius[Bowl].Y*FMath::Sin(M),0);
  bool bSkip=false;
  for(int Other=0;Other<BowlCount&&!bSkip;Other++)if(Other!=Bowl){const FVector D=Mid-Bowls[Other];bSkip=FMath::Square(D.X/Radius[Other].X)+FMath::Square(D.Y/Radius[Other].Y)<1.f;}
  if(bSkip)continue;
  Tube(Bowls[Bowl]+FVector(Radius[Bowl].X*FMath::Cos(A),Radius[Bowl].Y*FMath::Sin(A),652),Bowls[Bowl]+FVector(Radius[Bowl].X*FMath::Cos(B),Radius[Bowl].Y*FMath::Sin(B),652),3);
 }
 // A rail line either side of a launch bank: flat bar with posts underneath.
 auto Rail=[&](float Y,float X0,float X1,float Height){Tube(FVector(X0,Y,Height),FVector(X1,Y,Height),3.5);const int32 Posts=FMath::Max(2,FMath::RoundToInt((X1-X0)/400.f));for(int32 I=0;I<=Posts;I++){const float X=X0+(X1-X0)*I/Posts;Tube(FVector(X,Y,650),FVector(X,Y,Height),3);}};
 for(int Side:{-1,1})Rail(Side*900,350,1250,720);            // Fourth Ward launch bank, as installed
 for(int Side:{-1,1})Rail(800+Side*300,-3200,-2200,720);     // west launch bank, new
 // The quarter pipe's lip, and the south plaza ledge that feeds it.
 Tube(FVector(-300,-2400,1082),FVector(1500,-2400,1082),4);
 Rail(-1490,-200,900,780);
 if(TActorIterator<ABattleParkFurniture> It(GetWorld());It)for(const FVector Bench:{FVector(-1200,2100,650),FVector(900,2100,650),FVector(1700,-1150,650)})It->AddBench(FTransform(FRotator(0,180,0),GetActorLocation()+Bench));
 for(int I=0;I<2;I++){const FVector Local=I==0?FVector(-1450,-1050,738):FVector(1450,1050,738);const FTransform T(FRotator(0,I==0?0:180,0),GetActorLocation()+Local);auto* S=GetWorld()->SpawnActorDeferred<ABattleSkater>(ABattleSkater::StaticClass(),T,this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);if(S){S->RouteIndex=I==0?1:5;S->PhaseOffset=I*1.6f;S->FinishSpawning(T);}}
 for(FVector Local:{FVector(-200,850,738),FVector(200,820,738),FVector(-1750,0,738)})if(auto* P=GetWorld()->SpawnActor<APiedmontPedestrian>(GetActorLocation()+Local,FRotator(0,-90,0))){P->PauseRemaining=3600;P->Tags.Add(TEXT("SkateparkSpectator"));}

 for(FVector Local:{FVector(-950,-350,510),FVector(-950,400,550),FVector(1100,-500,990)}){
  const FTransform T(GetActorTransform().TransformPosition(Local));auto* P=GetWorld()->SpawnActorDeferred<ABattleColaPickup>(ABattleColaPickup::StaticClass(),T,this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  if(P){P->bTimeBonus=true;P->Tags.Add(TEXT("SkateparkBonus"));P->FinishSpawning(T);Bonuses.Add(P);}
 }
 UE_LOG(LogTemp,Display,TEXT("BattleSkatepark: bonuses=%d location=%s"),Bonuses.Num(),*GetActorLocation().ToString());
}
