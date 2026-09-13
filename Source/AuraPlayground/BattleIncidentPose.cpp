#include "PiedmontPedestrian.h"
#include "Animation/AnimSequence.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PiedmontBike.h"
#include "Kismet/GameplayStatics.h"

bool APiedmontPedestrian::BeginIncidentPose(UAnimSequence* Clip,float PoseSeconds,float HoldSeconds){
 if(!Clip||!FMath::IsFinite(PoseSeconds)||!FMath::IsFinite(HoldSeconds)||HoldSeconds<=0||PoseSeconds<0||PoseSeconds>=Clip->GetPlayLength())return false;
 if(!bNativeCrowdRig||bDead||bSwimming||KnockdownPhase||StumbleRemaining>0||SleepPhase||bBenchReaching||bIncidentPosing)return false;
 if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();
 GetCharacterMovement()->StopMovementImmediately();bHasDestination=false;PauseRemaining=0;
 SetBodySequence(Clip,false);HoldBodySequenceAt(PoseSeconds);
 if(!CreateIncidentCollision()){StopBodySequence();return false;}
 IncidentHoldRemaining=HoldSeconds;bIncidentPosing=true;bIncidentRecovering=false;AnimateBody(0);
 return true;
}
void APiedmontPedestrian::ReleaseIncidentPose(){
 if(!bIncidentPosing||bIncidentRecovering)return;
 // Keep an external release request pending if an obstruction delays get-up.
 IncidentHoldRemaining=0;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(IncidentStandingSpace),false,this);
 if(GetWorld()->OverlapBlockingTestByChannel(GetActorLocation(),FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(GetCapsuleComponent()->GetScaledCapsuleRadius(),GetCapsuleComponent()->GetScaledCapsuleHalfHeight()-2.f),Q))return;
 ResumeBodySequence();bIncidentRecovering=true;
}
void APiedmontPedestrian::CancelIncidentPose(){
 if(!bIncidentPosing)return;
 bIncidentPosing=false;bIncidentRecovering=false;IncidentHoldRemaining=0;StopBodySequence();ClearIncidentCollision();
}
bool APiedmontPedestrian::TickIncidentPose(float Dt){
 if(!bIncidentPosing)return false;
 UpdateIncidentCollision();
 auto* Mode=Cast<APiedmontRideMode>(UGameplayStatics::GetGameMode(this));
 if(bDead||bSwimming||!Mode||Mode->bRunEnded){CancelIncidentPose();return false;}
 if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();
 GetCharacterMovement()->StopMovementImmediately();bHasDestination=false;
 if(!bIncidentRecovering){IncidentHoldRemaining-=Dt;if(IncidentHoldRemaining<=0)ReleaseIncidentPose();}
 if(bIncidentRecovering&&!IsBodySequencePlaying()){CancelIncidentPose();PauseRemaining=1.f;}
 return true;
}

// Use the existing skeletal physics shapes as query targets while the standing
// movement capsule is disabled. No second visible mesh or physical simulation.
bool APiedmontPedestrian::CreateIncidentCollision(){
 auto* Mesh=Cast<USkeletalMesh>(Body->GetSkinnedAsset());if(!Mesh||!Mesh->GetPhysicsAsset())return false;
 IncidentCollision=NewObject<USkeletalMeshComponent>(this);AddInstanceComponent(IncidentCollision);
 IncidentCollision->SetSkeletalMeshAsset(Mesh);IncidentCollision->SetDisablePostProcessBlueprint(true);
 IncidentCollision->SetWorldTransform(Body->GetComponentTransform());IncidentCollision->SetVisibility(false);
 IncidentCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);IncidentCollision->SetCollisionObjectType(ECC_Pawn);
 IncidentCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
 IncidentCollision->SetCollisionResponseToChannel(ECC_Pawn,ECR_Block);
 IncidentCollision->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
 IncidentCollision->SetCanEverAffectNavigation(false);IncidentCollision->RegisterComponent();IncidentCollision->SetComponentTickEnabled(false);
 auto* Move=GetCharacterMovement();IncidentMovementMode=Move->MovementMode;IncidentCustomMovementMode=Move->CustomMovementMode;
 Move->StopMovementImmediately();Move->DisableMovement();GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 UpdateIncidentCollision();return true;
}
void APiedmontPedestrian::UpdateIncidentCollision(){
 if(!IncidentCollision)return;
 const auto* Mesh=Cast<USkeletalMesh>(Body->GetSkinnedAsset());if(!Mesh)return;
 for(const auto& Setup:Mesh->GetPhysicsAsset()->SkeletalBodySetups)
  if(auto* Instance=IncidentCollision->GetBodyInstance(Setup->BoneName)){
   const int32 Index=Body->GetBoneIndex(Setup->BoneName);
   if(Index!=INDEX_NONE)Instance->SetBodyTransform(Body->GetBoneTransform(Index),ETeleportType::TeleportPhysics);
  }
}
void APiedmontPedestrian::ClearIncidentCollision(){
 if(!IncidentCollision)return;
 IncidentCollision->DestroyComponent();IncidentCollision=nullptr;
 GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
 GetCharacterMovement()->SetMovementMode(static_cast<EMovementMode>(IncidentMovementMode),IncidentCustomMovementMode);
}
