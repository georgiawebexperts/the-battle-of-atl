#include "BattleMarketClosure.h"
#include "BattleMarketReturn.h"
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
 auto* Collision=Group(TEXT("MarketCollision"),Metal.Object);Collision->SetVisibility(false,true);Collision->SetHiddenInGame(true,true);
 auto Part=[](UInstancedStaticMeshComponent* M,FVector P,FVector Size){M->AddInstance(FTransform(FQuat::Identity,P,Size/100));};
 // Human-scale temporary fencing keeps the 12th Street setup closed without
 // obscuring the park. Gaps remain smaller than the player capsule diameter.
 constexpr float FenceTop=190.f;
 Part(Collision,FVector(0,0,180),FVector(20,1640,360));
 for(int Y=-800;Y<=800;Y+=40)Part(Frame,FVector(0,Y,FenceTop*.5f),FVector(10,6,FenceTop));
 for(float Z:{20.f,70.f,120.f,FenceTop-10.f})Part(Frame,FVector(0,0,Z),FVector(10,1640,5));
 for(int Y:{-800,-400,0,400,800}){Part(Frame,FVector(0,Y,FenceTop*.5f),FVector(16,16,FenceTop));Part(Feet,FVector(0,Y,12),FVector(120,70,24));}
 Part(Board,FVector(-14,0,123),FVector(18,720,105));
 auto Label=[&](const TCHAR* Name,const TCHAR* Words,float Z,float Size){auto* L=CreateDefaultSubobject<UTextRenderComponent>(Name);L->SetupAttachment(RootComponent);L->SetRelativeLocation(FVector(-25,0,Z));L->SetRelativeRotation(FRotator(0,180,0));L->SetWorldSize(Size);L->SetHorizontalAlignment(EHTA_Center);L->SetText(FText::FromString(Words));L->SetTextMaterial(Text.Object);};
 Label(TEXT("MarketSetup"),TEXT("FARMERS MARKET SETUP"),145,25);Label(TEXT("UseGate"),TEXT("PARK ENTRY: 14TH STREET"),108,20);
}
void ABattleMarketClosure::BeginPlay(){
 Super::BeginPlay();BuildGroundedReturn();
}
bool ABattleMarketClosure::BuildGroundedReturn(){
 if(bReturnBuilt)return true;
 FHitResult Floor;FCollisionQueryParams Q;Q.AddIgnoredActor(this);
 const FVector P=GetActorLocation();
 if(GetWorld()->LineTraceSingleByChannel(Floor,P+FVector(0,0,1000),P-FVector(0,0,1000),ECC_Visibility,Q))SetActorLocation(Floor.ImpactPoint,false,nullptr,ETeleportType::TeleportPhysics);
 auto* Mesh=Cast<UInstancedStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("MarketFence")));
 auto* Collision=Cast<UInstancedStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("MarketCollision")));if(!Mesh||!Collision)return false;
 TArray<FVector> Ground;
 for(const FVector2D XY:BattleMarketReturn::Points){
  FHitResult Hit;const FVector Probe(XY,0);
  if(!GetWorld()->LineTraceSingleByChannel(Hit,Probe+FVector(0,0,1500),Probe-FVector(0,0,1500),ECC_Visibility,Q)){UE_LOG(LogTemp,Error,TEXT("MarketReturn: missing ground"));return false;}
  Ground.Add(Hit.ImpactPoint-GetActorLocation());
 }
 ReturnGround.Reset();for(const FVector Base:Ground)ReturnGround.Add(Base+GetActorLocation());
 constexpr float FenceTop=190.f;
 for(const FVector Base:Ground)Mesh->AddInstance(FTransform(FQuat::Identity,Base+FVector(0,0,FenceTop*.5f),FVector(.10,.06,FenceTop/100.f)));
 for(int I=1;I<Ground.Num();I++){
  const FVector A=Ground[I-1],B=Ground[I],D=B-A;
  Collision->AddInstance(FTransform(D.Rotation(),(A+B)*.5+FVector(0,0,180),FVector((D.Size()+8)/100,.20,3.6)));
  for(float Z:{20.f,70.f,120.f,FenceTop-10.f})Mesh->AddInstance(FTransform(D.Rotation(),(A+B)*.5+FVector(0,0,Z),FVector((D.Size()+8)/100,.06,.05)));
 }
 UE_LOG(LogTemp,Display,TEXT("MarketReturn: grounded posts=%d"),Ground.Num());
 bReturnBuilt=true;return true;

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
