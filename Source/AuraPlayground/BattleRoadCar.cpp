#include "BattleRoadCar.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

ABattleRoadCar::ABattleRoadCar(){
 PrimaryActorTick.bCanEverTick=true;
 Collision=CreateDefaultSubobject<UBoxComponent>(TEXT("CarCollision"));SetRootComponent(Collision);
 Collision->SetBoxExtent(FVector(236,114,55));Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
 Body=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));Body->SetupAttachment(Collision);
 Glass=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Glass"));Glass->SetupAttachment(Collision);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> B(TEXT("/Game/Vehicles/SportsCar/SM_SportsCar.SM_SportsCar"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> G(TEXT("/Game/Vehicles/SportsCar/SM_SportsCar_Glass.SM_SportsCar_Glass"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> W(TEXT("/Game/Vehicles/SportsCar/SM_SportsCar_Wheel.SM_SportsCar_Wheel"));
 Body->SetStaticMesh(B.Object);Glass->SetStaticMesh(G.Object);
 for(auto* Part:{Body,Glass}){Part->SetRelativeLocation(FVector(-12,0,-59));Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);}
 for(int32 I=0;I<4;++I){auto* Wheel=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Wheel%d"),I));Wheel->SetupAttachment(Collision);Wheel->SetStaticMesh(W.Object);Wheel->SetCollisionEnabled(ECollisionEnabled::NoCollision);Wheels.Add(Wheel);}
}
FVector ABattleRoadCar::SampleRoute(float D) const {
 for(int32 I=1;I<Lengths.Num();++I)if(D<=Lengths[I])return FMath::Lerp(Route[I-1],Route[I],FMath::Clamp((D-Lengths[I-1])/(Lengths[I]-Lengths[I-1]),0.f,1.f));
 return Route.Last();
}
bool ABattleRoadCar::GroundPose(FVector Point,FVector Direction,FTransform& Pose,TArray<FVector>& Contacts) const {
 Direction.Z=0;Direction.Normalize();const FVector Right(-Direction.Y,Direction.X,0);
 FCollisionQueryParams Q(SCENE_QUERY_STAT(RoadCarGround),true,this);
 FCollisionObjectQueryParams Objects;Objects.AddObjectTypesToQuery(ECC_WorldStatic);
 for(int32 I=0;I<4;++I){const FVector XY=Point+Direction*(I<2?123.f:-141.2f)+Right*(I%2?90.f:-90.f);FHitResult Hit;
  if(!GetWorld()->LineTraceSingleByObjectType(Hit,XY+FVector(0,0,250),XY-FVector(0,0,250),Objects,Q)||Hit.ImpactNormal.Z<.9f)return false;
  Contacts.Add(Hit.ImpactPoint);
 }
 const FVector Front=(Contacts[0]+Contacts[1])*.5,Back=(Contacts[2]+Contacts[3])*.5;
 const FVector Across=((Contacts[1]+Contacts[3])-(Contacts[0]+Contacts[2])).GetSafeNormal();
 const FVector Up=FVector::CrossProduct((Front-Back).GetSafeNormal(),Across).GetSafeNormal();
 if(Up.Z<.9f)return false;
 // Root is the collision-box centre; visuals retain the template body's origin.
 const FVector Ground=(Front+Back)*.5+Direction*9.1f;
 Pose=FTransform(FRotationMatrix::MakeFromZX(Up,Direction).ToQuat(),Ground+Up*73.3f);
 return true;
}
void ABattleRoadCar::UpdateWheels(const TArray<FVector>& Contacts,float Travel,float Steering){
 WheelAngle=FMath::Fmod(WheelAngle+FMath::RadiansToDegrees(Travel/39.2669f),360.f);
 for(int32 I=0;I<4;++I){const bool Left=I%2==0;Wheels[I]->SetRelativeLocation(GetActorTransform().InverseTransformPosition(Contacts[I])+FVector(0,0,39.2669));Wheels[I]->SetRelativeRotation(FRotator(Left?WheelAngle:-WheelAngle,(Left?180.f:0.f)+(I<2?Steering:0.f),0));}
}
bool ABattleRoadCar::StartRoute(){
 bStarted=false;Speed=0;DistanceTravelled=0;RouteDistance=0;WheelAngle=0;bRouteFinished=false;Lengths.Reset();
 if(Route.Num()<2)return false;
 Lengths.Add(0);for(int32 I=1;I<Route.Num();++I){const float L=FVector::Dist2D(Route[I],Route[I-1]);if(L<1)return false;Lengths.Add(Lengths.Last()+L);}
 FTransform Pose;TArray<FVector> Contacts;bGrounded=GroundPose(Route[0],Route[1]-Route[0],Pose,Contacts);if(!bGrounded)return false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(RoadCarSpawn),false,this);
 if(GetWorld()->OverlapBlockingTestByChannel(Pose.GetLocation(),Pose.GetRotation(),ECC_WorldDynamic,FCollisionShape::MakeBox(Collision->GetUnscaledBoxExtent()),Q))return false;
 SetActorTransform(Pose);UpdateWheels(Contacts,0,0);bStarted=true;return true;
}
void ABattleRoadCar::Tick(float Dt){
 Super::Tick(Dt);if(!bStarted||bRouteFinished||Dt<=0)return;
 // Bound movement steps during hitches; every step sweeps the whole car body.
 float Remaining=FMath::Min(Dt,.25f);
 while(Remaining>SMALL_NUMBER){const float Step=FMath::Min(Remaining,1.f/60.f);Remaining-=Step;
  const float StopDistance=Speed*Speed/(2*900.f)+100.f;
  FHitResult Ahead;FCollisionQueryParams Q(SCENE_QUERY_STAT(RoadCarAhead),false,this);
  bObstacleAhead=GetWorld()->SweepSingleByChannel(Ahead,GetActorLocation(),GetActorLocation()+GetActorForwardVector()*StopDistance,GetActorQuat(),ECC_WorldDynamic,FCollisionShape::MakeBox(FVector(236,114,45)),Q);
  LastObstacle=Ahead.GetActor()?Ahead.GetActor()->GetName():TEXT("");
  const float ToEnd=Lengths.Last()-RouteDistance;
  const float Target=bObstacleAhead?0.f:FMath::Min(CruiseSpeed,FMath::Sqrt(2*900.f*FMath::Max(0.f,ToEnd-2.f)));
  Speed=FMath::FInterpConstantTo(Speed,Target,Step,Target<Speed?900.f:250.f);
  const float Travel=FMath::Min(ToEnd,Speed*Step);const FVector Next=SampleRoute(RouteDistance+Travel);
  FVector Heading=SampleRoute(FMath::Min(Lengths.Last(),RouteDistance+Travel+150))-Next;if(Heading.IsNearlyZero())Heading=GetActorForwardVector();
  FTransform Pose;TArray<FVector> Contacts;bGrounded=GroundPose(Next,Heading,Pose,Contacts);if(!bGrounded){Speed=0;break;}
  const float Turn=FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw,Pose.Rotator().Yaw);
  FHitResult Hit;SetActorLocationAndRotation(Pose.GetLocation(),Pose.GetRotation(),true,&Hit);
  if(Hit.bBlockingHit){LastObstacle=FString(TEXT("movement:"))+(Hit.GetActor()?Hit.GetActor()->GetName():TEXT("unknown"));Speed=0;break;}
  RouteDistance+=Travel;DistanceTravelled+=Travel;UpdateWheels(Contacts,Travel,FMath::Clamp(Turn*12.f,-25.f,25.f));
  if(ToEnd<3){bRouteFinished=true;Speed=0;break;}
 }
}
