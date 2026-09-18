#include "BattleDuck.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "PiedmontBike.h"
#include "PiedmontExplorer.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ABattleDuck::ABattleDuck(){
 PrimaryActorTick.bCanEverTick=true;InitialLifeSpan=0;Tags.Add(TEXT("BattleDuck"));
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("DuckRoot"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Feather(TEXT("/Game/BattleForTheA/Furniture/M_BenchWood.M_BenchWood"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> BeakMat(TEXT("/Game/PiedmontRide/Materials/M_Safety.M_Safety"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Dark(TEXT("/Game/PiedmontRide/Materials/M_Asphalt.M_Asphalt"));
 auto Part=[&](const TCHAR* Name,UStaticMesh* Mesh,UMaterialInterface* Material,const FVector& Location,const FVector& Scale,const FRotator& Rotation=FRotator::ZeroRotator){
  auto* M=CreateDefaultSubobject<UStaticMeshComponent>(Name);M->SetupAttachment(RootComponent);M->SetStaticMesh(Mesh);
  M->SetRelativeLocation(Location);M->SetRelativeScale3D(Scale);M->SetRelativeRotation(Rotation);M->SetMaterial(0,Material);
  M->SetCollisionEnabled(ECollisionEnabled::QueryOnly);M->SetCollisionResponseToAllChannels(ECR_Ignore);
  M->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);M->SetCanEverAffectNavigation(false);
  return M;
 };
 Body=Part(TEXT("DuckBody"),Sphere.Object,Feather.Object,FVector::ZeroVector,FVector(.42,.24,.22));
 Head=Part(TEXT("DuckHead"),Sphere.Object,Feather.Object,FVector(20,0,13),FVector(.13,.12,.13));
 Beak=Part(TEXT("DuckBeak"),Cone.Object,BeakMat.Object,FVector(30,0,12),FVector(.05,.05,.11),FRotator(0,0,-90));
 Tail=Part(TEXT("DuckTail"),Cube.Object,Dark.Object,FVector(-19,0,5),FVector(.16,.10,.07),FRotator(0,0,35));
}

void ABattleDuck::FlyTo(const FVector& WaterPoint,float InDelay){
 Target=WaterPoint;Delay=InDelay;State=0;Clock=0;
 SetActorLocation(Target+FVector(FMath::FRandRange(-2600.f,2600.f),FMath::FRandRange(-2600.f,2600.f),FMath::FRandRange(1400.f,2200.f)));
 Drift=FMath::FRandRange(0.f,6.28f);
 if(!Target.IsNearlyZero())BaseRotation=FRotator(0,(Target-GetActorLocation()).Rotation().Yaw,0);
}

float ABattleDuck::TakeDamage(float Amount,const FDamageEvent&,AController*,AActor*){
 if(!FMath::IsFinite(Amount)||Amount<=0)return 0;
 Destroy();
 return Amount;
}

void ABattleDuck::Tick(float Dt){
 Super::Tick(Dt);
 auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));if(!Mode||Mode->bRunEnded)return;
 BumpCooldown=FMath::Max(0.f,BumpCooldown-Dt);
 Clock+=Dt;
 if(Delay>0){Delay-=Dt;return;}
 auto* TargetPawn=UGameplayStatics::GetPlayerPawn(this,0);
 if(State==0){
  const FVector To=Target+FVector(0,0,22)-GetActorLocation();
  const float Distance=To.Size();
  if(Distance<45.f){State=1;Clock=0;SetActorLocation(FVector(Target.X,Target.Y,WaterZ+24));return;}
  const FVector Step=To.GetSafeNormal()*FMath::Min(Distance,900.f*Dt);
  SetActorLocation(GetActorLocation()+Step);
  SetActorRotation(Step.Rotation());
  return;
 }
 if(State==1){
  // Swimming: drift and bob, and get out of the way of a swimmer who crowds us.
  const float Bob=FMath::Sin(Clock*1.6f+Drift)*2.5f;
  FVector Position=GetActorLocation();
  FVector Wander(FMath::Cos(Clock*.35f+Drift),FMath::Sin(Clock*.35f+Drift),0);
  if(TargetPawn&&FVector::Dist2D(TargetPawn->GetActorLocation(),Position)<700.f){
   Wander+=FVector(TargetPawn->GetActorLocation()-Position).GetSafeNormal2D()*-1.2f;
  }
  Position+=Wander.GetSafeNormal()*38.f*Dt;
  if(FVector::Dist2D(Position,Target)>700.f)Position=FMath::Lerp(Position,Target,.6f*Dt);
  Position.Z=WaterZ+24+Bob;
  SetActorLocation(Position);
  SetActorRotation(FRotator(FMath::Sin(Clock*1.2f+Drift)*4.f,Wander.Rotation().Yaw,0));
  // Blundering into a duck ducks the swimmer under and costs time.
  if(TargetPawn&&BumpCooldown<=0){
   auto* Swimmer=Cast<APiedmontExplorer>(TargetPawn);
   if(Swimmer&&Swimmer->bSwimming&&FVector::Dist2D(Swimmer->GetActorLocation(),Position)<130.f){
    BumpCooldown=6.f;BumpsGiven++;
    const FVector WasAt=Swimmer->GetActorLocation();
    Swimmer->SetActorLocation(WasAt-FVector(0,0,150),false,nullptr,ETeleportType::TeleportPhysics);
    LastDunkDepthCm=FMath::Max(LastDunkDepthCm,float(WasAt.Z-Swimmer->GetActorLocation().Z));
    Mode->AdjustRunTime(-5.f,TEXT("DUCK!"));
    UE_LOG(LogTemp,Display,TEXT("BattleDuck: swimmer_ducked=1 duck=%s bumps=%d"),*GetName(),BumpsGiven);
    State=2;Clock=0;return;
   }
  }
  if(Clock>FMath::FRandRange(22.f,40.f)){State=2;Clock=0;}
  return;
 }
 // Climbing out: gain height, then pick a new spot on the water.
 const FVector Rise=FVector(FMath::FRandRange(-120.f,120.f),FMath::FRandRange(-120.f,120.f),FMath::Min(700.f,260.f+Clock*420.f));
 SetActorLocation(GetActorLocation()+Rise*Dt);
 SetActorRotation(Rise.Rotation());
 if(GetActorLocation().Z>WaterZ+900.f||Clock>9.f)FlyTo(FVector(Target.X+FMath::FRandRange(-1800.f,1800.f),Target.Y+FMath::FRandRange(-1800.f,1800.f),0),0);
}

ABattleDuckFlock::ABattleDuckFlock(){
 PrimaryActorTick.bCanEverTick=true;Tags.Add(TEXT("BattleDuckFlock"));
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("DuckFlockRoot"));
}

void ABattleDuckFlock::BeginPlay(){
 Super::BeginPlay();
 const APiedmontWaterHazard* Hazard=nullptr;
 for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->Polygon.Num()>=3){Hazard=*It;break;}
 if(!Hazard){UE_LOG(LogTemp,Warning,TEXT("BattleDuckFlock: no lake hazard found"));return;}
 WaterZ=Hazard->GetActorLocation().Z;
 // Scatter landing spots inside the outline, keeping away from the island.
 TArray<FVector> Ring;Ring.Reserve(Hazard->Polygon.Num());
 for(const FVector& Local:Hazard->Polygon)Ring.Add(Hazard->GetActorTransform().TransformPosition(Local));
 FVector Centre=FVector::ZeroVector;for(const FVector& Point:Ring)Centre+=Point;Centre/=Ring.Num();
 WaterCentre=Centre;
 for(int32 Try=0;Try<400&&Spots.Num()<8;++Try){
  const FVector Candidate=Centre+FVector(FMath::FRandRange(-3200.f,3200.f),FMath::FRandRange(-2600.f,2600.f),0);
  if(!Hazard->ContainsBike(FVector(Candidate.X,Candidate.Y,WaterZ)))continue;
  Spots.Add(Candidate);
 }
 LandingPoints=Spots.Num();
 for(int32 I=0;I<Spots.Num();++I){
  auto* Duck=GetWorld()->SpawnActor<ABattleDuck>(ABattleDuck::StaticClass(),FVector(Spots[I].X,Spots[I].Y,WaterZ+24),FRotator::ZeroRotator);
  if(!Duck)continue;
  Duck->WaterZ=WaterZ;Ducks.Add(Duck);Duck->FlyTo(Spots[I],float(I)*2.5f);
 }
 UE_LOG(LogTemp,Display,TEXT("BattleDuckFlock: ducks=%d landing_spots=%d water_z=%.1f centre=%s"),Ducks.Num(),LandingPoints,WaterZ,*Centre.ToString());
}

void ABattleDuckFlock::Tick(float Dt){
 Super::Tick(Dt);
 Ducks.RemoveAll([](const TObjectPtr<ABattleDuck>& Duck){return !IsValid(Duck);});
 // Keep a small raft alive even after the player shoots a few.
 SpawnClock+=Dt;
 if(Ducks.Num()<4&&SpawnClock>12.f&&LandingPoints>0){
  SpawnClock=0;
  const FVector Spot=Spots[FMath::RandHelper(Spots.Num())];
  if(auto* Duck=GetWorld()->SpawnActor<ABattleDuck>(ABattleDuck::StaticClass(),Spot+FVector(0,0,1400),FRotator::ZeroRotator)){Duck->WaterZ=WaterZ;Duck->FlyTo(Spot,FMath::FRandRange(0.f,4.f));Ducks.Add(Duck);}
 }
}
