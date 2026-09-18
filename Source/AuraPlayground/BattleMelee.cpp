#include "BattleRider.h"
#include "BattleBike.h"
#include "BattleZombie.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

void ABattleRider::BuildMeleeVisual(){
 MeleeRoot=CreateDefaultSubobject<USceneComponent>(TEXT("ChainLock"));MeleeRoot->SetupAttachment(Camera);MeleeRoot->SetVisibility(false,true);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 int32 Part=0;
 auto Rod=[&](FVector A,FVector B,float Radius){
  auto* M=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("LockPart%d"),Part++));M->SetupAttachment(MeleeRoot);M->SetStaticMesh(Cylinder.Object);
  M->SetRelativeLocation((A+B)*.5f);M->SetRelativeRotation(FQuat::FindBetweenNormals(FVector::UpVector,(B-A).GetSafeNormal()));M->SetRelativeScale3D(FVector(Radius/50,Radius/50,(B-A).Size()/100));
  M->SetCollisionEnabled(ECollisionEnabled::NoCollision);M->SetCanEverAffectNavigation(false);M->SetCastShadow(false);M->SetOnlyOwnerSee(true);M->SetVisibility(false);
 };
 // A steel U shackle with a thick locking bar, suspended from interlocking links.
 Rod(FVector(0,-7,0),FVector(0,-7,15),1.25f);Rod(FVector(0,7,0),FVector(0,7,15),1.25f);Rod(FVector(0,-9,0),FVector(0,9,0),2.4f);
 for(int32 I=0;I<12;I++){const float A=PI*I/12,B=PI*(I+1)/12;Rod(FVector(0,7*FMath::Cos(A),15+7*FMath::Sin(A)),FVector(0,7*FMath::Cos(B),15+7*FMath::Sin(B)),1.25f);}
 for(int32 I=0;I<6;I++)for(int32 J=0;J<8;J++){
  const float A=2*PI*J/8,B=2*PI*(J+1)/8,Z=-4-I*5;
  auto P=[&](float T){return I%2?FVector(2*FMath::Cos(T),0,Z+3.5f*FMath::Sin(T)):FVector(0,2*FMath::Cos(T),Z+3.5f*FMath::Sin(T));};Rod(P(A),P(B),.38f);
 }
}
bool ABattleRider::Melee(){
 if(!CanUseWeapon()||UGameplayStatics::IsGamePaused(this)||MeleeRemaining>0||ReloadRemaining>0)return false;
 MeleeRemaining=.7f;bMeleeResolved=false;MeleeSwings++;bAiming=false;
 if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_LockSwing.S_LockSwing")))UGameplayStatics::PlaySoundAtLocation(this,Sound,GetActorLocation());
 return true;
}
void ABattleRider::UpdateMelee(float Dt){
 if(!CanUseWeapon()){MeleeRemaining=0;bMeleeResolved=true;}
 if(MeleeRemaining>0){
  MeleeRemaining=FMath::Max(0.f,MeleeRemaining-Dt);
  if(!bMeleeResolved&&MeleeRemaining<=.48f){bMeleeResolved=true;ResolveMelee();}
  const float T=1-MeleeRemaining/.7f;const float Swing=FMath::Sin(PI*T);
  MeleeRoot->SetRelativeLocation(FVector(55+32*Swing,28-65*Swing,-20+22*Swing));MeleeRoot->SetRelativeRotation(FRotator(-35+90*Swing,65-150*T,20+35*Swing));
 }
 MeleeRoot->SetVisibility(MeleeRemaining>0,true);if(MeleeRemaining>0){Weapon->SetVisibility(false);bAiming=false;}
}
void ABattleRider::ResolveMelee(){
 auto* PC=Cast<APlayerController>(GetController());if(!PC)return;
 FVector Eye;FRotator View;PC->GetPlayerViewPoint(Eye,View);
 // The chain lock is swung from Ellison's own hands, so the sweep has to start
 // at his body, not at the chase camera. PiedmontExplorer parks that camera 320 cm
 // behind him, so sweeping 185 cm from the eye could only ever reach things
 // behind his back - a target standing right in front of him was unhittable.
 // The aim direction still comes from the view, so the swing follows the look.
 const FVector Start=GetActorLocation(),End=Start+View.Vector()*185.f;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(BattleChainLock),false,this);if(ParkedBike)Q.AddIgnoredActor(ParkedBike);
 FHitResult Hit;if(!GetWorld()->SweepSingleByChannel(Hit,Start,End,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(28),Q))return;
 auto* Victim=Cast<APiedmontExplorer>(Hit.GetActor());const bool Alive=Victim&&!Victim->bDead;
 const float Damage=UGameplayStatics::ApplyDamage(Hit.GetActor(),50,PC,this,UDamageType::StaticClass());
 if(Damage>0&&Alive){
  MeleeHits++;HitFeedback=.3f;
  if(auto* Z=Cast<ABattleZombie>(Victim)){if(!Z->bDead){Z->bTelegraphing=false;Z->LaunchCharacter(View.Vector()*330+FVector(0,0,80),true,true);}}
  if(Victim->bDead&&Victim->ActorHasTag(TEXT("PiedmontHostile"))&&ParkedBike)ParkedBike->AwardEnemyKill();
 }
 if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_LockHit.S_LockHit")))UGameplayStatics::PlaySoundAtLocation(this,Sound,Hit.ImpactPoint);
 PC->AddPitchInput(-.8f);
}
