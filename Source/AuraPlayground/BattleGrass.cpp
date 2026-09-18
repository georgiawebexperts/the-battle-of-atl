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
#include "PiedmontPathSpline.h"
#include "BattleRoadCrossing.h"
#include "GameFramework/PlayerStart.h"
#include "Components/SplineComponent.h"
#include "EngineUtils.h"

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
 const bool bBeltLine=BattleGrassRules::IsBeltLine(Bike->GetActorLocation());
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
 // A plain root: the post carries a non-uniform scale, and children of a scaled
 // component inherit that scale. The board used to hang off the post, so it was
 // squashed to a sliver floating 2.8 m above the post instead of sitting on it.
 Root=CreateDefaultSubobject<USceneComponent>(TEXT("SignRoot"));RootComponent=Root;
 Post=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SignPost"));Post->SetupAttachment(Root);
 if(auto* Cylinder=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))Post->SetStaticMesh(Cylinder);
 if(auto* Metal=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/PiedmontRide/Materials/M_GunMetal.M_GunMetal")))Post->SetMaterial(0,Metal);
 // The engine cylinder is 100 cm tall and 100 cm across; stand it on the ground.
 Post->SetRelativeLocation(FVector(0,0,100));Post->SetRelativeScale3D(FVector(0.14f,0.14f,2.0f));Post->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Board=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SignBoard"));Board->SetupAttachment(Root);
 if(auto* Cube=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")))Board->SetStaticMesh(Cube);
 if(auto* Wood=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Furniture/M_BenchWood.M_BenchWood")))Board->SetMaterial(0,Wood);
 // 6 cm thick, 150 cm wide, 90 cm tall, centred at eye height on the post.
 Board->SetRelativeLocation(FVector(0,0,170));Board->SetRelativeScale3D(FVector(0.06f,1.5f,0.9f));
 Board->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Face=CreateDefaultSubobject<UTextRenderComponent>(TEXT("SignFace"));Face->SetupAttachment(Root);
 Face->SetText(FText::FromString(TEXT("TREES ATL\nWORKING ON THE GRASS\nPLEASE KEEP OFF")));
  // Just in front of the board, facing the same way the actor does, sized to fit.
 Face->SetWorldSize(15.f);Face->SetTextRenderColor(FColor(250,246,232));
 Face->SetHorizontalAlignment(EHTA_Center);Face->SetRelativeLocation(FVector(4,0,170));Face->SetRelativeRotation(FRotator::ZeroRotator);
 Message=TEXT("TREES ATL working on the grass, please keep off");
}
void ABattleGrassSign::BeginPlay(){
 Super::BeginPlay();
 // Sit beside the route on the park side of the BeltLine, where a rider
 // heading for Monroe reads it before they cross - not at the finish gate,
 // which is where this used to end up, 90 m short of home on the wrong end
 // of the whole route.
 FVector Target=BattleHomeData::Gate;bool bHaveCrossing=false;
 for(TActorIterator<ABattleRoadCrossing> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("MonroeCrossing"))){Target=It->GetActorLocation();bHaveCrossing=true;break;}
 FVector ParkStart=Target;
 if(TActorIterator<APlayerStart> It(GetWorld());It)ParkStart=It->GetActorLocation();
 // Stand 85 m short of the crossing on the line the rider actually rides in on.
 const FVector Approach=(Target-ParkStart).GetSafeNormal2D();
 const FVector Across=FVector::CrossProduct(FVector::UpVector,Approach).GetSafeNormal();
 if(bHaveCrossing&&!Approach.IsNearlyZero()){
  const FVector Shoulder=Target-Approach*8500.f+Across*700.f;
  // Snap that point onto the nearest rideable route so the board stands at the
  // roadside, not out in the grass where nobody rides past it.
  float NearestRoute=BIG_NUMBER,HalfWidth=250.f;FVector OnRoute=Shoulder,Travel=Approach;
  for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It){
   auto* S=It->Centerline.Get();if(!S)continue;
   const float Length=S->GetSplineLength();
   for(float D=0.f;D<=Length;D+=200.f){
    const FVector P=S->GetLocationAtDistanceAlongSpline(D,ESplineCoordinateSpace::World);
    const float Distance=FVector::Dist2D(P,Shoulder);
    if(Distance<NearestRoute){NearestRoute=Distance;OnRoute=P;HalfWidth=It->WidthCm*.5f;Travel=S->GetDirectionAtDistanceAlongSpline(D,ESplineCoordinateSpace::World).GetSafeNormal2D();}
   }
  }
  FHitResult Ground;FCollisionQueryParams Q;Q.bIgnoreTouches=true;
  FVector Roadside=OnRoute+FVector::CrossProduct(FVector::UpVector,Travel).GetSafeNormal()*(HalfWidth+180.f);
  if(!GetWorld()->LineTraceSingleByChannel(Ground,Roadside+FVector(0,0,1500),Roadside-FVector(0,0,2500),ECC_Visibility,Q))
   Roadside=OnRoute+FVector::CrossProduct(FVector::UpVector,Travel).GetSafeNormal()*-(HalfWidth+180.f);
  UE_LOG(LogTemp,Display,TEXT("GrassSignPlacement: crossing=%s route_distance_cm=%.0f place=%s"),*Target.ToString(),NearestRoute,*Roadside.ToString());
  if(NearestRoute<4000.f){
   Place=GetWorld()->LineTraceSingleByChannel(Ground,Roadside+FVector(0,0,1500),Roadside-FVector(0,0,2500),ECC_Visibility,Q)?Ground.ImpactPoint:Roadside;
   SetActorLocation(Place);
   SetActorRotation(FRotator(0,(-Travel).Rotation().Yaw,0));
   return;
  }
  UE_LOG(LogTemp,Warning,TEXT("GrassSignPlacement: nearest route is %.0f cm away; falling back"),NearestRoute);
 }else UE_LOG(LogTemp,Warning,TEXT("GrassSignPlacement: no Monroe crossing found; falling back"));
 UE_LOG(LogTemp,Warning,TEXT("GrassSignPlacement: no route spline within reach of %s; falling back"),*Target.ToString());
 // Fallback: the old placement relative to the park gate.
 const FVector Gate=BattleHomeData::Gate;
 const auto& Krog=BattleCheckpoints::Anchors[1];
 const FVector FallbackTarget((float)Krog.X,(float)Krog.Y,Gate.Z);
 const FVector FallbackDir=(FallbackTarget-Gate).GetSafeNormal2D();
 const FVector FallbackSide=FVector::CrossProduct(FVector::UpVector,FallbackDir).GetSafeNormal();
 const FVector FallbackTry=Gate+FallbackDir*9000.f+FallbackSide*700.f;
 FHitResult FallbackGround;FCollisionQueryParams FallbackQuery;FallbackQuery.bIgnoreTouches=true;
 Place=GetWorld()->LineTraceSingleByChannel(FallbackGround,FallbackTry+FVector(0,0,1200),FallbackTry-FVector(0,0,2000),ECC_Visibility,FallbackQuery)?FallbackGround.ImpactPoint:FallbackTry;
 SetActorLocation(Place);
 SetActorRotation(FRotator(0,(-FallbackDir).Rotation().Yaw,0));
}
