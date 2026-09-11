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
 for(auto* M:{Concrete.Get(),Berm.Get()}){M->SetupAttachment(RootComponent);M->SetCollisionProfileName(TEXT("BlockAll"));M->SetCanEverAffectNavigation(true);}
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Iron(TEXT("/Game/BattleForTheA/Furniture/M_BenchFrame.M_BenchFrame"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 Metal=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CopingAndRails"));Metal->SetupAttachment(RootComponent);Metal->SetStaticMesh(Cylinder.Object);Metal->SetMaterial(0,Iron.Object);Metal->SetCollisionProfileName(TEXT("BlockAll"));
 auto* Panel=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SignBoard"));Panel->SetupAttachment(RootComponent);Panel->SetStaticMesh(Cube.Object);Panel->SetMaterial(0,Iron.Object);Panel->SetRelativeLocation(FVector(1815,780,850));Panel->SetRelativeScale3D(FVector(.12,6,1.8));Panel->SetCollisionProfileName(TEXT("BlockAll"));
 for(int I=0;I<2;I++){auto* Post=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("SignPost%d"),I));Post->SetupAttachment(RootComponent);Post->SetStaticMesh(Cube.Object);Post->SetMaterial(0,Iron.Object);Post->SetRelativeLocation(FVector(1810,600+I*360,740));Post->SetRelativeScale3D(FVector(.1,.1,2.5));Post->SetCollisionProfileName(TEXT("BlockAll"));}
 auto* Sign=CreateDefaultSubobject<UTextRenderComponent>(TEXT("SkateparkSign"));Sign->SetupAttachment(RootComponent);Sign->SetRelativeLocation(FVector(1823,780,850));Sign->SetText(FText::FromString(TEXT("FOURTH WARD\nSKATEPARK\nBOWLS / RAMPS / +30s")));Sign->SetWorldSize(38);Sign->SetHorizontalAlignment(EHTA_Center);Sign->SetVerticalAlignment(EVRTA_TextCenter);Sign->SetTextRenderColor(FColor(245,235,207));if(auto* Unlit=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/EngineMaterials/UnlitText.UnlitText")))Sign->SetTextMaterial(Unlit);Sign->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Tags={TEXT("RidePath"),TEXT("RideRamp"),TEXT("BattleSkatepark")};
}
void ABattleSkatepark::BeginPlay(){
 Super::BeginPlay();
 auto Tube=[&](FVector A,FVector B,float R){Metal->AddInstance(FTransform(FQuat::FindBetweenNormals(FVector::UpVector,(B-A).GetSafeNormal()),(A+B)*.5,FVector(R/50,R/50,(B-A).Size()/100)));};
 const FVector Bowls[]={FVector(-950,-350,0),FVector(-950,400,0)};const FVector2D Radius[]={FVector2D(650,600),FVector2D(600,550)};
 for(int Bowl=0;Bowl<2;Bowl++)for(int I=0;I<96;I++){const float A=I*2*PI/96,B=(I+1)*2*PI/96,M=(A+B)*.5;const FVector Mid=Bowls[Bowl]+FVector(Radius[Bowl].X*FMath::Cos(M),Radius[Bowl].Y*FMath::Sin(M),0),Other=Mid-Bowls[1-Bowl];if(FMath::Square(Other.X/Radius[1-Bowl].X)+FMath::Square(Other.Y/Radius[1-Bowl].Y)<1)continue;Tube(Bowls[Bowl]+FVector(Radius[Bowl].X*FMath::Cos(A),Radius[Bowl].Y*FMath::Sin(A),652),Bowls[Bowl]+FVector(Radius[Bowl].X*FMath::Cos(B),Radius[Bowl].Y*FMath::Sin(B),652),3);}
 for(int Side:{-1,1}){Tube(FVector(350,Side*900,720),FVector(1250,Side*900,720),3.5);for(int X:{400,800,1200})Tube(FVector(X,Side*900,650),FVector(X,Side*900,720),3);}
 if(TActorIterator<ABattleParkFurniture> It(GetWorld());It)for(int X:{-150,1000})It->AddBench(FTransform(FRotator(0,180,0),GetActorLocation()+FVector(X,1150,650)));
 for(int I=0;I<2;I++){const FVector Local=I==0?FVector(-1450,-1050,738):FVector(1450,1050,738);const FTransform T(FRotator(0,I==0?0:180,0),GetActorLocation()+Local);auto* S=GetWorld()->SpawnActorDeferred<ABattleSkater>(ABattleSkater::StaticClass(),T,this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);if(S){S->RouteIndex=I==0?1:5;S->PhaseOffset=I*1.6f;S->FinishSpawning(T);}}
 for(FVector Local:{FVector(-200,850,738),FVector(200,820,738),FVector(-1750,0,738)})if(auto* P=GetWorld()->SpawnActor<APiedmontPedestrian>(GetActorLocation()+Local,FRotator(0,-90,0))){P->PauseRemaining=3600;P->Tags.Add(TEXT("SkateparkSpectator"));}

 for(FVector Local:{FVector(-950,-350,510),FVector(-950,400,550),FVector(1100,-500,990)}){
  const FTransform T(GetActorTransform().TransformPosition(Local));auto* P=GetWorld()->SpawnActorDeferred<ABattleColaPickup>(ABattleColaPickup::StaticClass(),T,this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  if(P){P->bTimeBonus=true;P->Tags.Add(TEXT("SkateparkBonus"));P->FinishSpawning(T);Bonuses.Add(P);}
 }
 UE_LOG(LogTemp,Display,TEXT("BattleSkatepark: bonuses=%d location=%s"),Bonuses.Num(),*GetActorLocation().ToString());
}
