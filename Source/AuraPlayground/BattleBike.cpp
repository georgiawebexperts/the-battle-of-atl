#include "BattleBike.h"
#include "PiedmontPathSpline.h"
#include "PiedmontTrafficDirector.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
ABattleBike::ABattleBike(const FObjectInitializer& Init):Super(Init.SetDefaultSubobjectClass<UBattleBikeMovement>(ACharacter::CharacterMovementComponentName)){
 PrimaryActorTick.bCanEverTick=true;bUseControllerRotationYaw=false;
 Capsule=GetCapsuleComponent();Capsule->InitCapsuleSize(32,96);Capsule->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
 GetMesh()->SetVisibility(false);GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Ride=Cast<UBattleBikeMovement>(GetCharacterMovement());Tags.Add(TEXT("RideBike"));
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
void ABattleBike::BeginPlay(){
 Super::BeginPlay();Ride->LastSafeLocation=GetActorLocation();
 for(TActorIterator<AActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("RideWater")))Capsule->IgnoreActorWhenMoving(*It,true);
 if(auto* Mesh=Cast<USkeletalMesh>(Rider->GetSkinnedAsset())){
  const auto& Ref=Mesh->GetRefSkeleton();for(int32 I=0;I<Ref.GetNum();++I){Parents.Add(Ref.GetParentIndex(I));BoneNames.Add(Ref.GetBoneName(I));FTransform T=Ref.GetRefBonePose()[I];if(Parents[I]>=0)T=T*ReferencePose[Parents[I]];ReferencePose.Add(T);}
 }
}
void ABattleBike::SetupPlayerInputComponent(UInputComponent* I){
 Super::SetupPlayerInputComponent(I);I->BindKey(EKeys::Up,IE_Pressed,this,&ABattleBike::GearUp);I->BindKey(EKeys::Down,IE_Pressed,this,&ABattleBike::GearDown);I->BindKey(EKeys::Tab,IE_Pressed,this,&ABattleBike::ToggleCamera);
}
void ABattleBike::ToggleCamera(){bFirstPerson=!bFirstPerson;Chase->SetActive(!bFirstPerson);Handlebar->SetActive(bFirstPerson);Rider->SetVisibility(!bFirstPerson);}
void ABattleBike::Tick(float Dt){
 Super::Tick(Dt);
 if(auto* PC=Cast<APlayerController>(GetController())){
  Ride->Pedal=PC->IsInputKeyDown(EKeys::W)?1:0;
  Ride->Steer=(PC->IsInputKeyDown(EKeys::D)||PC->IsInputKeyDown(EKeys::Right)?1.f:0.f)-(PC->IsInputKeyDown(EKeys::A)||PC->IsInputKeyDown(EKeys::Left)?1.f:0.f);
  Ride->Brake=PC->IsInputKeyDown(EKeys::SpaceBar)?1:0;
 }
 if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this)))if(Mode->StartCountdown>0||Mode->bRunEnded)Ride->Pedal=Ride->Steer=0;
 const float Fall=Ride->Recovery>0?FMath::Sin((2-Ride->Recovery)*PI/2):0;
 const float Lean=Ride->Steer*FMath::Min(24.f,Ride->Speed*.035f);
 Visual->SetRelativeRotation(FRotator(0,0,Lean+Fall*65));Rider->SetRelativeLocation(FVector(0,-50*Fall,15*Fall));
 WheelAngle+=Ride->Speed*Dt/35*180/PI;FrontWheel->SetRelativeRotation(FRotator(WheelAngle,Ride->Steer*20,90));RearWheel->SetRelativeRotation(FRotator(WheelAngle,0,90));PoseRider(Dt);
}
FVector ABattleBike::FindPathReturn() const{
 FVector Best=Ride->LastSafeLocation;double Distance=TNumericLimits<double>::Max();
 FCollisionQueryParams Q(SCENE_QUERY_STAT(ArcadePathReturn),false,this);
 for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It){
  const FVector Point=It->Centerline->FindLocationClosestToWorldLocation(GetActorLocation(),ESplineCoordinateSpace::World);const double D=FVector::DistSquared2D(Point,GetActorLocation());if(D>=Distance)continue;
  FHitResult Ground;if(!GetWorld()->LineTraceSingleByChannel(Ground,Point+FVector(0,0,150),Point-FVector(0,0,150),ECC_Visibility,Q)||!Ground.GetActor()||Ground.ImpactNormal.Z<.4f)continue;
  if(!Ground.GetActor()->ActorHasTag(TEXT("RidePath"))&&!Ground.GetActor()->ActorHasTag(TEXT("RideDirt"))&&!Ground.GetActor()->ActorHasTag(TEXT("RideBridge")))continue;
  const FVector Return=Ground.ImpactPoint+FVector(0,0,98);if(GetWorld()->OverlapBlockingTestByChannel(Return,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,96),Q))continue;
  Best=Return;Distance=D;
 }
 return Best;
}
void ABattleBike::ValidationKey(FName Key,bool Pressed){
#if WITH_EDITOR
 if(auto* PC=Cast<APlayerController>(GetController()))PC->InputKey(FInputKeyParams(FKey(Key),Pressed?IE_Pressed:IE_Released,Pressed?1.0:0.0));
#endif
}
ABattleLabMode::ABattleLabMode(){DefaultPawnClass=ABattleBike::StaticClass();HUDClass=ABattleLabHUD::StaticClass();}
void ABattleLabMode::StartPlay(){AGameModeBase::StartPlay();if(TActorIterator<APiedmontPathSpline>(GetWorld()))if(auto* Director=GetWorld()->SpawnActor<APiedmontTrafficDirector>())Director->DesiredPopulation=50;}
void ABattleLabMode::Tick(float Dt){
 AGameModeBase::Tick(Dt);if(StartCountdown>0){StartCountdown=FMath::Max(0.f,StartCountdown-Dt);return;}if(!bRunEnded){TimeRemaining=FMath::Max(0.f,TimeRemaining-Dt);if(TimeRemaining<=0)bRunEnded=true;}
}
void ABattleLabHUD::DrawHUD(){
 Super::DrawHUD();if(!Canvas)return;auto* Bike=Cast<ABattleBike>(GetOwningPawn());if(!Bike)return;
 DrawRect(FLinearColor(.03,.015,.02,.85),20,20,720,125);DrawText(TEXT("BATTLE FOR THE A | ARCADE BIKE TEST"),FColor::White,35,30,nullptr,1.8);
 DrawText(FString::Printf(TEXT("GEAR %d / 5   %.0f MPH   Wipeouts %d"),Bike->Ride->Gear,Bike->Ride->Speed*.0223694f,Bike->Ride->Wipeouts),FColor(255,190,80),35,65,nullptr,1.5);
 DrawText(TEXT("W pedal | Arrows or A/D steer | Up/Down gears | Space brake | Tab view"),FColor::White,35,105,nullptr,1.1);
 if(Bike->Ride->Recovery>0)DrawText(FString::Printf(TEXT("%s — back in %.1f"),*Bike->Ride->RecoveryReason,Bike->Ride->Recovery),FColor::Yellow,Canvas->SizeX*.4,Canvas->SizeY*.5,nullptr,2);
}
void ABattleBike::PoseRider(float Dt){
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
