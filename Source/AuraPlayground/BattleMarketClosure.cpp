#include "BattleMarketClosure.h"
#include "BattleBike.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
ABattleMarketClosure::ABattleMarketClosure(){
 PrimaryActorTick.bCanEverTick=true;
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("MarketClosureRoot"));Tags.Add(TEXT("RideBarrier"));Tags.Add(TEXT("MarketSetupClosure"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Metal(TEXT("/Game/BattleForTheA/Furniture/M_BenchFrame.M_BenchFrame")),Timber(TEXT("/Game/BattleForTheA/Furniture/M_BenchWood.M_BenchWood")),Red(TEXT("/Game/BattleForTheA/Materials/M_ColaRed.M_ColaRed")),Text(TEXT("/Engine/EngineMaterials/UnlitText.UnlitText"));
 auto Group=[&](const TCHAR* Name,UMaterialInterface* Material){auto* M=CreateDefaultSubobject<UInstancedStaticMeshComponent>(Name);M->SetupAttachment(RootComponent);M->SetStaticMesh(Cube.Object);M->SetMaterial(0,Material);M->SetCollisionProfileName(TEXT("BlockAll"));return M;};
 auto* Frame=Group(TEXT("MarketFence"),Metal.Object);auto* Feet=Group(TEXT("MarketFeet"),Timber.Object);auto* Board=Group(TEXT("MarketClosureBoard"),Red.Object);
 auto Part=[](UInstancedStaticMeshComponent* M,FVector P,FVector Size){M->AddInstance(FTransform(FQuat::Identity,P,Size/100));};
 // Visible welded mesh, with gaps smaller than the player capsule diameter.
 for(int Y=-800;Y<=800;Y+=40)Part(Frame,FVector(0,Y,180),FVector(10,6,360));
 for(int Z=20;Z<=360;Z+=40)Part(Frame,FVector(0,0,Z),FVector(10,1640,5));
 for(int Y:{-800,-400,0,400,800}){Part(Frame,FVector(0,Y,180),FVector(16,16,360));Part(Feet,FVector(0,Y,12),FVector(120,70,24));}
 Part(Board,FVector(-14,0,220),FVector(18,720,150));
 auto Label=[&](const TCHAR* Name,const TCHAR* Words,float Z,float Size){auto* L=CreateDefaultSubobject<UTextRenderComponent>(Name);L->SetupAttachment(RootComponent);L->SetRelativeLocation(FVector(-25,0,Z));L->SetRelativeRotation(FRotator(0,180,0));L->SetWorldSize(Size);L->SetHorizontalAlignment(EHTA_Center);L->SetText(FText::FromString(Words));L->SetTextMaterial(Text.Object);};
 Label(TEXT("MarketSetup"),TEXT("FARMERS MARKET SETUP"),255,34);Label(TEXT("UseGate"),TEXT("PARK ENTRY: 14TH STREET"),199,29);
}
void ABattleMarketClosure::BeginPlay(){
 Super::BeginPlay();FHitResult Floor;FCollisionQueryParams Q;Q.AddIgnoredActor(this);
 const FVector P=GetActorLocation();
 if(GetWorld()->LineTraceSingleByChannel(Floor,P+FVector(0,0,1000),P-FVector(0,0,1000),ECC_Visibility,Q))SetActorLocation(Floor.ImpactPoint,false,nullptr,ETeleportType::TeleportPhysics);
}

void ABattleMarketClosure::Tick(float Dt){
 Super::Tick(Dt);
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleMarketImpactAudit"))){extern void TickMarketImpactAudit(ABattleMarketClosure*,float);TickMarketImpactAudit(this,Dt);}
#endif
 if(const auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));Mode&&!Mode->bTutorialActive){
  UE_LOG(LogTemp,Display,TEXT("MarketClosure: tutorial ended; clearing entrance"));
  Destroy();
 }
}
