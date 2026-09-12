#include "BattleBenchFire.h"
#include "BattleParkFurniture.h"
#include "EngineUtils.h"
#include "Components/MaterialBillboardComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/CommandLine.h"
ABattleBenchFire::ABattleBenchFire(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("FireRoot"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> FireMat(TEXT("/Game/BattleForTheA/Effects/BenchFire/M_BenchFire.M_BenchFire"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> SmokeMat(TEXT("/Game/BattleForTheA/Effects/BenchFire/M_BenchSmoke.M_BenchSmoke"));
 for(int I=0;I<3;++I){
  auto* C=CreateDefaultSubobject<UMaterialBillboardComponent>(*FString::Printf(TEXT("Flame%d"),I));C->SetupAttachment(RootComponent);
  C->SetRelativeLocation(FVector((I-1)*48,0,90+(I%2)*14));C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCanEverAffectNavigation(false);
  C->AddElement(FireMat.Object,nullptr,false,43,65,nullptr);Flames.Add(C);
 }
 Smoke=CreateDefaultSubobject<UMaterialBillboardComponent>(TEXT("Smoke"));Smoke->SetupAttachment(RootComponent);Smoke->SetRelativeLocation(FVector(0,0,210));
 Smoke->SetCollisionEnabled(ECollisionEnabled::NoCollision);Smoke->SetCanEverAffectNavigation(false);Smoke->AddElement(SmokeMat.Object,nullptr,false,100,120,nullptr);
 Glow=CreateDefaultSubobject<UPointLightComponent>(TEXT("FireGlow"));Glow->SetupAttachment(RootComponent);Glow->SetRelativeLocation(FVector(0,0,95));
 Glow->SetLightColor(FLinearColor(1,.22,.025));Glow->SetAttenuationRadius(360);Glow->SetIntensity(700);Glow->SetCastShadows(false);
 Tags.Add(TEXT("BattleBenchFire"));
}
void ABattleBenchFire::BeginPlay(){
 Super::BeginPlay();SetLifeSpan(FMath::Max(1.f,Duration));
 for(auto C:Flames){
#if !UE_BUILD_SHIPPING
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleBenchFireSolidReview")))C->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial")));
  UE_LOG(LogTemp,Display,TEXT("BenchFlameState: visible=%d hidden=%d elements=%d location=%s material=%s"),C->IsVisible(),C->bHiddenInGame,C->Elements.Num(),*C->GetComponentLocation().ToString(),*GetNameSafe(C->GetMaterial(0)));
#endif
  auto* M=UMaterialInstanceDynamic::Create(C->GetMaterial(0),this);C->SetMaterial(0,M);FlameMaterials.Add(M);}
 SmokeMaterial=UMaterialInstanceDynamic::Create(Smoke->GetMaterial(0),this);Smoke->SetMaterial(0,SmokeMaterial);
}
void ABattleBenchFire::Tick(float Dt){
 Super::Tick(Dt);Age+=Dt;
 for(int I=0;I<FlameMaterials.Num();++I)FlameMaterials[I]->SetScalarParameterValue(TEXT("Frame"),FMath::Fmod(Age*24+I*11,36.f));
 if(SmokeMaterial)SmokeMaterial->SetScalarParameterValue(TEXT("Frame"),FMath::Fmod(Age*12,64.f));
 Glow->SetIntensity(650+100*FMath::Sin(Age*13)+65*FMath::Sin(Age*21));
}

ABattleBenchFire* ABattleBenchFire::IgniteBench(ABattleParkFurniture* Furniture,int32 Index){
 if(!IsValid(Furniture)||!Furniture->IsBenchAvailable(Index))return nullptr;
 UWorld* World=Furniture->GetWorld();int32 Active=0;
 for(TActorIterator<ABattleBenchFire> It(World);It;++It)if(!It->IsActorBeingDestroyed())++Active;
 if(Active>=2)return nullptr;
 const FTransform Transform=Furniture->Benches[Index];
 auto* Fire=World->SpawnActorDeferred<ABattleBenchFire>(StaticClass(),Transform,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
 if(!Fire)return nullptr;
 if(!Furniture->ReserveBench(Index,Fire)){Fire->Destroy();return nullptr;}
 Fire->ReservedFurniture=Furniture;Fire->ReservedBench=Index;Fire->FinishSpawning(Transform);return Fire;
}
void ABattleBenchFire::EndPlay(const EEndPlayReason::Type Reason){
 if(auto* Furniture=ReservedFurniture.Get())Furniture->ReleaseBench(ReservedBench,this);
 Super::EndPlay(Reason);
}
