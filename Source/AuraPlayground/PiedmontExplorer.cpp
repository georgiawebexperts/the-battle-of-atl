#include "PiedmontExplorer.h"
#include "PiedmontBike.h"
#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
APiedmontExplorer::APiedmontExplorer(){
 PrimaryActorTick.bCanEverTick=true;
 GetCapsuleComponent()->InitCapsuleSize(30,88);
 GetCharacterMovement()->MaxWalkSpeed=380;
 GetCharacterMovement()->MaxStepHeight=35;
 GetCharacterMovement()->BrakingDecelerationWalking=1600;
 bUseControllerRotationYaw=true;
 GetMesh()->SetVisibility(false);GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Body=CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("WalkingRider"));Body->SetupAttachment(GetCapsuleComponent());
 Body->SetRelativeLocation(FVector(0,0,-88));Body->SetRelativeRotation(FRotator(0,-90,0));Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 static ConstructorHelpers::FObjectFinder<USkeletalMesh> Human(TEXT("/Game/PiedmontRide/Rider/Casual.Casual"));Body->SetSkinnedAssetAndUpdate(Human.Object);
 CameraArm=CreateDefaultSubobject<USpringArmComponent>(TEXT("ExplorerArm"));CameraArm->SetupAttachment(GetCapsuleComponent());CameraArm->TargetArmLength=320;CameraArm->SetRelativeLocation(FVector(0,0,65));CameraArm->bUsePawnControlRotation=true;
 Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("ExplorerCamera"));Camera->SetupAttachment(CameraArm);Camera->FieldOfView=85;
}
void APiedmontExplorer::BeginPlay(){
 Super::BeginPlay();
 for(TActorIterator<AActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("RideWater")))GetCapsuleComponent()->IgnoreActorWhenMoving(*It,true);
 if(auto* Asset=Cast<USkeletalMesh>(Body->GetSkinnedAsset())){
  const auto& Ref=Asset->GetRefSkeleton();
  for(int32 I=0;I<Ref.GetNum();++I){Parents.Add(Ref.GetParentIndex(I));Bones.Add(Ref.GetBoneName(I));RestPose.Add(Ref.GetRefBonePose()[I]);}
 }
}
void APiedmontExplorer::SetupPlayerInputComponent(UInputComponent* Input){Super::SetupPlayerInputComponent(Input);Input->BindKey(EKeys::E,IE_Pressed,this,&APiedmontExplorer::Interact);}
void APiedmontExplorer::Interact(){Remount();}
bool APiedmontExplorer::Remount(){return !bSwimming&&IsValid(Bike)&&Bike->Remount(this);}
void APiedmontExplorer::Tick(float Dt){
 Super::Tick(Dt);UpdateSwimming(Dt);
 if(auto* PC=Cast<APlayerController>(GetController())){
  float X=0,Y=0;PC->GetInputMouseDelta(X,Y);AddControllerYawInput(X*.18f);AddControllerPitchInput(Y*-.12f);
  const float Forward=(PC->IsInputKeyDown(EKeys::W)||PC->IsInputKeyDown(EKeys::Up)?1.f:0.f)-(PC->IsInputKeyDown(EKeys::S)||PC->IsInputKeyDown(EKeys::Down)?1.f:0.f);
  const float Side=(PC->IsInputKeyDown(EKeys::D)||PC->IsInputKeyDown(EKeys::Right)?1.f:0.f)-(PC->IsInputKeyDown(EKeys::A)||PC->IsInputKeyDown(EKeys::Left)?1.f:0.f);
  FRotator Heading(0,PC->GetControlRotation().Yaw,0);FVector Move=Heading.Vector()*Forward+FRotationMatrix(Heading).GetUnitAxis(EAxis::Y)*Side;
  AddMovementInput(Move.GetSafeNormal(),FMath::Min(1.f,Move.Size()));
 }
 AnimateBody(Dt);
}
void APiedmontExplorer::AnimateBody(float Dt){
 if(RestPose.IsEmpty())return;
 Body->SetRelativeLocation(bSwimming?FVector(-65,0,-75):FVector(0,0,-88));
 Body->SetRelativeRotation(bSwimming?FRotator(0,-90,90):FRotator(0,-90,0));
 Gait+=bSwimming?Dt*3.f:GetVelocity().Size2D()*Dt/85.f;
 const float Blend=FMath::Clamp(GetVelocity().Size2D()/220.f,0.f,1.f);
 TArray<FTransform> Pose=RestPose;
 for(int32 I=0;I<Pose.Num();++I)if(Parents[I]>=0)Pose[I]=Pose[I]*Pose[Parents[I]];
 auto Index=[&](const TCHAR* Name){return Bones.IndexOfByKey(FName(Name));};
 auto Descendant=[&](int I,int Root){while(I>=0){if(I==Root)return true;I=Parents[I];}return false;};
 auto MoveBranch=[&](int Root,FVector Target,FQuat Rotation){
  if(Root<0)return;const FVector Old=Pose[Root].GetLocation();
  for(int I=Root;I<Pose.Num();++I)if(Descendant(I,Root)){Pose[I].SetLocation(Target+Rotation.RotateVector(Pose[I].GetLocation()-Old));Pose[I].SetRotation(Rotation*Pose[I].GetRotation());}
 };
 auto Limb=[&](const TCHAR* Upper,const TCHAR* Lower,const TCHAR* End,FVector Target,FVector Bend){
  const int U=Index(Upper),L=Index(Lower),E=Index(End);if(U<0||L<0||E<0)return;
  const FVector Origin=Pose[U].GetLocation();const float A=FVector::Distance(Origin,Pose[L].GetLocation()),B=FVector::Distance(Pose[L].GetLocation(),Pose[E].GetLocation());
  const FVector Direction=(Target-Origin).GetSafeNormal();const float D=FMath::Clamp(FVector::Distance(Origin,Target),FMath::Abs(A-B)+.1f,A+B-.1f);Target=Origin+Direction*D;
  const float Along=(A*A-B*B+D*D)/(2*D),Height=FMath::Sqrt(FMath::Max(0.f,A*A-Along*Along));
  const FVector Joint=Origin+Direction*Along+(Bend-Direction*FVector::DotProduct(Bend,Direction)).GetSafeNormal()*Height;
  MoveBranch(U,Origin,FQuat::FindBetweenVectors(Pose[L].GetLocation()-Origin,Joint-Origin));
  MoveBranch(L,Joint,FQuat::FindBetweenVectors(Pose[E].GetLocation()-Pose[L].GetLocation(),Target-Joint));
 };
 const float Stroke=.5f+.5f*FMath::Sin(Gait);
 for(int Side:{1,-1}){
  const FVector Hand=bSwimming?FVector(Side*(20+35*Stroke),20+10*FMath::Cos(Gait),175-45*Stroke):FVector(Side*26,FMath::Sin(Gait)*Side*18*Blend,80);
  if(Side==1)Limb(TEXT("UpperArm_L"),TEXT("LowerArm_L"),TEXT("Hand_L"),Hand,FVector(1,1,0));
  else Limb(TEXT("UpperArm_R"),TEXT("LowerArm_R"),TEXT("Hand_R"),Hand,FVector(-1,1,0));
  const int Leg=Index(Side==1?TEXT("UpperLeg_L"):TEXT("UpperLeg_R"));
  if(Leg>=0)MoveBranch(Leg,Pose[Leg].GetLocation(),FQuat(FVector::ForwardVector,FMath::DegreesToRadians(Side*FMath::Sin(Gait)*(bSwimming?8.f:28.f*Blend))));
 }
 for(int I=0;I<Pose.Num();++I)Body->BoneSpaceTransforms[I]=Parents[I]>=0?Pose[I].GetRelativeTransform(Pose[Parents[I]]):Pose[I];
 Body->MarkRefreshTransformDirty();
}
void APiedmontExplorer::ValidationKey(FName Key,bool Pressed){
#if WITH_EDITOR
 if(auto* PC=Cast<APlayerController>(GetController()))PC->InputKey(FInputKeyParams(FKey(Key),Pressed?IE_Pressed:IE_Released,Pressed?1.0:0.0));
#endif
}

void APiedmontExplorer::UpdateSwimming(float Dt){
 APiedmontWaterHazard* Water=nullptr;
 const FVector Here=GetActorLocation();
 for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It){
  const float Level=It->GetActorLocation().Z;
  if(It->ContainsBike(FVector(Here.X,Here.Y,Level+1))&&Here.Z-88<Level+25){Water=*It;break;}
 }
 const bool InWater=Water!=nullptr;
 if(InWater!=bSwimming){
  bSwimming=InWater;GetCharacterMovement()->SetMovementMode(bSwimming?MOVE_Flying:MOVE_Walking);
  GetCharacterMovement()->MaxFlySpeed=200;GetCharacterMovement()->BrakingDecelerationFlying=1200;
 }
 if(bSwimming){
  const float Target=Water->GetActorLocation().Z+88;
  FHitResult Hit;SetActorLocation(FVector(Here.X,Here.Y,FMath::FInterpTo(Here.Z,Target,Dt,8)),true,&Hit);
  GetCharacterMovement()->Velocity.Z=0;
 }
}
