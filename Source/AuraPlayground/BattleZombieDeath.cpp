#include "BattleZombie.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"

bool ABattleZombie::BeginDeathPhysics(FVector Direction){
 auto* Mesh=Cast<USkeletalMesh>(Body->GetSkinnedAsset());if(!Mesh||!Mesh->GetPhysicsAsset())return false;
 auto* DeathAsset=Mesh->GetPhysicsAsset();SetTickGroup(TG_PostPhysics);
 for(int Side=0;Side<2;Side++){const FName Foot=Side==0?TEXT("Foot_L"):TEXT("Foot_R"),Leg=Side==0?TEXT("LowerLeg_L"):TEXT("LowerLeg_R");DeathFootFromLeg[Side]=Body->GetSocketTransform(Foot).GetRelativeTransform(Body->GetSocketTransform(Leg));}
 DeathPhysics=NewObject<USkeletalMeshComponent>(this);AddInstanceComponent(DeathPhysics);
 DeathPhysics->SetSkeletalMeshAsset(Mesh);DeathPhysics->SetPhysicsAsset(DeathAsset,true);
 DeathPhysics->SetWorldTransform(Body->GetComponentTransform());
 DeathPhysics->SetCollisionProfileName(TEXT("Ragdoll"));DeathPhysics->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);DeathPhysics->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);DeathPhysics->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
 DeathPhysics->SetCanEverAffectNavigation(false);DeathPhysics->PhysicsTransformUpdateMode=EPhysicsTransformUpdateMode::ComponentTransformIsKinematic;
 DeathPhysics->SetVisibility(false);DeathPhysics->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
 DeathPhysics->RegisterComponent();DeathPhysics->RefreshBoneTransforms();DeathPhysics->SetAllBodiesSimulatePhysics(true);DeathPhysics->SetSimulatePhysics(true);
 const auto& Ref=Mesh->GetRefSkeleton();
 for(int I=0;I<Ref.GetNum();I++)if(auto* Instance=DeathPhysics->GetBodyInstance(Ref.GetBoneName(I))){Instance->SetBodyTransform(Body->GetBoneTransform(I),ETeleportType::TeleportPhysics);Instance->SetUseCCD(true);}
 DeathPhysics->SetAllPhysicsLinearVelocity(Direction.GetSafeNormal2D()*120+FVector(0,0,15));
 DeathPhysics->AddImpulse(Direction.GetSafeNormal2D()*100,TEXT("Chest"),true);
 MirrorDeathPose();return true;
}
void ABattleZombie::MirrorDeathPose(){
 if(!DeathPhysics)return;
 const auto& Ref=DeathPhysics->GetSkeletalMeshAsset()->GetRefSkeleton();TArray<FTransform> World,Local;
 if(auto* Chest=DeathPhysics->GetBodyInstance(TEXT("Chest")))Body->SetWorldLocation(Chest->GetUnrealWorldTransform().GetLocation()-FVector(0,0,125));
 const FTransform Frame=Body->GetComponentTransform();
 for(int I=0;I<Ref.GetNum();I++){
  const int Parent=Ref.GetParentIndex(I);FTransform T;
  if(auto* Instance=DeathPhysics->GetBodyInstance(Ref.GetBoneName(I))){T=Instance->GetUnrealWorldTransform();T.SetScale3D(Instance->Scale3D);}
  else T=Parent>=0?Ref.GetRefBonePose()[I]*World[Parent]:Ref.GetRefBonePose()[I]*Frame;
  World.Add(T);Local.Add(Parent>=0?T.GetRelativeTransform(World[Parent]):T.GetRelativeTransform(Frame));
 }
 // These source shoes are root-parented animation controls, not shin children.
 // Preserve their animated offsets from the shins while the body is physical.
 for(int Side=0;Side<2;Side++){const int Foot=Ref.FindBoneIndex(Side==0?TEXT("Foot_L"):TEXT("Foot_R")),Leg=Ref.FindBoneIndex(Side==0?TEXT("LowerLeg_L"):TEXT("LowerLeg_R"));if(Foot>=0&&Leg>=0)World[Foot]=DeathFootFromLeg[Side]*World[Leg];}
 for(int I=0;I<Ref.GetNum();I++){const int Parent=Ref.GetParentIndex(I);Local[I]=Parent>=0?World[I].GetRelativeTransform(World[Parent]):World[I].GetRelativeTransform(Frame);}
 Body->BoneSpaceTransforms=Local;Body->MarkRefreshTransformDirty();Body->RefreshBoneTransforms();Body->UpdateBounds();
}
