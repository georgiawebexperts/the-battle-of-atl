#include "PiedmontBike.h"
#include "PiedmontExplorer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PiedmontPathSpline.h"
#include "Components/SplineComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/Canvas.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

UPiedmontBikeMovement::UPiedmontBikeMovement(){PrimaryComponentTick.bCanEverTick=true;}
float UPiedmontBikeMovement::GearLimit() const {static const float Caps[]={310,490,670,850,1040,1220,1430};return Caps[FMath::Clamp(Gear-1,0,6)];}
void UPiedmontBikeMovement::Shift(int D){Gear=FMath::Clamp(Gear+D,1,7);}
void UPiedmontBikeMovement::Crash(const FString& Reason){
 if(Recovery>0)return;
 if(Reason.StartsWith(TEXT("Water"))){
  if(LastCrash==Reason&&bRecoveryHold&&Speed<=1)return;
  Crashes++;LastCrash=Reason;
  if(auto* Bike=Cast<APiedmontBike>(PawnOwner))if(Bike->Dismount(true))return;
  // A blocked exit must never teleport the bike away from its water contact.
  Speed=0;VerticalSpeed=0;Pedal=0;bRecoveryHold=true;return;
 }
 Recovery=4.f;Crashes++;LastCrash=Reason;Speed=0;VerticalSpeed=0;Velocity=FVector::ZeroVector;
}
void UPiedmontBikeMovement::Respawn(){
 if(!UpdatedComponent)return;
 bRecoveryHold=LastCrash.StartsWith(TEXT("Water"));
 if(LastCrash.StartsWith(TEXT("Water"))){
  const FVector From=UpdatedComponent->GetComponentLocation();double Best=TNumericLimits<double>::Max();
  FCollisionQueryParams Query(SCENE_QUERY_STAT(PiedmontShoreRecovery),false,PawnOwner);
  auto Consider=[&](const FVector& P,const FVector& Direction){
   const double Distance=FVector::DistSquared2D(P,From);if(Distance>=Best)return;
   FHitResult Hit;
   if(!GetWorld()->SweepSingleByChannel(Hit,P+FVector(0,0,150),P-FVector(0,0,150),FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(2.f),Query))return;
   AActor* Surface=Hit.GetActor();
   if(!Surface||(!Surface->ActorHasTag(TEXT("RidePath"))&&!Surface->ActorHasTag(TEXT("RideDirt")))||Hit.ImpactNormal.Z<.6f)return;
   const FVector Target=Hit.ImpactPoint+FVector(0,0,101);
   // Test the whole tire/capsule footprint in the water plane. A high root
   // position must not make a horizontally submerged recovery point valid.
   if(!Surface->ActorHasTag(TEXT("RideBridge")))for(const auto& Water:WaterHazards)if(Water.IsValid()){
    for(int32 I=0;I<9;++I){
     const float Angle=(I-1)*PI/4;const float Radius=I==0?0.f:42.f;
     const FVector Sample(Target.X+FMath::Cos(Angle)*Radius,Target.Y+FMath::Sin(Angle)*Radius,Water->GetActorLocation().Z+1);
     if(Water->ContainsBike(Sample))return;
    }
   }
   if(GetWorld()->OverlapBlockingTestByChannel(Target,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32.f,96.f),Query))return;
   Best=Distance;SafeLocation=Target;SafeRotation=FRotator(0,Direction.Rotation().Yaw,0);
  };
  for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It){
   for(int32 I=0;I+1<It->Centerline->GetNumberOfSplinePoints();++I){
    const FVector A=It->Centerline->GetLocationAtSplinePoint(I,ESplineCoordinateSpace::World);
    const FVector B=It->Centerline->GetLocationAtSplinePoint(I+1,ESplineCoordinateSpace::World);const FVector D=B-A;
    const double Length2=D.X*D.X+D.Y*D.Y;
    const double T=Length2>0?FMath::Clamp(((From.X-A.X)*D.X+(From.Y-A.Y)*D.Y)/Length2,0.,1.):0.;
    Consider(A+D*T,D);Consider(A,D);Consider(B,D);
   }
  }
 }
 UpdatedComponent->SetWorldLocationAndRotation(SafeLocation,SafeRotation,false,nullptr,ETeleportType::TeleportPhysics);
 Speed=0;VerticalSpeed=0;Lean=0;Recovery=0;BrakePressure=0;SteeringRack=0;GripOverload=0;Velocity=FVector::ZeroVector;
}
void UPiedmontBikeMovement::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Fn){
 Super::TickComponent(Dt,Type,Fn);if(!PawnOwner||!UpdatedComponent||ShouldSkipUpdate(Dt))return;
 float Remaining=FMath::Min(Dt,.12f);while(Remaining>SMALL_NUMBER&&IsComponentTickEnabled()){const float StepTime=FMath::Min(Remaining,1.f/120);Step(StepTime);Remaining-=StepTime;}
 UpdateComponentVelocity();
}
void UPiedmontBikeMovement::Step(float Dt){
 BrakePressure=FMath::FInterpConstantTo(BrakePressure,Brake,Dt,Brake>0?3.f:6.f);
 if(Recovery>0){Recovery-=Dt;if(Recovery<=0)Respawn();return;}
 const FVector Location=UpdatedComponent->GetComponentLocation();
 if(Location.Z<-2000){Crash(TEXT("Off course"));return;}
 FCollisionQueryParams Q(SCENE_QUERY_STAT(BikeGround),false,PawnOwner);
 const FVector Forward=UpdatedComponent->GetForwardVector();
 FHitResult Ground,Front,Rear;
 auto Probe=[&](FVector P,FHitResult& H){
  const FVector Start=P+FVector(0,0,100),End=P-FVector(0,0,180);
  if(GetWorld()->LineTraceSingleByChannel(H,Start,End,ECC_Visibility,Q))return true;
  // A finite tire contact remains stable at exact Landscape triangle corners.
  return GetWorld()->SweepSingleByChannel(H,Start,End,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(2.f),Q);
 };
 Probe(Location,Ground);Probe(Location+Forward*60,Front);Probe(Location-Forward*60,Rear);
 bGrounded=Ground.bBlockingHit&&(Location.Z-Ground.ImpactPoint.Z<118)&&Ground.ImpactNormal.Z>.45;
 const bool OnBridge=Ground.bBlockingHit&&Ground.GetActor()&&Ground.GetActor()->ActorHasTag(TEXT("RideBridge"))&&Ground.ImpactNormal.Z>.45;
 if(!OnBridge)for(const auto& Water:WaterHazards)if(Water.IsValid()&&Water->ContainsBike(Location)){Crash(TEXT("Water — shoreline recovery"));return;}
 bGrass=bGrounded&&Ground.GetActor()&&(Ground.GetActor()->ActorHasTag(TEXT("RideGrass"))||Ground.GetActor()->ActorHasTag(TEXT("RideDirt")));
 if(bGrounded&&Ground.GetActor()&&Ground.GetActor()->ActorHasTag(TEXT("RideWater"))){Crash(TEXT("Water — shoreline recovery"));return;}
 const float Grip=bGrass?.6f:1.f;
 const float Slope=bGrounded?FVector::DotProduct(Forward,Ground.ImpactNormal):0.f;
 // Power and gear-dependent wheel torque; high gears are intentionally weak off the line.
 const float Cap=GearLimit()*(bGrass?.6f:1.f);
 static const float Accel[]={290,235,195,163,138,119,104};
 float Drive=Pedal>0&&BrakePressure<.01f?Accel[Gear-1]*FMath::Clamp((Cap-Speed)/120.f,0.f,1.f):0;
 float Resistance=(Speed>1?14.f+Speed*Speed*.000027f:0.f)*(bGrass?2.8f:1.f);
 // Keep a recovered bike stationary until the rider pedals again.
 if(Pedal>0||Speed>1)bRecoveryHold=false;
 float Acceleration=(bGrounded&&!bRecoveryHold?Drive-Resistance+980.f*Slope:0.f);
 if(BrakePressure>0&&bGrounded)Acceleration-=FMath::Min(Speed/FMath::Max(Dt,.001f),620.f*Grip*BrakePressure);
 Speed=FMath::Clamp(Speed+Acceleration*Dt,0.f,1650.f);TopSpeed=FMath::Max(TopSpeed,Speed);
 SteeringRack=FMath::FInterpConstantTo(SteeringRack,Steer,Dt,2.2f);
 const float WheelSteer=FMath::DegreesToRadians(SteeringRack*FMath::Lerp(32.f,8.f,FMath::Clamp(Speed/1400.f,0.f,1.f)));
 const float YawRate=bGrounded?(Speed/118.f)*FMath::Tan(WheelSteer):0.f;
 const float LateralAccel=Speed*YawRate;
 const float LeanTarget=FMath::RadiansToDegrees(FMath::Atan2(LateralAccel,980.f));
 Lean=FMath::FInterpTo(Lean,LeanTarget,Dt,5.f);
 const bool Overloaded=bGrounded&&Speed>650&&FMath::Abs(LateralAccel)>980.f*Grip*FMath::Lerp(.88f,.48f,BrakePressure);
 GripOverload=Overloaded?GripOverload+Dt:FMath::Max(0.f,GripOverload-Dt*2.f);
 if(GripOverload>.2f){Crash(TEXT("Traction lost — ease steering / braking"));return;}
 FRotator Heading=UpdatedComponent->GetComponentRotation();Heading.Yaw+=FMath::RadiansToDegrees(YawRate)*Dt;Heading.Pitch=Heading.Roll=0;
 const FVector NewForward=Heading.Vector();
 if(bGrounded){VerticalSpeed=0;Pitch=FMath::FInterpTo(Pitch,(Front.bBlockingHit&&Rear.bBlockingHit)?FMath::RadiansToDegrees(FMath::Atan2(Front.ImpactPoint.Z-Rear.ImpactPoint.Z,120.f)):0,Dt,7.f);}
 else VerticalSpeed-=980.f*Dt;
 FVector Travel=NewForward*Speed*Dt;
 if(bGrounded)Travel.Z=(Ground.ImpactPoint.Z+96-Location.Z)*FMath::Min(1.f,Dt*20.f)-FVector::DotProduct(NewForward,Ground.ImpactNormal)/FMath::Max(Ground.ImpactNormal.Z,.45)*Speed*Dt;
 else Travel.Z=VerticalSpeed*Dt;
 FHitResult Hit;SafeMoveUpdatedComponent(Travel,Heading.Quaternion(),true,Hit);
 if(Hit.IsValidBlockingHit()){
  const bool Water=Hit.GetActor()&&Hit.GetActor()->ActorHasTag(TEXT("RideWater"));
  if(Water){Crash(TEXT("Water — shoreline recovery"));return;}
  if(Hit.ImpactNormal.Z<.45){
   // A small curb or ramp lip is climbable; a wall or tall step is not.
   FHitResult Ahead;const FVector Here=UpdatedComponent->GetComponentLocation();
   GetWorld()->LineTraceSingleByChannel(Ahead,Here+NewForward*48+FVector(0,0,30),Here+NewForward*48-FVector(0,0,130),ECC_Visibility,Q);
   const float Rise=Ahead.bBlockingHit?Ahead.ImpactPoint.Z-(Here.Z-96):1000;
   bool Stepped=false;
   if(bGrounded&&Ahead.bBlockingHit&&Ahead.ImpactNormal.Z>.6&&Rise>=0&&Rise<=24&&(!Ahead.GetActor()||!Ahead.GetActor()->ActorHasTag(TEXT("RideWater")))){
    FHitResult UpHit,OverHit,DownHit;
    SafeMoveUpdatedComponent(FVector(0,0,26),Heading.Quaternion(),true,UpHit);
    if(!UpHit.bBlockingHit){
     SafeMoveUpdatedComponent(Travel*(1-Hit.Time),Heading.Quaternion(),true,OverHit);
     SafeMoveUpdatedComponent(FVector(0,0,-28),Heading.Quaternion(),true,DownHit);
     Stepped=!OverHit.bBlockingHit;
    }
   }
   if(!Stepped){
    if(Speed>330){Crash(FString::Printf(TEXT("Impact — %s"),Hit.GetActor()?*Hit.GetActor()->GetName():TEXT("obstacle")));return;}
    Speed*=.4f;SlideAlongSurface(Travel,1-Hit.Time,Hit.Normal,Hit,true);
   }
  }else {
   if(VerticalSpeed<0)VerticalSpeed=0;
   SlideAlongSurface(Travel,1-Hit.Time,Hit.Normal,Hit,true);
  }
 }
 Velocity=(UpdatedComponent->GetComponentLocation()-Location)/Dt;
 Cadence+=((Pedal>0&&Speed>5)?FMath::Clamp(Speed/(Gear*120.f),.35f,1.8f):0.f)*Dt*2*PI;
 if(bGrounded&&!bGrass&&Ground.GetActor()&&Ground.GetActor()->ActorHasTag(TEXT("RidePath"))){
  SafeTime+=Dt;if(SafeTime>1.5f){SafeLocation=UpdatedComponent->GetComponentLocation()+FVector(0,0,5);SafeRotation=Heading;SafeTime=0;}
 }
}

APiedmontBike::APiedmontBike(){
 PrimaryActorTick.bCanEverTick=true;
 Capsule=CreateDefaultSubobject<UCapsuleComponent>(TEXT("PhysicalBike"));SetRootComponent(Capsule);Capsule->InitCapsuleSize(32,96);Capsule->SetCollisionProfileName(TEXT("Pawn"));
 Ride=CreateDefaultSubobject<UPiedmontBikeMovement>(TEXT("BikeMovement"));Ride->SetUpdatedComponent(Capsule);
 Visual=CreateDefaultSubobject<USceneComponent>(TEXT("LeanAssembly"));Visual->SetupAttachment(Capsule);Visual->SetRelativeLocation(FVector(0,0,-96));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Paint(TEXT("/Game/BeltLineGlide/Materials/M_Bike.M_Bike"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Rubber(TEXT("/Game/BeltLineGlide/Materials/M_Rubber.M_Rubber"));
 auto Part=[&](FString Name,FVector Loc,FVector Scale,bool Round,UMaterialInterface* Mat){auto* M=CreateDefaultSubobject<UStaticMeshComponent>(*Name);M->SetupAttachment(Visual);M->SetStaticMesh(Round?Cylinder.Object:Cube.Object);M->SetRelativeLocation(Loc);M->SetRelativeScale3D(Scale);M->SetCollisionEnabled(ECollisionEnabled::NoCollision);M->SetMaterial(0,Mat);return M;};
 auto Tube=[&](FString Name,FVector A,FVector B,float Radius,UMaterialInterface* Mat){auto* M=Part(Name,(A+B)*.5,FVector(Radius/50,Radius/50,(B-A).Size()/100),true,Mat);M->SetRelativeRotation(FQuat::FindBetweenNormals(FVector::UpVector,(B-A).GetSafeNormal()));return M;};
 const FVector Back(-60,0,35),Front(60,0,35),Crank(-5,0,36),Seat(-23,0,92),Head(43,0,91);
 Tube(TEXT("SeatTube"),Crank,Seat,2.2,Paint.Object);Tube(TEXT("TopTube"),Seat,Head,2,Paint.Object);Tube(TEXT("DownTube"),Crank,Head,3,Paint.Object);
 for(int S:{-1,1}){Tube(FString::Printf(TEXT("Stay%d"),S),Back+FVector(0,S*7,0),Seat,1.4,Paint.Object);Tube(FString::Printf(TEXT("ChainStay%d"),S),Back+FVector(0,S*7,0),Crank,1.4,Paint.Object);Tube(FString::Printf(TEXT("Fork%d"),S),Head,Front+FVector(0,S*6,0),1.6,Rubber.Object);}
 Part(TEXT("Battery"),FVector(16,0,59),FVector(.14,.1,.42),false,Rubber.Object)->SetRelativeRotation(FRotator(35,0,0));
 Part(TEXT("Saddle"),FVector(-23,0,98),FVector(.29,.19,.055),false,Rubber.Object);
 Tube(TEXT("Stem"),Head,FVector(40,0,112),2,Rubber.Object);Tube(TEXT("Handlebar"),FVector(40,-30,112),FVector(40,30,112),1.5,Rubber.Object);
 FrontWheel=Part(TEXT("FrontWheel"),Front,FVector(.70,.70,.055),true,Rubber.Object);FrontWheel->SetRelativeRotation(FRotator(0,0,90));
 RearWheel=Part(TEXT("RearWheel"),Back,FVector(.70,.70,.055),true,Rubber.Object);RearWheel->SetRelativeRotation(FRotator(0,0,90));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> DetailedWheel(TEXT("/Game/PiedmontRide/Bike/SM_BikeWheel.SM_BikeWheel"));
 if(DetailedWheel.Succeeded()){FrontWheel->SetStaticMesh(DetailedWheel.Object);RearWheel->SetStaticMesh(DetailedWheel.Object);FrontWheel->SetRelativeScale3D(FVector(1));RearWheel->SetRelativeScale3D(FVector(1));}
 Tube(TEXT("HubMotor"),Back-FVector(0,6,0),Back+FVector(0,6,0),9,Rubber.Object);
 Rider=CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("RiggedRider"));Rider->SetupAttachment(Visual);Rider->SetRelativeRotation(FRotator(0,-90,0));Rider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 static ConstructorHelpers::FObjectFinder<USkeletalMesh> Human(TEXT("/Game/PiedmontRide/Rider/Casual.Casual"));
 Rider->SetSkinnedAssetAndUpdate(Human.Object);
 Arm=CreateDefaultSubobject<USpringArmComponent>(TEXT("ChaseArm"));Arm->SetupAttachment(Capsule);Arm->TargetArmLength=470;Arm->SetRelativeLocation(FVector(0,0,130));Arm->SetRelativeRotation(FRotator(-12,0,0));Arm->bEnableCameraLag=true;Arm->CameraLagSpeed=7;
 Chase=CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));Chase->SetupAttachment(Arm);Chase->FieldOfView=85;
 Handlebar=CreateDefaultSubobject<UCameraComponent>(TEXT("HandlebarCamera"));Handlebar->SetupAttachment(Visual);Handlebar->SetRelativeLocation(FVector(37,0,151));Handlebar->FieldOfView=95;Handlebar->SetAutoActivate(false);
}
UPawnMovementComponent* APiedmontBike::GetMovementComponent() const{return Ride;}
void APiedmontBike::BeginPlay(){
 Super::BeginPlay();Ride->SafeLocation=GetActorLocation();Ride->SafeRotation=GetActorRotation();
 for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)Ride->WaterHazards.Add(*It);
 if(auto* Mesh=Cast<USkeletalMesh>(Rider->GetSkinnedAsset())){
  const auto& Ref=Mesh->GetRefSkeleton();
  for(int32 I=0;I<Ref.GetNum();++I){Parents.Add(Ref.GetParentIndex(I));BoneNames.Add(Ref.GetBoneName(I));FTransform T=Ref.GetRefBonePose()[I];if(Parents[I]>=0)T=T*ReferencePose[Parents[I]];ReferencePose.Add(T);}
 }
}
void APiedmontBike::SetupPlayerInputComponent(UInputComponent* I){
 Super::SetupPlayerInputComponent(I);I->BindKey(EKeys::E,IE_Pressed,this,&APiedmontBike::Interact);I->BindAxis(TEXT("RidePedal"),this,&APiedmontBike::PedalInput);I->BindAxis(TEXT("RideSteer"),this,&APiedmontBike::Steering);
 I->BindAction(TEXT("RideBrake"),IE_Pressed,this,&APiedmontBike::BrakeOn);I->BindAction(TEXT("RideBrake"),IE_Released,this,&APiedmontBike::BrakeOff);
 I->BindAction(TEXT("RideGearUp"),IE_Pressed,this,&APiedmontBike::GearUp);I->BindAction(TEXT("RideGearDown"),IE_Pressed,this,&APiedmontBike::GearDown);
 I->BindAction(TEXT("RideCamera"),IE_Pressed,this,&APiedmontBike::ToggleCamera);I->BindAction(TEXT("RideReset"),IE_Pressed,this,&APiedmontBike::ResetRide);
}
void APiedmontBike::PedalInput(float V){Ride->Pedal=V;}void APiedmontBike::Steering(float V){Ride->Steer=V;}
void APiedmontBike::BrakeOn(){Ride->Brake=1;}void APiedmontBike::BrakeOff(){Ride->Brake=0;}
void APiedmontBike::GearUp(){Ride->Shift(1);}void APiedmontBike::GearDown(){Ride->Shift(-1);}
void APiedmontBike::ResetRide(){if(!bDismounted&&!Ride->LastCrash.StartsWith(TEXT("Water")))Ride->Respawn();}void APiedmontBike::ToggleCamera(){bFirstPerson=!bFirstPerson;Chase->SetActive(!bFirstPerson);Handlebar->SetActive(bFirstPerson);Rider->SetVisibility(!bFirstPerson);}
void APiedmontBike::Tick(float Dt){
 Super::Tick(Dt);if(bDismounted)return;const float CrashRoll=Ride->Recovery>0?FMath::Min(80.f,(4-Ride->Recovery)*140):Ride->Lean;
 Visual->SetRelativeRotation(FRotator(Ride->Pitch,0,CrashRoll));
 // Procedural dismount: the rider separates from the saddle during the fall.
 float Fall=Ride->Recovery>0?FMath::Clamp((4-Ride->Recovery)*2.f,0.f,1.f):0.f;
 Rider->SetRelativeLocation(FVector(35*Fall,-85*Fall,25*FMath::Sin(Fall*PI)));
 Rider->SetRelativeRotation(FRotator(-15*Fall,-90,20*Fall));
 WheelAngle+=Ride->Speed*Dt/35*180/PI;
 FrontWheel->SetRelativeRotation(FRotator(WheelAngle,Ride->Steer*20,90));RearWheel->SetRelativeRotation(FRotator(WheelAngle,0,90));PoseRider(Dt);
}
void APiedmontBike::PoseRider(float Dt){
 if(ReferencePose.IsEmpty())return;
 TArray<FTransform> Pose=ReferencePose;
 auto Index=[&](const TCHAR* Name){return BoneNames.IndexOfByKey(FName(Name));};
 auto Descendant=[&](int I,int Root){while(I>=0){if(I==Root)return true;I=Parents[I];}return false;};
 auto MoveBranch=[&](int Root,FVector Target,FQuat Rotation){if(Root<0)return;FVector Old=Pose[Root].GetLocation();for(int I=Root;I<Pose.Num();++I)if(Descendant(I,Root)){Pose[I].SetLocation(Target+Rotation.RotateVector(Pose[I].GetLocation()-Old));Pose[I].SetRotation(Rotation*Pose[I].GetRotation());}};
 const int Pelvis=Index(TEXT("Hips"));MoveBranch(Pelvis,FVector(0,-23,99),FQuat(FVector::ForwardVector,FMath::DegreesToRadians(-28.f-(Ride->Brake>0?8.f:0.f))));
 auto Limb=[&](const TCHAR* UpperName,const TCHAR* LowerName,const TCHAR* EndName,FVector Target,FVector Bend){
  int U=Index(UpperName),L=Index(LowerName),E=Index(EndName);if(U<0||L<0||E<0)return;
  FVector Origin=Pose[U].GetLocation();float A=FVector::Distance(Origin,Pose[L].GetLocation()),B=FVector::Distance(Pose[L].GetLocation(),Pose[E].GetLocation());
  FVector Direction=(Target-Origin).GetSafeNormal();float D=FMath::Clamp(FVector::Distance(Origin,Target),FMath::Abs(A-B)+.1f,A+B-.1f);Target=Origin+Direction*D;
  float Along=(A*A-B*B+D*D)/(2*D),Height=FMath::Sqrt(FMath::Max(0.f,A*A-Along*Along));FVector BendNormal=(Bend-Direction*FVector::DotProduct(Bend,Direction)).GetSafeNormal();FVector Joint=Origin+Direction*Along+BendNormal*Height;
  MoveBranch(U,Origin,FQuat::FindBetweenVectors(Pose[L].GetLocation()-Origin,Joint-Origin));
  MoveBranch(L,Joint,FQuat::FindBetweenVectors(Pose[E].GetLocation()-Pose[L].GetLocation(),Target-Joint));
 };
 const float C=Ride->Cadence;
 Limb(TEXT("UpperLeg_L"),TEXT("LowerLeg_L"),TEXT("Foot_L"),FVector(12,-5+FMath::Sin(C)*16,32+FMath::Cos(C)*16),FVector(0,1,0));
 Limb(TEXT("UpperLeg_R"),TEXT("LowerLeg_R"),TEXT("Foot_R"),FVector(-12,-5-FMath::Sin(C)*16,32-FMath::Cos(C)*16),FVector(0,1,0));
 Limb(TEXT("UpperArm_L"),TEXT("LowerArm_L"),TEXT("Hand_L"),FVector(28,29,114),FVector(1,0,-.4));
 Limb(TEXT("UpperArm_R"),TEXT("LowerArm_R"),TEXT("Hand_R"),FVector(-28,29,114),FVector(-1,0,-.4));
 for(int I=0;I<Pose.Num();++I)Rider->BoneSpaceTransforms[I]=Parents[I]>=0?Pose[I].GetRelativeTransform(Pose[Parents[I]]):Pose[I];
 Rider->MarkRefreshTransformDirty();
}
void APiedmontRideHUD::DrawHUD(){
 Super::DrawHUD();auto* Bike=Cast<APiedmontBike>(GetOwningPawn());
 if(auto* Person=Cast<APiedmontExplorer>(GetOwningPawn())){
  if(!Canvas)return;
  DrawRect(FLinearColor(.015,.027,.036,.9),24,24,680,82);
  DrawText(Person->bSwimming?TEXT("SWIMMING  /  WASD or arrows move  /  Mouse look"):TEXT("ON FOOT  /  WASD or arrows move  /  Mouse look"),FColor::White,40,40,nullptr,1.5f);
  DrawText(Person->bSwimming?TEXT("Swim to shore, then return to your bike  /  E remount"):TEXT("E remount near your bike  /  Bike stays where you left it"),FColor(89,229,203),40,75,nullptr,1.2f);return;
 }
 if(!Canvas||!Bike)return;auto* M=Bike->Ride.Get();
 float S=Canvas->SizeX/1280.f;auto Text=[&](FString T,float X,float Y,FColor C,float Size){DrawText(T,C,X*S,Y*S,nullptr,Size*S);};
 DrawRect(FLinearColor(.015,.027,.036,.88),24*S,24*S,700*S,96*S);
 Text(TEXT("PIEDMONT RIDE / PHYSICS LAB  •  BUILD 0.2.1"),40,37,FColor::White,1.65);
 Text(TEXT("W pedal   ↑ / ↓ gears   ← / → or A / D steer   SPACE brake"),40,72,FColor(193,219,220),1.25);
 Text(TEXT("TAB / SHIFT camera   E dismount below 8 MPH   R recover   ESC stop"),40,95,FColor(193,219,220),1.05);
 float Y=Canvas->SizeY/S-140;
 DrawRect(FLinearColor(.015,.027,.036,.9),24*S,Y*S,390*S,112*S);
 Text(FString::Printf(TEXT("%02.0f MPH     GEAR %d / 7"),M->Speed*.0223694,M->Gear),42,Y+16,FColor(89,229,203),2.5);
 Text(FString::Printf(TEXT("%s   |   LEAN %.0f°   |   CRASHES %d"),M->bGrass?TEXT("GRASS / 60% GRIP"):TEXT("PAVEMENT"),M->Lean,M->Crashes),42,Y+62,FColor::White,1.1);
 Text(TEXT("Web Experts  /  www.webexperts.com"),42,Y+87,FColor(163,183,184),.9);
 if(M->Recovery>0){DrawRect(FLinearColor(.1,.015,.01,.9),Canvas->SizeX*.25,Canvas->SizeY*.4,Canvas->SizeX*.5,100*S);Text(FString::Printf(TEXT("%s  —  %.1f sec"),*M->LastCrash,M->Recovery),Canvas->SizeX/S*.27,Canvas->SizeY/S*.4+30,FColor(255,177,126),1.5);}
}
APiedmontRideMode::APiedmontRideMode(){DefaultPawnClass=APiedmontBike::StaticClass();HUDClass=APiedmontRideHUD::StaticClass();}

void APiedmontBike::ValidationKey(FName Key,bool Pressed){
#if WITH_EDITOR
 if(auto* PC=Cast<APlayerController>(GetController()))PC->InputKey(FInputKeyParams(FKey(Key),Pressed?IE_Pressed:IE_Released,Pressed?1.0:0.0));
#endif
}

APiedmontWaterHazard::APiedmontWaterHazard(){
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("WaterBoundary"));
 PrimaryActorTick.bCanEverTick=false;
}
bool APiedmontWaterHazard::ContainsBike(const FVector& WorldPoint) const {
 if(Polygon.Num()<3)return false;
 const FVector P=GetActorTransform().InverseTransformPosition(WorldPoint);
 if(P.Z>DetectionHeight)return false;
 auto Contains=[&](const TArray<FVector>& Ring){
  bool Inside=false;
  for(int I=0,J=Ring.Num()-1;I<Ring.Num();J=I++){
   const FVector& A=Ring[I];const FVector& B=Ring[J];
   if((A.Y>P.Y)!=(B.Y>P.Y)&&P.X<(B.X-A.X)*(P.Y-A.Y)/(B.Y-A.Y)+A.X)Inside=!Inside;
  }
  return Inside;
 };
 return Contains(Polygon)&&!Contains(IslandPolygon);
}

void APiedmontBike::Interact(){Dismount();}
bool APiedmontBike::Dismount(bool WaterEntry){
 auto* PC=Cast<APlayerController>(GetController());
 if(!PC||bDismounted||Ride->Recovery>0)return false;
 // Keep dismounts controlled; high-speed falls are handled by crash logic.
 if(!WaterEntry&&Ride->Speed>350)return false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(PiedmontDismount),false,this);
 FVector Exit;bool Found=false;
 if(WaterEntry){
  for(float Distance:{130.f,220.f,320.f}){
   const FVector Ahead=GetActorLocation()+GetActorForwardVector()*Distance;
   for(const auto& Water:Ride->WaterHazards)if(Water.IsValid()){
    const FVector Target(Ahead.X,Ahead.Y,Water->GetActorLocation().Z+100);
    if(Water->ContainsBike(Target)&&!GetWorld()->OverlapBlockingTestByChannel(Target,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,88),Q)){Exit=Target;Found=true;break;}
   }
   if(Found)break;
  }
 }
 for(float Side:{1.f,-1.f}){
  if(Found)break;
  const FVector Candidate=GetActorLocation()+GetActorRightVector()*Side*105;
  FHitResult Ground;
  if(!GetWorld()->LineTraceSingleByChannel(Ground,Candidate+FVector(0,0,80),Candidate-FVector(0,0,200),ECC_Visibility,Q)||Ground.ImpactNormal.Z<.65f)continue;
  const FVector Target=Ground.ImpactPoint+FVector(0,0,92);
  bool Wet=false;for(const auto& Water:Ride->WaterHazards)if(Water.IsValid()&&Water->ContainsBike(Target))Wet=true;
  if(Wet||GetWorld()->OverlapBlockingTestByChannel(Target,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,88),Q))continue;
  FHitResult Crossing;
  if(GetWorld()->SweepSingleByChannel(Crossing,GetActorLocation(),Target,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,88),Q))continue;
  Exit=Target;Found=true;break;
 }
 if(!Found)return false;
 FActorSpawnParameters Params;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
 auto* Person=GetWorld()->SpawnActor<APiedmontExplorer>(APiedmontExplorer::StaticClass(),Exit,GetActorRotation(),Params);if(!Person)return false;
 Person->Bike=this;
 const FVector Momentum=GetActorForwardVector()*Ride->Speed;
 Ride->Pedal=Ride->Steer=Ride->Brake=0;Ride->Speed=0;Ride->Velocity=FVector::ZeroVector;Ride->SetComponentTickEnabled(false);
 bDismounted=true;Explorer=Person;Rider->SetVisibility(false);PC->Possess(Person);PC->SetControlRotation(GetActorRotation());
 if(!WaterEntry)Person->LaunchCharacter(Momentum,false,false);return true;
}
bool APiedmontBike::Remount(APiedmontExplorer* Person){
 if(!bDismounted||Person!=Explorer||!IsValid(Person)||FVector::DistSquared(Person->GetActorLocation(),GetActorLocation())>FMath::Square(180.f))return false;
 auto* PC=Cast<APlayerController>(Person->GetController());if(!PC)return false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(PiedmontRemount),false,this);Q.AddIgnoredActor(Person);
 FHitResult Hit;if(GetWorld()->LineTraceSingleByChannel(Hit,Person->GetActorLocation(),GetActorLocation(),ECC_Visibility,Q))return false;
 PC->Possess(this);PC->SetControlRotation(GetActorRotation());bDismounted=false;Explorer=nullptr;
 Ride->Pedal=Ride->Steer=Ride->Brake=0;Ride->bRecoveryHold=true;Ride->SetComponentTickEnabled(true);Rider->SetVisibility(!bFirstPerson);
 Person->Destroy();return true;
}
