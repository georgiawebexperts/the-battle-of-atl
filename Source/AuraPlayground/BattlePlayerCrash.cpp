#include "BattlePlayerCrash.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleSpirit.h"
#include "BattleFallenBike.h"
#include "BattleCrashCamera.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
ABattlePlayerCrash::ABattlePlayerCrash(){PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickGroup=TG_PostPhysics;}
bool ABattlePlayerCrash::Start(ABattleBike* Source,const FVector& Velocity){
 if(!IsValid(Source)||Source->bParked||Source->RiderHealth<=0||!Source->GetController())return false;
 auto* Mesh=Cast<USkeletalMesh>(Source->Rider->GetSkinnedAsset());
 const bool Detailed=Mesh&&Mesh->GetRefSkeleton().FindBoneIndex(TEXT("pelvis"))>=0;
 auto* Asset=Detailed?Mesh->GetPhysicsAsset():LoadObject<UPhysicsAsset>(nullptr,TEXT("/Game/BattleForTheA/Rider/Physics/PA_EllisonCrashCandidateV6.PA_EllisonCrashCandidateV6"));
 HipBone=Detailed?TEXT("pelvis"):TEXT("Hips");
 if(!Mesh||!Asset)return false;
 Bike=Source;SetOwner(Bike);Bike->RefreshRiderPose();
 Physics=NewObject<USkeletalMeshComponent>(this);AddInstanceComponent(Physics);SetRootComponent(Physics);
 Physics->SetDisablePostProcessBlueprint(true);Physics->SetSkeletalMeshAsset(Mesh);Physics->SetPhysicsAsset(Asset,true);Physics->SetWorldTransform(Bike->Rider->GetComponentTransform());
 Physics->SetCollisionProfileName(TEXT("Ragdoll"));Physics->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);Physics->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);Physics->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
 Physics->SetCanEverAffectNavigation(false);Physics->PhysicsTransformUpdateMode=EPhysicsTransformUpdateMode::ComponentTransformIsKinematic;Physics->RegisterComponent();Physics->RefreshBoneTransforms();Physics->SetAllBodiesSimulatePhysics(true);Physics->SetSimulatePhysics(true);
 const auto& Ref=Mesh->GetRefSkeleton();for(int I=0;I<Ref.GetNum();I++)if(auto* B=Physics->GetBodyInstance(Ref.GetBoneName(I))){B->SetBodyTransform(Bike->Rider->GetBoneTransform(I),ETeleportType::TeleportPhysics);B->SetUseCCD(true);}
 Display=NewObject<UPoseableMeshComponent>(this);AddInstanceComponent(Display);Display->SetSkinnedAssetAndUpdate(Mesh);Display->SetWorldTransform(Physics->GetComponentTransform());Display->SetCollisionEnabled(ECollisionEnabled::NoCollision);Display->RegisterComponent();
 Physics->SetVisibility(false);Physics->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
 Physics->SetAllPhysicsLinearVelocity(Velocity+FVector(0,0,60));MirrorPose();
 Fallen=GetWorld()->SpawnActor<ABattleFallenBike>();if(!Fallen||!Fallen->InitializeFrom(Bike,Velocity*.75f)){Destroy();return false;}
 Camera=GetWorld()->SpawnActor<ACameraActor>();
 if(Camera){const FVector Focus=Display->GetSocketLocation(HipBone);Camera->SetActorLocation(Focus+FVector(-320,-420,300));}
 Bike->bCrashActive=true;Bike->bParked=true;Bike->Rider->SetVisibility(false,true);Bike->Ride->Pedal=Bike->Ride->Steer=Bike->Ride->Brake=0;Bike->Ride->DisableMovement();
 Bike->AttachDetailedRiderParts(Display,true);
 ABattleSpirit::CancelForRider(Bike);
 UE_LOG(LogTemp,Display,TEXT("PlayerCrashLive: started speed=%.1f"),Velocity.Size());return true;
}
void ABattlePlayerCrash::MirrorPose(){
 const auto& Ref=Physics->GetSkeletalMeshAsset()->GetRefSkeleton();TArray<FTransform> World,Local;
 if(auto* Hip=Physics->GetBodyInstance(HipBone))Display->SetWorldLocation(Hip->GetUnrealWorldTransform().GetLocation()-FVector(0,0,90));
 const FTransform Frame=Display->GetComponentTransform();
 for(int I=0;I<Ref.GetNum();I++){const int Parent=Ref.GetParentIndex(I);FTransform T;
  if(auto* B=Physics->GetBodyInstance(Ref.GetBoneName(I))){T=B->GetUnrealWorldTransform();T.SetScale3D(B->Scale3D);}else T=Parent>=0?Ref.GetRefBonePose()[I]*World[Parent]:Ref.GetRefBonePose()[I]*Frame;
  World.Add(T);Local.Add(Parent>=0?T.GetRelativeTransform(World[Parent]):T.GetRelativeTransform(Frame));
 }Display->BoneSpaceTransforms=Local;Display->MarkRefreshTransformDirty();Display->RefreshBoneTransforms();Display->UpdateBounds();
}
void ABattlePlayerCrash::Tick(float Dt){
 Super::Tick(Dt);if(!IsValid(Bike)){Destroy();return;}
 if(bRecovered){if(IsValid(Fallen))Bike->SetActorLocation(Fallen->GetActorLocation(),false,nullptr,ETeleportType::TeleportPhysics);return;}
 if(const auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));Mode&&(Mode->bRunEnded||Mode->StartCountdown>0))return;
 if(Bike->RiderHealth<=0){Destroy();return;}
 auto* PC=Cast<APlayerController>(Bike->GetController());if(!PC){Destroy();return;}
 Clock+=Dt;
 if(!bGettingUp){MirrorPose();Settled=Physics->GetPhysicsLinearVelocity(HipBone).Size()<100?Settled+Dt:0;
  if(Clock>1.2f&&Settled>.5f){bGettingUp=Recovery.Begin(Bike,Physics,Display);if(!bGettingUp)Settled=0;else Bike->AttachDetailedRiderParts(Recovery.Pose.Get(),true);}
 }else if(Recovery.Tick(Dt)&&FinishRecovery(Dt))return;
 const FVector Hip=bGettingUp&&Recovery.Pose.IsValid()?Recovery.Pose->GetSocketLocation(HipBone):Display->GetSocketLocation(HipBone);
 // Keep the player target at the body so enemies pursue the fallen rider.
 Bike->SetActorLocation(Hip+FVector(0,0,25),false,nullptr,ETeleportType::TeleportPhysics);
 if(Camera){UpdateBattleCrashCamera(Camera,Bike,Fallen,Hip+FVector(0,0,25),Dt);PC->SetViewTarget(Camera);}
}
UPoseableMeshComponent* ABattlePlayerCrash::GetRecoveryPose() const{return Recovery.Pose.Get();}
float ABattlePlayerCrash::GetRecoveryTime() const{return Recovery.Clock;}
bool ABattlePlayerCrash::FinishRecovery(float Dt){
 auto* PC=Cast<APlayerController>(Bike->GetController());if(!PC||!Recovery.Pose.IsValid())return false;
 FVector Position=Recovery.Pose->GetComponentLocation()+FVector(0,0,90);const FRotator Rotation(0,Recovery.Pose->GetComponentRotation().Yaw+90,0);
 FCollisionQueryParams Q;Q.AddIgnoredActor(Bike);Q.AddIgnoredActor(this);if(Fallen)Q.AddIgnoredActor(Fallen);
 const auto Shape=FCollisionShape::MakeCapsule(30,88);
 auto Occupied=[&](const FVector& P){return GetWorld()->OverlapBlockingTestByChannel(P,FQuat::Identity,ECC_Pawn,Shape,Q);};
 FCollisionObjectQueryParams Geometry;Geometry.AddObjectTypesToQuery(ECC_WorldStatic);Geometry.AddObjectTypesToQuery(ECC_WorldDynamic);Geometry.AddObjectTypesToQuery(ECC_PhysicsBody);
 auto ClearPath=[&](const FVector& A,const FVector& B){FHitResult Hit;return !GetWorld()->SweepSingleByObjectType(Hit,A,B,FQuat::Identity,Geometry,Shape,Q);};
 if(Occupied(Position)&&!bExitReposition){
  if(Clock<NextExitSearch)return false;NextExitSearch=Clock+.25f;
  // A crowd member may enter after the original get-up clearance check. Resolve
  // locally, preserving solid geometry and floor support rather than teleporting.
  for(float Radius:{70.f,110.f})for(int Direction=0;Direction<8&&!bExitReposition;Direction++){
   const float Angle=Direction*PI/4;const FVector Probe=Position+FVector(FMath::Cos(Angle)*Radius,FMath::Sin(Angle)*Radius,0);FHitResult Floor;
   FCollisionObjectQueryParams Ground;Ground.AddObjectTypesToQuery(ECC_WorldStatic);
   if(!GetWorld()->LineTraceSingleByObjectType(Floor,Probe+FVector(0,0,30),Probe-FVector(0,0,130),Ground,Q)||Floor.ImpactNormal.Z<.65f)continue;
   const FVector Candidate=Floor.ImpactPoint+FVector(0,0,90);
   if(FMath::Abs(Candidate.Z-Position.Z)>25||Occupied(Candidate)||!ClearPath(Position,Candidate))continue;
   ExitTarget=Candidate;bExitReposition=true;UE_LOG(LogTemp,Display,TEXT("PlayerRecoverySpace: moving_to_clearance_cm=%.2f"),FVector::Dist(Position,Candidate));
  }
  if(!bExitReposition)return false;
 }
 if(bExitReposition){
  if(Occupied(ExitTarget)||!ClearPath(Position,ExitTarget)){bExitReposition=false;return false;}
  const FVector Next=FMath::VInterpConstantTo(Position,ExitTarget,Dt,140.f);
  Recovery.Pose->SetWorldLocation(Next-FVector(0,0,90));Position=Next;
  if(!Position.Equals(ExitTarget,.1f))return false;bExitReposition=false;
  if(Occupied(Position))return false;
 }

 FActorSpawnParameters Params;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
 auto* Person=GetWorld()->SpawnActor<ABattleRider>(Position,Rotation,Params);if(!Person)return false;
 Person->ParkedBike=Bike;Person->Health=Bike->RiderHealth;Person->RestoreLoadout();Person->GetCapsuleComponent()->IgnoreActorWhenMoving(Bike,true);
 PC->Possess(Person);PC->SetControlRotation(Rotation);PC->SetViewTargetWithBlend(Person,.3f);
 Bike->bCrashActive=false;Bike->Ride->Recovery=0;Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;
 if(Fallen)Bike->SetActorLocation(Fallen->GetActorLocation(),false,nullptr,ETeleportType::TeleportPhysics);
 Bike->AttachDetailedRiderParts(Bike->Rider,false);Recovery.Reset();Display->DestroyComponent();Physics->DestroyComponent();if(Camera){Camera->SetLifeSpan(.4f);Camera=nullptr;}
 bRecovered=true;UE_LOG(LogTemp,Display,TEXT("PlayerCrashLive: recovered on foot health=%.1f ammo=%d"),Person->Health,Person->Ammo);return true;
}
bool ABattlePlayerCrash::PrepareRemount(ABattleRider* Person){
 if(!bRecovered||!IsValid(Fallen)||!IsValid(Person)||FVector::Dist(Person->GetActorLocation(),Fallen->GetActorLocation())>240)return false;
 const FVector Base=Fallen->GetActorLocation();FCollisionQueryParams Q;Q.AddIgnoredActor(this);Q.AddIgnoredActor(Bike);Q.AddIgnoredActor(Fallen);Q.AddIgnoredActor(Person);
 for(const FVector Offset:{FVector::ZeroVector,FVector(140,0,0),FVector(-140,0,0),FVector(0,140,0),FVector(0,-140,0),FVector(220,0,0),FVector(-220,0,0),FVector(0,220,0),FVector(0,-220,0)}){
  FHitResult Floor;const FVector Probe=Base+Offset;
  if(!GetWorld()->LineTraceSingleByChannel(Floor,Probe+FVector(0,0,150),Probe-FVector(0,0,250),ECC_Visibility,Q)||Floor.ImpactNormal.Z<.65f)continue;
  const FVector Position=Floor.ImpactPoint+FVector(0,0,98);
  if(FVector::Dist(Position,Person->GetActorLocation())>240||GetWorld()->OverlapBlockingTestByChannel(Position,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,96),Q))continue;
  Bike->SetActorLocationAndRotation(Position,FRotator(0,Person->GetActorRotation().Yaw,0),false,nullptr,ETeleportType::TeleportPhysics);return true;
 }
 UE_LOG(LogTemp,Display,TEXT("PlayerCrashRemount: no clear upright spot bike=%s rider=%s"),*Base.ToString(),*Person->GetActorLocation().ToString());return false;
}
float ABattlePlayerCrash::TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer){return IsValid(Bike)&&!bRecovered?Bike->ApplyRiderDamage(Amount):0;}
void ABattlePlayerCrash::EndPlay(const EEndPlayReason::Type Reason){
 if(IsValid(Bike))Bike->AttachDetailedRiderParts(Bike->Rider,!Bike->bParked&&!Bike->bFirstPerson);
 Recovery.Reset();if(IsValid(Fallen))Fallen->Destroy();
 if(IsValid(Camera)){if(IsValid(Bike))if(auto* PC=Cast<APlayerController>(Bike->GetController()))PC->SetViewTarget(Bike);Camera->Destroy();}
 if(IsValid(Bike)){Bike->bCrashActive=false;Bike->Rider->SetVisibility(!Bike->bParked&&!Bike->bFirstPerson,true);}
 Super::EndPlay(Reason);
}

bool ABattleBike::StartPhysicalCrash(const FVector& Velocity){
 if(IsValid(PlayerCrash)||bParked||RiderHealth<=0)return false;
 auto* Crash=GetWorld()->SpawnActor<ABattlePlayerCrash>();if(!Crash)return false;
 if(!Crash->Start(this,Velocity)){Crash->Destroy();return false;}PlayerCrash=Crash;ReloadTimer=0;GunHold=0;return true;
}
void ABattleBike::ClearPhysicalCrash(){if(IsValid(PlayerCrash))PlayerCrash->Destroy();PlayerCrash=nullptr;bCrashActive=false;}
