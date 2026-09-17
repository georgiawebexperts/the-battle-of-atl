#include "BattleHints.h"
#include "BattleRider.h"
#include "BattleBike.h"
#include "BattleCheckpoints.h"
#include "BattleHomeData.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

/** Riding the grass up on the BeltLine stretch calls out the Trees ATL crew. */
ABattleGrassWatch::ABattleGrassWatch(){
 PrimaryActorTick.bCanEverTick=true;Tags.Add(TEXT("BattleGrassWatch"));
}
void ABattleGrassWatch::BeginPlay(){Super::BeginPlay();}
void ABattleGrassWatch::Tick(float Dt){
 Super::Tick(Dt);
 auto* Bike=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(this,0));
 if(!Bike){auto* Person=Cast<ABattleRider>(UGameplayStatics::GetPlayerPawn(this,0));if(Person)Bike=Person->ParkedBike;}
 for(int32 I=Crew.Num()-1;I>=0;--I){
  auto* P=Crew[I].Actor.Get();Crew[I].Remaining-=Dt;
  if(!P||Crew[I].Remaining<=0||P->IsActorBeingDestroyed()){if(P)P->Destroy();Crew.RemoveAt(I);continue;}
  if(!Bike)continue;
  const FVector To=Bike->GetActorLocation()-P->GetActorLocation();
  const FVector Flat(To.X,To.Y,0);
  if(Flat.SizeSquared2D()>90.f*90.f){
   const FVector Step=Flat.GetSafeNormal2D()*FMath::Min(520.f*Dt,Flat.Size2D());
   P->SetActorLocation(P->GetActorLocation()+Step,true);
   P->SetActorRotation(FRotator(0,Flat.Rotation().Yaw,0));
  }
 }
 Pursuers=Crew.Num();
 if(!Bike||!Bike->Ride)return;
 const bool bBeltLine=Bike->GetActorLocation().Y>20000.f;
 const bool bOnGrass=Bike->Ride->bGrass&&Bike->Ride->Speed>250&&!Bike->bParked;
 Cooldown-=Dt;
 if(!bBeltLine||!bOnGrass||Cooldown>0||Crew.Num()>=2)return;
 Cooldown=9.f;++Chases;
 if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this)))
  Mode->PushHint(TEXT("treesatl"),TEXT("TREES ATL — KEEP OFF THE GRASS"),5.f);
 for(int32 I=0;I<2;++I){
  const FVector Side=FVector::CrossProduct(FVector::UpVector,Bike->GetActorForwardVector()).GetSafeNormal();
  const FVector Spot=Bike->GetActorLocation()-Bike->GetActorForwardVector()*(700.f+I*220.f)+Side*(I==0?-260.f:260.f)+FVector(0,0,98.f);
  auto* P=GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),
   FTransform((Bike->GetActorLocation()-Spot).Rotation(),Spot),this,nullptr,
   ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
  if(!P)continue;
  P->PauseRemaining=100.f;
  P->FinishSpawning(FTransform((Bike->GetActorLocation()-Spot).Rotation(),Spot));
  Crew.Add({P,4.5f});
 }
}

/** Board on the way to the BeltLine: the crew is working on the grass. */
ABattleGrassSign::ABattleGrassSign(){
 PrimaryActorTick.bCanEverTick=false;Tags.Add(TEXT("BattleGrassSign"));
 Post=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SignPost"));RootComponent=Post;
 if(auto* Cylinder=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))Post->SetStaticMesh(Cylinder);
 if(auto* Metal=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/PiedmontRide/Materials/M_GunMetal.M_GunMetal")))Post->SetMaterial(0,Metal);
 Post->SetRelativeScale3D(FVector(0.14f,0.14f,2.0f));Post->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Board=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SignBoard"));Board->SetupAttachment(Post);
 if(auto* Cube=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")))Board->SetStaticMesh(Cube);
 if(auto* Wood=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Furniture/M_BenchWood.M_BenchWood")))Board->SetMaterial(0,Wood);
 Board->SetRelativeLocation(FVector(0,0,140));Board->SetRelativeScale3D(FVector(0.06f,1.5f,0.9f));
 Board->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Face=CreateDefaultSubobject<UTextRenderComponent>(TEXT("SignFace"));Face->SetupAttachment(Board);
 Face->SetText(FText::FromString(TEXT("TREES ATL\nWORKING ON THE GRASS\nPLEASE KEEP OFF")));
 Face->SetWorldSize(26.f);Face->SetTextRenderColor(FColor(250,246,232));
 Face->SetHorizontalAlignment(EHTA_Center);Face->SetRelativeLocation(FVector(0,-16,0));Face->SetRelativeRotation(FRotator(0,180,0));
 Message=TEXT("TREES ATL working on the grass, please keep off");
}
void ABattleGrassSign::BeginPlay(){
 Super::BeginPlay();
 // Sit beside the route on the stretch before the BeltLine.
 const FVector Gate=BattleHomeData::Gate;
 const auto& Krog=BattleCheckpoints::Anchors[1];
 const FVector Target((float)Krog.X,(float)Krog.Y,Gate.Z);
 const FVector Dir=(Target-Gate).GetSafeNormal2D();
 const FVector Side=FVector::CrossProduct(FVector::UpVector,Dir).GetSafeNormal();
 const FVector Try=Gate+Dir*9000.f+Side*700.f;
 FHitResult Ground;FCollisionQueryParams Q;Q.bIgnoreTouches=true;
 Place=GetWorld()->LineTraceSingleByChannel(Ground,Try+FVector(0,0,1200),Try-FVector(0,0,2000),ECC_Visibility,Q)?Ground.ImpactPoint:Try;
 SetActorLocation(Place);
 SetActorRotation(FRotator(0,(-Dir).Rotation().Yaw,0));
}
