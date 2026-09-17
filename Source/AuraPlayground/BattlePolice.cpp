#include "BattlePolice.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Animation/AnimSequence.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleShot.h"
#include "PiedmontBlood.h"
#include "AIController.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
ABattlePolice::ABattlePolice(){
 WarningVoice=CreateDefaultSubobject<UAudioComponent>(TEXT("WarningVoice"));
 WarningVoice->SetupAttachment(GetCapsuleComponent());WarningVoice->SetRelativeLocation(FVector(0,0,60));
 WarningVoice->bAutoActivate=false;WarningVoice->bIsUISound=false;WarningVoice->bOverrideAttenuation=true;
 auto& Attenuation=WarningVoice->AttenuationOverrides;Attenuation.bAttenuate=true;Attenuation.bSpatialize=true;
 Attenuation.AttenuationShapeExtents=FVector(300,0,0);Attenuation.FalloffDistance=1800;
 Attenuation.bEnableOcclusion=true;Attenuation.OcclusionVolumeAttenuation=.35f;Attenuation.OcclusionLowPassFilterFrequency=1000;
 AimBeam=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TaserAimBeam"));AimBeam->SetupAttachment(GetCapsuleComponent());
 static ConstructorHelpers::FObjectFinder<UStaticMesh> BeamMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));AimBeam->SetStaticMesh(BeamMesh.Object);
 AimBeam->SetCollisionEnabled(ECollisionEnabled::NoCollision);AimBeam->SetCastShadow(false);AimBeam->SetCanEverAffectNavigation(false);AimBeam->SetVisibility(false);
 Tags.Add(TEXT("BattlePolice"));Tags.Add(TEXT("BattleHostile"));
 AIControllerClass=AAIController::StaticClass();AutoPossessAI=EAutoPossessAI::PlacedInWorldOrSpawned;bUseControllerRotationYaw=false;
 GetCharacterMovement()->bOrientRotationToMovement=true;GetCharacterMovement()->MaxWalkSpeed=460;
 static ConstructorHelpers::FObjectFinder<USkeletalMesh> OfficerMesh(TEXT("/Game/BattleForTheA/Police/Swat.Swat"));
 Body->SetSkinnedAssetAndUpdate(OfficerMesh.Object);
 static ConstructorHelpers::FObjectFinder<UAnimSequence> Aim(TEXT("/Game/PiedmontRide/Rider/Animations/Animations_CharacterArmature_Idle_Gun_Shoot.Animations_CharacterArmature_Idle_Gun_Shoot"));TaserAim=Aim.Object;
 static ConstructorHelpers::FObjectFinder<UAnimSequence> Idle(TEXT("/Game/PiedmontRide/Rider/Animations/Animations_CharacterArmature_Idle.Animations_CharacterArmature_Idle"));TaserIdle=Idle.Object;
 static ConstructorHelpers::FObjectFinder<UAnimSequence> Walk(TEXT("/Game/PiedmontRide/Rider/Animations/Animations_CharacterArmature_Walk.Animations_CharacterArmature_Walk"));TaserWalk=Walk.Object;
 static ConstructorHelpers::FObjectFinder<UAnimSequence> Run(TEXT("/Game/PiedmontRide/Rider/Animations/Animations_CharacterArmature_Run.Animations_CharacterArmature_Run"));TaserRun=Run.Object;
 for(int32 Side:{-1,1}){auto* Label=CreateDefaultSubobject<UTextRenderComponent>(Side>0?TEXT("PoliceFront"):TEXT("PoliceBack"));Label->SetupAttachment(GetCapsuleComponent());Label->SetRelativeLocation(FVector(Side*22,0,44));Label->SetRelativeRotation(FRotator(0,Side>0?0:180,0));Label->SetText(FText::FromString(TEXT("POLICE")));Label->SetWorldSize(9);Label->SetHorizontalAlignment(EHTA_Center);Label->SetTextRenderColor(FColor::White);Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);}
 TauntLabel=CreateDefaultSubobject<UTextRenderComponent>(TEXT("MurderKTaunt"));TauntLabel->SetupAttachment(GetCapsuleComponent());TauntLabel->SetRelativeLocation(FVector(0,0,145));TauntLabel->SetHorizontalAlignment(EHTA_Center);TauntLabel->SetWorldSize(22);TauntLabel->SetTextRenderColor(FColor(255,190,80));TauntLabel->SetCollisionEnabled(ECollisionEnabled::NoCollision);TauntLabel->SetVisibility(false);
}
void ABattlePolice::BeginPlay(){
 Super::BeginPlay();
 WarningVoice->SetSound(LoadObject<USoundWave>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_APDStop.S_APDStop")));
 if(auto* Mesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/BattleForTheA/Police/Taser/Taser/StaticMeshes/Taser.Taser"))){Weapon->SetStaticMesh(Mesh);Weapon->SetRelativeScale3D(FVector(1));}
 if(auto* Material=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_Splash.M_Splash")))AimBeam->SetMaterial(0,Material);
 Weapon->SetVisibility(false);
 if(bAmbientMurderK){Tags.Remove(TEXT("BattleHostile"));Cooldown=BIG_NUMBER;Weapon->SetVisibility(false);}
}
FVector ABattlePolice::TaserMuzzle() const{return Weapon->GetComponentTransform().TransformPosition(FVector(17.5f,0,4));}
void ABattlePolice::UpdateTaserBeam(){
 const bool Visible=!bDead&&bWarning&&!WarningAimPoint.IsNearlyZero();AimBeam->SetVisibility(Visible);if(!Visible)return;
 const FVector Start=TaserMuzzle(),Delta=WarningAimPoint-Start;
 AimBeam->SetWorldLocation(Start+Delta*.5f);AimBeam->SetWorldRotation(FQuat::FindBetweenNormals(FVector::UpVector,Delta.GetSafeNormal()));
 AimBeam->SetWorldScale3D(FVector(.007f,.007f,FMath::Max(1.f,Delta.Size())/100.f));
}
// The officer rig calls its hand joints Wrist_L/R; its authored gun clip does
// not animate those tracks. Solve the arms and grip on this rig explicitly.
static void PosePoliceHands(UPoseableMeshComponent* Body,float Blend){
 if(Blend<=0)return;
 const auto& Ref=CastChecked<USkeletalMesh>(Body->GetSkinnedAsset())->GetRefSkeleton();
 TArray<FTransform> Pose=Body->BoneSpaceTransforms;
 for(int32 I=0;I<Pose.Num();I++)if(Ref.GetParentIndex(I)>=0)Pose[I]=Pose[I]*Pose[Ref.GetParentIndex(I)];
 auto Child=[&](int I,int Root){while(I>=0){if(I==Root)return true;I=Ref.GetParentIndex(I);}return false;};
 auto Move=[&](int Root,FVector Target,FQuat Rotation){if(Root<0)return;const FVector Old=Pose[Root].GetLocation();for(int I=Root;I<Pose.Num();I++)if(Child(I,Root)){Pose[I].SetLocation(Target+Rotation.RotateVector(Pose[I].GetLocation()-Old));Pose[I].SetRotation(Rotation*Pose[I].GetRotation());}};
 auto Rotate=[&](int Root,FQuat R){if(Root>=0)Move(Root,Pose[Root].GetLocation(),R);};
 for(const TCHAR* Side:{TEXT("R"),TEXT("L")}){
  auto Index=[&](const TCHAR* Prefix){return Ref.FindBoneIndex(*FString::Printf(TEXT("%s_%s"),Prefix,Side));};
  const bool Right=FCString::Strcmp(Side,TEXT("R"))==0;
  const int U=Index(TEXT("UpperArm")),L=Index(TEXT("LowerArm")),H=Index(TEXT("Wrist"));if(U<0||L<0||H<0)continue;
  const FVector Origin=Pose[U].GetLocation();
  FVector Target=FMath::Lerp(Pose[H].GetLocation(),Right?FVector(-14,43,128):FVector(-6,49,125),Blend);
  const float A=FVector::Dist(Origin,Pose[L].GetLocation()),B=FVector::Dist(Pose[L].GetLocation(),Pose[H].GetLocation());
  const FVector Direction=(Target-Origin).GetSafeNormal();const float D=FMath::Clamp(FVector::Dist(Origin,Target),FMath::Abs(A-B)+.1f,A+B-.1f);
  Target=Origin+Direction*D;const float Along=(A*A-B*B+D*D)/(2*D),Height=FMath::Sqrt(FMath::Max(0.f,A*A-Along*Along));
  const FVector Bend(Right?-1:1,0,-1),Joint=Origin+Direction*Along+FVector::VectorPlaneProject(Bend,Direction).GetSafeNormal()*Height;
  Move(U,Origin,FQuat::FindBetweenVectors(Pose[L].GetLocation()-Origin,Joint-Origin));
  Move(L,Joint,FQuat::FindBetweenVectors(Pose[H].GetLocation()-Pose[L].GetLocation(),Target-Joint));
  const int Middle=Index(TEXT("Middle1"));
  if(Middle>=0)Rotate(H,FQuat::Slerp(FQuat::Identity,FQuat::FindBetweenVectors(Pose[Middle].GetLocation()-Pose[H].GetLocation(),FVector(0,1,0)),Blend));
  for(const TCHAR* Finger:{TEXT("Index"),TEXT("Middle"),TEXT("Ring"),TEXT("Pinky")})for(int J=2;J<=4;J++){
   const int F=Ref.FindBoneIndex(*FString::Printf(TEXT("%s%d_%s"),Finger,J,Side));
   Rotate(F,FQuat(FVector::ForwardVector,FMath::DegreesToRadians((J==2?-40.f:J==3?-55.f:-30.f)*Blend)));
  }
 }
 for(int I=0;I<Pose.Num();I++)Body->BoneSpaceTransforms[I]=Ref.GetParentIndex(I)>=0?Pose[I].GetRelativeTransform(Pose[Ref.GetParentIndex(I)]):Pose[I];
 Body->MarkRefreshTransformDirty();Body->RefreshBoneTransforms();
}
void ABattlePolice::AnimateBody(float Dt){
 const bool Raised=!bDead&&(bWarning||DischargeRemaining>0);
 TaserDrawBlend=FMath::FInterpConstantTo(TaserDrawBlend,Raised?1.f:0.f,Dt,4.f);
 const TArray<FTransform> Previous=Body->BoneSpaceTransforms;
 SetLocomotionClips(Raised?TaserAim:TaserIdle,TaserWalk,TaserRun);
 Super::AnimateBody(Dt);
 PosePoliceHands(Body,TaserDrawBlend);
 if(Previous.Num()==Body->BoneSpaceTransforms.Num()&&Dt>0){
  const float Weight=1.f-FMath::Exp(-12.f*Dt);
  for(int32 I=0;I<Previous.Num();I++){FTransform P;P.Blend(Previous[I],Body->BoneSpaceTransforms[I],Weight);Body->BoneSpaceTransforms[I]=P;}
  Body->MarkRefreshTransformDirty();Body->RefreshBoneTransforms();
 }
 const FVector Hand=Body->GetSocketLocation(TEXT("Wrist_R"));
 FVector AimDirection=GetActorForwardVector();
 if(auto* Target=UGameplayStatics::GetPlayerPawn(this,0))AimDirection=(Target->GetActorLocation()-Hand).GetSafeNormal();
 const FQuat AimRotation=AimDirection.Rotation().Quaternion(),HolsterRotation=(GetActorRotation()+FRotator(-75,0,0)).Quaternion();
 const FQuat Rotation=FQuat::Slerp(HolsterRotation,AimRotation,TaserDrawBlend);
 const FVector Holster=GetActorTransform().TransformPosition(FVector(-4,24,-15));
 const FVector Held=Hand-AimRotation.RotateVector(FVector(-11,0,-6));
 Weapon->SetWorldLocationAndRotation(FMath::Lerp(Holster,Held,TaserDrawBlend),Rotation);
}
bool ABattlePolice::CanReachTarget(APawn* Target) const{
 if(!Target||FVector::Dist(GetActorLocation(),Target->GetActorLocation())>800)return false;
 FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(PoliceTaser),false,this);
 const FVector Shoulder=GetActorLocation()+FVector(0,0,40),Start=TaserDrawBlend>.8f?TaserMuzzle():Shoulder;
 // A device protruding through nearby cover must not bypass that cover.
 if(GetWorld()->LineTraceSingleByChannel(Hit,Shoulder,Start,ECC_Visibility,Q)&&Hit.GetActor()!=Target)return false;
 return !GetWorld()->LineTraceSingleByChannel(Hit,Start,Target->GetActorLocation(),ECC_Visibility,Q)||Hit.GetActor()==Target;
}
bool ABattlePolice::CanStartTaser(APawn* Target) const{
 if(!Target)return false;
 const float Distance=FVector::DistSquared2D(GetActorLocation(),Target->GetActorLocation());
 for(TActorIterator<ABattlePolice> It(GetWorld());It;++It){
  const auto* Other=*It;if(Other==this||Other->bDead||Other->IsActorBeingDestroyed())continue;
  // Police pursue as a group, but only one officer announces and aims a taser.
  // Give the closest officer priority so the warning beam is easy to locate.
  if(Other->bWarning)return false;
  if(Other->Cooldown<=0&&FVector::DistSquared2D(Other->GetActorLocation(),Target->GetActorLocation())+1.f<Distance)return false;
 }
 return true;
}
bool ABattlePolice::FireTaser(){
 auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));auto* Target=UGameplayStatics::GetPlayerPawn(this,0);
 if(bDead||!Mode||Mode->StartCountdown>0||Mode->bRunEnded||UGameplayStatics::IsGamePaused(this)||!CanReachTarget(Target))return false;
 auto* Bike=Cast<ABattleBike>(Target);if(auto* Person=Cast<ABattleRider>(Target))Bike=Person->ParkedBike;if(!Bike)return false;
 const FVector End=WarningAimPoint.IsNearlyZero()?Target->GetActorLocation():WarningAimPoint,Start=TaserMuzzle();LastTaserOrigin=Start;
 // The officer commits to the warned shot instead of snapping the taser onto
 // the player's latest position. A decisive lateral move, jump, or turn can miss.
 const float DodgeDistance=FMath::Sqrt(FMath::PointDistToSegmentSquared(Target->GetActorLocation(),Start,End));
 const bool Hit=DodgeDistance<=TaserHitRadiusCm&&Bike->ApplyTaser();DischargeRemaining=.65f;
 if(auto* FX=GetWorld()->SpawnActorDeferred<ABattleShotFX>(ABattleShotFX::StaticClass(),FTransform(Start),this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn)){
  FX->Start=Start;FX->End=End;FX->FinishSpawning(FTransform(Start));FX->SetLifeSpan(.65f);auto* M=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_Splash.M_Splash"));FX->Tracer->SetMaterial(0,M);FX->Flash->SetMaterial(0,M);FX->Light->SetLightColor(FLinearColor(.1,.5,1));
 }
 TaserShots++;Cooldown=TaserCooldownSeconds;bWarning=false;WarningAimPoint=FVector::ZeroVector;
 // A shot, hit or miss, opens a squad-wide escape window. Without this, a
 // second officer can begin charging immediately and make the first dodge moot.
 for(TActorIterator<ABattlePolice> It(GetWorld());It;++It)if(*It!=this&&!It->bDead){It->bWarning=false;It->WarningAimPoint=FVector::ZeroVector;It->Cooldown=FMath::Max(It->Cooldown,SquadRecoverySeconds);It->UpdateTaserBeam();}
 return Hit;
}
void ABattlePolice::Tick(float Dt){
 VoiceCooldown=FMath::Max(0.f,VoiceCooldown-Dt);
 DischargeRemaining=FMath::Max(0.f,DischargeRemaining-Dt);
 Super::Tick(Dt);
 if(bAmbientMurderK){
  Weapon->SetVisibility(false);bWarning=false;WarningAimPoint=FVector::ZeroVector;UpdateTaserBeam();
  auto* Target=UGameplayStatics::GetPlayerPawn(this,0);auto* AI=Cast<AAIController>(GetController());if(AI)AI->StopMovement();
  TauntVisible=FMath::Max(0.f,TauntVisible-Dt);TauntCooldown=FMath::Max(0.f,TauntCooldown-Dt);
  const float Distance=Target?FVector::Dist2D(Target->GetActorLocation(),GetActorLocation()):BIG_NUMBER;
  if(Target&&Distance<1500){SetActorRotation(FRotator(0,(Target->GetActorLocation()-GetActorLocation()).Rotation().Yaw,0));if(TauntCooldown<=0){
    static const TCHAR* Lines[]={TEXT("APD: KEEP IT MOVING!"),TEXT("APD: NICE BIKE, TOUR DE MIDTOWN!"),TEXT("APD: DON'T MAKE ME DO PAPERWORK!"),TEXT("APD: WATCH THE POTHOLES, HERO!")};
    TauntLabel->SetText(FText::FromString(Lines[TauntsHurled%UE_ARRAY_COUNT(Lines)]));TauntsHurled++;TauntVisible=2.4f;TauntCooldown=FMath::FRandRange(7.f,11.f);
   }}
  TauntLabel->SetVisibility(TauntVisible>0);if(TauntVisible>0&&Target)TauntLabel->SetWorldRotation((Target->GetActorLocation()-TauntLabel->GetComponentLocation()).Rotation());return;
 }
 Weapon->SetVisibility(!bDead);UpdateTaserBeam();

 if(bDead){WarningVoice->Stop();Body->SetRelativeRotation(FRotator(0,-90,90));UpdateTaserBeam();return;}
 auto* AI=Cast<AAIController>(GetController());auto* Target=UGameplayStatics::GetPlayerPawn(this,0);auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));
 if(!Target||!Mode||Mode->StartCountdown>0||Mode->bRunEnded){WarningVoice->Stop();bWarning=false;if(AI)AI->StopMovement();UpdateTaserBeam();return;}
 auto* Bike=Cast<ABattleBike>(Target);if(auto* Person=Cast<ABattleRider>(Target))Bike=Person->ParkedBike;
 if(!Bike||Bike->RiderHealth<=0||Bike->RespawnRemaining>0||Bike->TaserGrace>0){bWarning=false;if(AI)AI->StopMovement();Cooldown=FMath::Max(0.f,Cooldown-Dt);UpdateTaserBeam();return;}
 Cooldown=FMath::Max(0.f,Cooldown-Dt);PathDelay-=Dt;
 if(bWarning){if(AI)AI->StopMovement();if(WarningRemaining>TaserAimLockSeconds)WarningAimPoint=Target->GetActorLocation();SetActorRotation(FRotator(0,(WarningAimPoint-GetActorLocation()).Rotation().Yaw,0));
  if(!CanReachTarget(Target)){bWarning=false;Cooldown=3;UpdateTaserBeam();return;}
  WarningRemaining-=Dt;if(WarningRemaining<=0)FireTaser();UpdateTaserBeam();return;}
 if(Cooldown<=0&&CanReachTarget(Target)&&CanStartTaser(Target)){bWarning=true;WarningRemaining=TaserWarningSeconds;WarningAimPoint=Target->GetActorLocation();
  if(VoiceCooldown<=0&&WarningVoice->Sound){WarningVoice->Play();WarningVoiceStarts++;VoiceCooldown=6.f;}
  UpdateTaserBeam();return;}
 if(AI&&PathDelay<=0){PathDelay=.6f;AI->MoveToActor(Target,550,true,true,true,nullptr,true);}
}
float ABattlePolice::TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer){
 if(bDead||!FMath::IsFinite(Amount)||Amount<=0)return 0;const float Applied=FMath::Min(Health,Amount);Health-=Applied;
 APiedmontBlood::Burst(GetWorld(),GetActorLocation()+FVector(0,0,30),Causer?(GetActorLocation()-Causer->GetActorLocation()).GetSafeNormal():FVector::UpVector);
 bWarning=false;Cooldown=FMath::Max(Cooldown,.5f);
 if(Health<=0){WarningVoice->Stop();bDead=true;TInlineComponentArray<UTextRenderComponent*> Labels(this);for(auto* Label:Labels)Label->SetVisibility(false);if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();GetCharacterMovement()->DisableMovement();GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);SetLifeSpan(10);}
 return Applied;
}
