#include "BattleRoadCar.h"
#include "BattleBike.h"
#include "BattleRoadCrossing.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/CommandLine.h"
#include "Kismet/GameplayStatics.h"
#include "BattleRider.h"
#include "Misc/Parse.h"
#include "Kismet/KismetSystemLibrary.h"

ABattleRoadCar::ABattleRoadCar(){
 PrimaryActorTick.bCanEverTick=true;Tags.Add(TEXT("RideVehicle"));
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
void ABattleRoadCar::BeginPlay(){Super::BeginPlay();ApplyPaint(PaintVariant<0?FMath::RandHelper(6):PaintVariant);}
void ABattleRoadCar::ApplyPaint(int32 Variant){
 static const FLinearColor Colors[]={FLinearColor(.65f,.68f,.72f),FLinearColor(.045f,.05f,.06f),FLinearColor(.035f,.14f,.32f),FLinearColor(.32f,.035f,.025f),FLinearColor(.06f,.18f,.11f),FLinearColor(.7f,.16f,.025f)};
 PaintVariant=FMath::Clamp(Variant,0,UE_ARRAY_COUNT(Colors)-1);
 for(int32 Slot=0;Slot<Body->GetNumMaterials();++Slot)if(auto* Paint=Body->CreateDynamicMaterialInstance(Slot))Paint->SetVectorParameterValue(TEXT("Paint Tint"),Colors[PaintVariant]);
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
 if(Travel>SMALL_NUMBER)SteeringAngle=FMath::Lerp(SteeringAngle,Steering,1.f-FMath::Exp(-12.f*Travel/FMath::Max(1.f,Speed)));
 WheelAngle=FMath::Fmod(WheelAngle+FMath::RadiansToDegrees(Travel/39.2669f),360.f);
 for(int32 I=0;I<4;++I){const bool Left=I%2==0;Wheels[I]->SetRelativeLocation(GetActorTransform().InverseTransformPosition(Contacts[I])+FVector(0,0,39.2669));Wheels[I]->SetRelativeRotation(FRotator(Left?WheelAngle:-WheelAngle,(Left?180.f:0.f)+(I<2?SteeringAngle:0.f),0));}
}
void ABattleRoadCar::EndPlay(const EEndPlayReason::Type Reason){
 for(auto& Gate:Crossings)if(IsValid(Gate.Crossing))Gate.Crossing->ReleaseVehicle(this);
 Super::EndPlay(Reason);
}
bool ABattleRoadCar::StartRoute(){
 bUseBodyHull=FParse::Param(FCommandLine::Get(),TEXT("BattleBodyHull"));
 if(bUseBodyHull){
  auto* Hull=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/BattleForTheA/Traffic/SM_RoadCarHull.SM_RoadCarHull"));if(!Hull)return false;
  Body->SetStaticMesh(Hull);Body->SetCollisionProfileName(TEXT("BlockAllDynamic"));Body->SetCollisionEnabled(ECollisionEnabled::QueryOnly);Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 }

 for(auto& Gate:Crossings)if(IsValid(Gate.Crossing))Gate.Crossing->ReleaseVehicle(this);
 bStarted=false;Speed=0;DistanceTravelled=0;RouteDistance=0;WheelAngle=0;SteeringAngle=0;bRouteFinished=false;Lengths.Reset();ClearedCrossings.Reset();AmberStopping.Reset();bWaitingForCrossing=false;
 CompletedLoops=0;
 if(Route.Num()<2)return false;
 if(bLoopRoute&&(Route.Num()<4||!Route[0].Equals(Route.Last(),.1f)||FVector::DotProduct((Route[1]-Route[0]).GetSafeNormal2D(),(Route.Last()-Route[Route.Num()-2]).GetSafeNormal2D())<.95f))return false;
 Lengths.Add(0);for(int32 I=1;I<Route.Num();++I){const float L=FVector::Dist2D(Route[I],Route[I-1]);if(L<1)return false;Lengths.Add(Lengths.Last()+L);}
 for(const auto& Gate:Crossings)if(!IsValid(Gate.Crossing)||Gate.Crossing->GetWorld()!=GetWorld()||Gate.StopDistance<0||Gate.StopDistance>=Lengths.Last())return false;
 FTransform Pose;TArray<FVector> Contacts;bGrounded=GroundPose(Route[0],Route[1]-Route[0],Pose,Contacts);if(!bGrounded)return false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(RoadCarSpawn),false,this);
 if(GetWorld()->OverlapBlockingTestByChannel(Pose.GetLocation(),Pose.GetRotation(),ECC_WorldDynamic,FCollisionShape::MakeBox(Collision->GetUnscaledBoxExtent()),Q))return false;
 SetActorTransform(Pose);UpdateWheels(Contacts,0,0);bStarted=true;return true;
}
void ABattleRoadCar::Tick(float Dt){
 Super::Tick(Dt);if(!bStarted||bRouteFinished||Dt<=0)return;
 // Traffic with a grudge: it steers at the rider whenever they are sharing the
 // car lanes. Riding the painted cycle track keeps the rider safe.
 float SwerveTarget=0.f;
 if(auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0)){
  auto* Bike=Cast<ABattleBike>(Pawn);
  if(!Bike)if(auto* Person=Cast<ABattleRider>(Pawn))Bike=Person->ParkedBike;
  const bool bSafe=Bike&&Bike->Ride&&Bike->Ride->bBikeLane;
  const FVector To=Pawn->GetActorLocation()-GetActorLocation();
  const FVector Fwd=GetActorForwardVector();
  const float Ahead=FVector::DotProduct(FVector(To.X,To.Y,0),FVector(Fwd.X,Fwd.Y,0));
  const float Side=FVector::DotProduct(FVector(To.X,To.Y,0),FVector(-Fwd.Y,Fwd.X,0));
  if(!bSafe&&Ahead>200.f&&Ahead<5400.f&&FMath::Abs(Side)<430.f)SwerveTarget=FMath::Clamp(Side,-140.f,140.f);
 }
 HostilityBlend=FMath::Lerp(HostilityBlend,SwerveTarget,1.f-FMath::Exp(-3.f*Dt));
 // Bound movement steps during hitches; every step sweeps the whole car body.
 float Remaining=FMath::Min(Dt,.25f);
 while(Remaining>SMALL_NUMBER){const float Step=FMath::Min(Remaining,1.f/60.f);Remaining-=Step;
  const float StopDistance=Speed*Speed/(2*900.f)+100.f;
  FHitResult Ahead;FCollisionQueryParams Q(SCENE_QUERY_STAT(RoadCarAhead),false,this);
  bObstacleAhead=GetWorld()->SweepSingleByChannel(Ahead,GetActorLocation(),GetActorLocation()+GetActorForwardVector()*StopDistance,GetActorQuat(),ECC_WorldDynamic,FCollisionShape::MakeBox(FVector(236,114,45)),Q);
  LastObstacle=Ahead.GetActor()?Ahead.GetActor()->GetName():TEXT("");
  const float ToEnd=Lengths.Last()-RouteDistance;
  float Available=bLoopRoute?ToEnd+Lengths.Last():ToEnd;bWaitingForCrossing=false;
  for(int32 I=0;I<Crossings.Num();++I){
   if(ClearedCrossings.Contains(I))continue;
   const auto& Gate=Crossings[I];const float Gap=Gate.StopDistance-RouteDistance;
   // On amber, commit only when comfortable braking cannot reach the line.
   const bool Amber=IsValid(Gate.Crossing)&&!Gate.Crossing->bVehicleGreen&&Gate.Crossing->bVehicleAmber;
   if(!Amber)AmberStopping.Remove(I);
   const bool AmberCommit=Amber&&!AmberStopping.Contains(I)&&Speed>1&&Gap<Speed*Speed/(2*900.f);
   if(Amber&&!AmberCommit)AmberStopping.Add(I);
   const bool Clear=IsValid(Gate.Crossing)&&Gate.Crossing->CanEnter(this,AmberCommit);
   if(Clear&&(AmberCommit||Gap<=Speed*Step+3.f)&&Gate.Crossing->TryReserve(this,AmberCommit)){ClearedCrossings.Add(I);continue;}
   if(!Clear){Available=FMath::Min(Available,FMath::Max(0.f,Gap));if(Gap<100)bWaitingForCrossing=true;}
  }
  const float Target=bObstacleAhead?0.f:FMath::Min(CruiseSpeed,FMath::Sqrt(2*900.f*FMath::Max(0.f,Available-2.f)));
  Speed=FMath::FInterpConstantTo(Speed,Target,Step,Target<Speed?900.f:250.f);
  const float Travel=FMath::Min(ToEnd,FMath::Min(Available,Speed*Step));const FVector Next=SampleRoute(RouteDistance+Travel);
  FVector Heading=SampleRoute(FMath::Min(Lengths.Last(),RouteDistance+Travel+150))-Next;if(Heading.IsNearlyZero())Heading=GetActorForwardVector();
  // The swerve that steers a car at the rider has to be part of the point the
  // suspension is solved from, not a shift applied to the finished pose.
  // GroundPose traces four contacts from the point it is handed and UpdateWheels
  // places each wheel at its own contact, so shifting the pose afterwards left
  // the body up to 140 cm from its own wheels - one side poking out of the
  // fenders like a detached wheel, the other buried under the floor.
  FVector Solve=Next;
  if(!FMath::IsNearlyZero(HostilityBlend))Solve+=FVector(-Heading.Y,Heading.X,0).GetSafeNormal()*HostilityBlend;
  FTransform Pose;TArray<FVector> Contacts;bGrounded=GroundPose(Solve,Heading,Pose,Contacts);if(!bGrounded){Speed=0;break;}
  const float Turn=FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw,Pose.Rotator().Yaw);
  // Steering follows curvature and wheelbase, not degrees rotated per rendered frame.
  const float WheelSteer=Travel>KINDA_SMALL_NUMBER?FMath::Clamp(FMath::RadiansToDegrees(FMath::Atan(264.2f*FMath::DegreesToRadians(Turn)/Travel)),-55.f,55.f):SteeringAngle;
  FHitResult Hit;
  if(bUseBodyHull){
   TArray<FHitResult> Hits;FComponentQueryParams Query(SCENE_QUERY_STAT(RoadCarBody),this);Query.bTraceComplex=false;
   const FVector EndBody=Pose.TransformPosition(Body->GetRelativeLocation());
   GetWorld()->ComponentSweepMulti(Hits,Body,Body->GetComponentLocation(),EndBody,Pose.GetRotation(),Query);
   for(const auto& Candidate:Hits)if(Candidate.bBlockingHit&&(!Hit.bBlockingHit||Candidate.Time<Hit.Time))Hit=Candidate;
   SetActorLocationAndRotation(FMath::Lerp(GetActorLocation(),Pose.GetLocation(),Hit.bBlockingHit?Hit.Time:1.f),Pose.GetRotation(),false);
  }else SetActorLocationAndRotation(Pose.GetLocation(),Pose.GetRotation(),true,&Hit);
  if(Hit.bBlockingHit){
#if !UE_BUILD_SHIPPING
   if(FParse::Param(FCommandLine::Get(),TEXT("BattleRoadContactAudit"))){
    const FVector Local=GetActorTransform().InverseTransformPosition(Hit.ImpactPoint);
    for(int32 I=0;I<Contacts.Num();++I){
     FHitResult V;FCollisionQueryParams Probe(SCENE_QUERY_STAT(RoadSupportDiagnostic),true,this);const FVector P=Contacts[I];
     GetWorld()->LineTraceSingleByChannel(V,P+FVector(0,0,250),P-FVector(0,0,250),ECC_Visibility,Probe);
     UE_LOG(LogTemp,Display,TEXT("RoadWheelContact: index=%d point=%s visibility=%s actor=%s type=%d"),I,*P.ToString(),*V.ImpactPoint.ToString(),V.GetActor()?*V.GetActor()->GetName():TEXT("none"),V.GetComponent()?int32(V.GetComponent()->GetCollisionObjectType()):-1);
    }
    UE_LOG(LogTemp,Display,TEXT("RoadContact: distance=%.3f point=%s local=%s normal=%s pose=%s time=%.5f penetrating=%d depth=%.3f"),RouteDistance,*Hit.ImpactPoint.ToString(),*Local.ToString(),*Hit.ImpactNormal.ToString(),*GetActorTransform().ToHumanReadableString(),Hit.Time,Hit.bStartPenetrating,Hit.PenetrationDepth);
    UKismetSystemLibrary::QuitGame(this,nullptr,EQuitPreference::Quit,false);
   }
#endif
   // A swept move can stop part-way through the requested step. Keep route
   // progress and wheel travel aligned with the position actually reached.
   const float ActualTravel=Travel*FMath::Clamp(Hit.Time,0.f,1.f);
   RouteDistance+=ActualTravel;DistanceTravelled+=ActualTravel;
   UpdateWheels(Contacts,ActualTravel,WheelSteer);
   if(auto* Bike=Cast<ABattleBike>(Hit.GetActor())){
    const FVector RelativeVelocity=GetActorForwardVector()*Speed-Bike->GetVelocity();
    const float ClosingSpeed=-FVector::DotProduct(RelativeVelocity,Hit.ImpactNormal);
    if(!Bike->bParked&&Bike->RiderHealth>0&&ClosingSpeed>500.f)Bike->Ride->Wipeout(TEXT("Traffic impact"));
   }
   LastObstacle=FString(TEXT("movement:"))+(Hit.GetActor()?Hit.GetActor()->GetName():TEXT("unknown"));Speed=0;break;
  }
  RouteDistance+=Travel;DistanceTravelled+=Travel;UpdateWheels(Contacts,Travel,WheelSteer);
  if(Lengths.Last()-RouteDistance<.01f&&bLoopRoute){
   RouteDistance=0;++CompletedLoops;ClearedCrossings.Reset();AmberStopping.Reset();
   for(auto& Gate:Crossings)if(IsValid(Gate.Crossing))Gate.Crossing->ReleaseVehicle(this);
  }else if(ToEnd<3&&!bLoopRoute){bRouteFinished=true;Speed=0;break;}
 }
}
