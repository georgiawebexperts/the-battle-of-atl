#include "BattleSkater.h"
#include "BattleBike.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
ABattleSkater::ABattleSkater(){
 Tags.Add(TEXT("SkateparkSkater"));
 SkateAssembly=CreateDefaultSubobject<USceneComponent>(TEXT("SkateAssembly"));SkateAssembly->SetupAttachment(GetCapsuleComponent());SkateAssembly->SetRelativeLocation(FVector(0,0,-88));
 Body->SetupAttachment(SkateAssembly);Body->SetRelativeLocation(FVector::ZeroVector);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube")),Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder")),Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Wood(TEXT("/Game/BattleForTheA/Furniture/M_BenchWood.M_BenchWood")),Iron(TEXT("/Game/BattleForTheA/Furniture/M_BenchFrame.M_BenchFrame"));
 auto Part=[&](FString Name,UStaticMesh* Mesh,FVector P,FVector S,FRotator R,UMaterialInterface* Mat){auto* M=CreateDefaultSubobject<UStaticMeshComponent>(*Name);M->SetupAttachment(SkateAssembly);M->SetStaticMesh(Mesh);M->SetMaterial(0,Mat);M->SetRelativeLocation(P);M->SetRelativeScale3D(S);M->SetRelativeRotation(R);M->SetCollisionEnabled(ECollisionEnabled::NoCollision);M->SetCanEverAffectNavigation(false);};
 Part(TEXT("Deck"),Cube.Object,FVector(0,0,9),FVector(.68,.22,.025),FRotator::ZeroRotator,Wood.Object);
 for(int End:{-1,1}){
  Part(FString::Printf(TEXT("DeckTip%d"),End),Sphere.Object,FVector(End*34,0,10),FVector(.17,.22,.035),FRotator(End*8,0,0),Wood.Object);
  Part(FString::Printf(TEXT("Truck%d"),End),Cube.Object,FVector(End*24,0,5),FVector(.04,.20,.035),FRotator::ZeroRotator,Iron.Object);
  for(int Side:{-1,1})Part(FString::Printf(TEXT("Wheel%d_%d"),End,Side),Cylinder.Object,FVector(End*24,Side*10,3.5),FVector(.07,.07,.03),FRotator(0,0,90),Iron.Object);
 }
}
void ABattleSkater::BeginPlay(){
 Super::BeginPlay();PreviousLocation=GetActorLocation();
 auto* Move=GetCharacterMovement();Move->MaxWalkSpeed=300;Move->MaxAcceleration=550;Move->BrakingDecelerationWalking=500;Move->RotationRate=FRotator(0,150,0);Move->SetWalkableFloorAngle(60);
 if(auto* Mesh=Cast<USkeletalMesh>(Body->GetSkinnedAsset())){const auto& Ref=Mesh->GetRefSkeleton();for(int I=0;I<Ref.GetNum();I++){SkateParents.Add(Ref.GetParentIndex(I));SkateBones.Add(Ref.GetBoneName(I));SkateRest.Add(Ref.GetRefBonePose()[I]);if(SkateParents[I]>=0)SkateRest[I]=SkateRest[I]*SkateRest[SkateParents[I]];}}
}
void ABattleSkater::Tick(float Dt){
 // Reuse visitor health, collision and weapon rules; skate routes replace walking AI.
 APiedmontExplorer::Tick(Dt);
 Travelled+=FVector::Dist2D(GetActorLocation(),PreviousLocation);PreviousLocation=GetActorLocation();
 if(bDead){GetCharacterMovement()->StopMovementImmediately();Body->SetRelativeRotation(FRotator(0,-90,85));return;}
 const auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));if(!Mode||Mode->bRunEnded||Mode->StartCountdown>0)return;
 if(HornReactions!=LastHorn){LastHorn=HornReactions;YieldTime=2;if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();}
 YieldCooldown=FMath::Max(0.f,YieldCooldown-Dt);YieldTime=FMath::Max(0.f,YieldTime-Dt);StumbleRemaining=FMath::Max(0.f,StumbleRemaining-Dt);
 if(StumbleRemaining>0||YieldTime>0){GetCharacterMovement()->MaxWalkSpeed=0;return;}
 SkateClock+=Dt;GetCharacterMovement()->MaxWalkSpeed=260+40*FMath::Max(0.f,FMath::Sin((SkateClock+PhaseOffset)*2*PI/3.2f));
 static const FVector Route[]={FVector(-1450,-1050,0),FVector(1450,-1050,0),FVector(1650,-850,0),FVector(1650,850,0),FVector(1450,1050,0),FVector(-1450,1050,0),FVector(-1650,850,0),FVector(-1650,-850,0)};
 FVector To=FVector(39000,74000,0)+Route[RouteIndex]-GetActorLocation();To.Z=0;
 if(To.Size()<100){RouteIndex=(RouteIndex+1)%8;Corners++;To=FVector(39000,74000,0)+Route[RouteIndex]-GetActorLocation();To.Z=0;}
 const float Turn=FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw,To.Rotation().Yaw);CarveLean=FMath::FInterpTo(CarveLean,FMath::Clamp(Turn*.15f,-12.f,12.f),Dt,5);
 AddMovementInput(To.GetSafeNormal(),1);
}
void ABattleSkater::AnimateBody(float Dt){
 if(SkateRest.IsEmpty())return;TArray<FTransform> Pose=SkateRest;
 auto Index=[&](const TCHAR* N){return SkateBones.IndexOfByKey(FName(N));};
 auto Child=[&](int I,int Root){while(I>=0){if(I==Root)return true;I=SkateParents[I];}return false;};
 auto Move=[&](int Root,FVector Target,FQuat Q){if(Root<0)return;const FVector Old=Pose[Root].GetLocation();for(int I=Root;I<Pose.Num();I++)if(Child(I,Root)){Pose[I].SetLocation(Target+Q.RotateVector(Pose[I].GetLocation()-Old));Pose[I].SetRotation(Q*Pose[I].GetRotation());}};
 const float Phase=FMath::Fmod(SkateClock+PhaseOffset,3.2f);PushAmount=GetVelocity().Size2D()>50&&Phase<1.f?FMath::Sin(Phase*PI):0;
 const int Hips=Index(TEXT("Hips"));Move(Hips,FVector(0,-3,76-5*PushAmount),FQuat(FVector::UpVector,FMath::DegreesToRadians(55.f))*FQuat(FVector::ForwardVector,FMath::DegreesToRadians(-8.f)));
 const int Chest=Index(TEXT("Chest")),Head=Index(TEXT("Head"));if(Chest>=0)Move(Chest,Pose[Chest].GetLocation(),FQuat(FVector::UpVector,FMath::DegreesToRadians(-30.f)));if(Head>=0)Move(Head,Pose[Head].GetLocation(),FQuat(FVector::UpVector,FMath::DegreesToRadians(-20.f)));
 StanceError=0;
 auto Limb=[&](const TCHAR* Upper,const TCHAR* Lower,const TCHAR* End,FVector Target,FVector Bend,bool Foot,float FootYaw){
  const int U=Index(Upper),L=Index(Lower),E=Index(End);if(U<0||L<0||E<0)return;
  const FVector Start=Pose[U].GetLocation();const float A=FVector::Distance(Start,Pose[L].GetLocation()),B=FVector::Distance(Pose[L].GetLocation(),Pose[E].GetLocation());const FVector Desired=Target,Dir=(Target-Start).GetSafeNormal();const float D=FMath::Clamp(FVector::Distance(Start,Target),FMath::Abs(A-B)+.1f,A+B-.1f);Target=Start+Dir*D;
  const float Along=(A*A-B*B+D*D)/(2*D),Height=FMath::Sqrt(FMath::Max(0.f,A*A-Along*Along));const FVector Joint=Start+Dir*Along+(Bend-Dir*FVector::DotProduct(Bend,Dir)).GetSafeNormal()*Height;
  Move(U,Start,FQuat::FindBetweenVectors(Pose[L].GetLocation()-Start,Joint-Start));Move(L,Joint,FQuat::FindBetweenVectors(Pose[E].GetLocation()-Pose[L].GetLocation(),Target-Joint));
  if(Foot){const FQuat Q=FQuat(FVector::UpVector,FMath::DegreesToRadians(FootYaw))*SkateRest[E].GetRotation();Move(E,Pose[E].GetLocation(),Q*Pose[E].GetRotation().Inverse());StanceError=FMath::Max(StanceError,float(FVector::Distance(Pose[E].GetLocation(),Desired)));}
 };
 Limb(TEXT("UpperLeg_L"),TEXT("LowerLeg_L"),TEXT("Foot_L"),FVector(0,22,15),FVector(1,1,0),true,35);
 const FVector Back=FMath::Lerp(FVector(-2,-22,15),FVector(-25,-45,7),PushAmount);Limb(TEXT("UpperLeg_R"),TEXT("LowerLeg_R"),TEXT("Foot_R"),Back,FVector(-1,0,0),true,70*(1-PushAmount));
 Limb(TEXT("UpperArm_L"),TEXT("LowerArm_L"),TEXT("Hand_L"),FVector(33,20,100+FMath::Abs(CarveLean)),FVector(1,1,0),false,0);
 Limb(TEXT("UpperArm_R"),TEXT("LowerArm_R"),TEXT("Hand_R"),FVector(-30,-20,94-10*PushAmount),FVector(-1,0,0),false,0);
 const FVector N=GetCharacterMovement()->CurrentFloor.HitResult.ImpactNormal;const float Pitch=GetCharacterMovement()->IsMovingOnGround()?FMath::RadiansToDegrees(FMath::Atan2(-FVector::DotProduct(GetActorForwardVector(),N),N.Z)):0;
 SkateAssembly->SetRelativeRotation(FMath::RInterpTo(SkateAssembly->GetRelativeRotation(),FRotator(FMath::Clamp(Pitch,-30.f,30.f),0,CarveLean),Dt,8));Body->SetRelativeLocation(FVector::ZeroVector);Body->SetRelativeRotation(FRotator(0,-90,StumbleRemaining>0?FMath::Sin(StumbleRemaining*4)*20:0));
 for(int I=0;I<Pose.Num();I++)Body->BoneSpaceTransforms[I]=SkateParents[I]>=0?Pose[I].GetRelativeTransform(Pose[SkateParents[I]]):Pose[I];Body->MarkRefreshTransformDirty();
}
