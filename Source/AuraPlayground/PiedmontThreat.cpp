#include "PiedmontThreat.h"
#include "Engine/World.h"
#include "PiedmontBike.h"
#include "PiedmontCombat.h"
#include "PiedmontBlood.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "DrawDebugHelpers.h"
#include "Sound/SoundBase.h"
#include "Engine/StaticMesh.h"
#include "Components/CapsuleComponent.h"
APiedmontThreat::APiedmontThreat(){
 GetCharacterMovement()->bRunPhysicsWithNoController=true;GetCharacterMovement()->MaxWalkSpeed=450;
 Blade=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Knife"));Blade->SetupAttachment(GetCapsuleComponent());Blade->SetRelativeLocation(FVector(48,15,42));Blade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 bUseControllerRotationYaw=false;Tags.Add(TEXT("PiedmontHostile"));
}
void APiedmontThreat::BeginPlay(){
 Super::BeginPlay();
 if(auto* Mesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/PiedmontRide/Bike/SM_Knife.SM_Knife"),nullptr,LOAD_NoWarn)){Blade->SetStaticMesh(Mesh);Blade->SetRelativeScale3D(FVector(27.f/FMath::Max(1.f,Mesh->GetBounds().BoxExtent.Z*2)));}
}
bool APiedmontThreat::CanUseWeapon() const{return !bDead;}
void APiedmontThreat::Tick(float Dt){
 Super::Tick(Dt);
 if(bDead){Blade->SetVisibility(false);DeadTime+=Dt;Body->SetRelativeRotation(FRotator(0,-90,FMath::Min(90.f,DeadTime*180)));Body->SetRelativeLocation(FVector(0,0,-85));return;}
 auto* Mode=Cast<APiedmontRideMode>(UGameplayStatics::GetGameMode(this));if(!Mode||Mode->bRunEnded)return;
 auto* Target=UGameplayStatics::GetPlayerPawn(this,0);if(!Target)return;
 const FVector Delta=Target->GetActorLocation()-GetActorLocation();const float Distance=Delta.Size2D();
 SetActorRotation(FRotator(0,Delta.Rotation().Yaw,0));bWeaponDrawn=true;Weapon->SetVisibility(bGunman);Blade->SetVisibility(!bGunman);Blade->SetRelativeLocation(FVector(bWindingUp?40:48,15,42));
 FCollisionQueryParams Q(SCENE_QUERY_STAT(PiedmontThreatSight),true,this);FHitResult Sight;
 const FVector Eye=GetActorLocation()+FVector(0,0,45);
 const bool Blocked=GetWorld()->LineTraceSingleByChannel(Sight,Eye,Target->GetActorLocation()+FVector(0,0,25),ECC_Visibility,Q)&&Sight.GetActor()!=Target;
 AttackCooldown=FMath::Max(0.f,AttackCooldown-Dt);
 if(bWindingUp){
  Windup-=Dt;if(Windup>0)return;bWindingUp=false;AttackCooldown=bGunman?2.4f:1.4f;Attacks++;
  if(bGunman){
   // Aim is locked at windup start so a moving player can evade.
   const FVector End=AimPoint+FMath::VRand()*75;FHitResult Hit;GetWorld()->LineTraceSingleByChannel(Hit,Eye,End,ECC_Visibility,Q);
   DrawDebugLine(GetWorld(),Eye,Hit.bBlockingHit?Hit.ImpactPoint:End,FColor::Orange,false,.12f,0,2);
   if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/PiedmontRide/Audio/S_Gunshot.S_Gunshot"),nullptr,LOAD_NoWarn))UGameplayStatics::PlaySoundAtLocation(this,Sound,Eye);
   if(Hit.GetActor()==Target)UGameplayStatics::ApplyPointDamage(Target,1,(End-Eye).GetSafeNormal(),Hit,nullptr,this,UPiedmontBulletDamage::StaticClass());
  }else if(Distance<130&&!Blocked)UGameplayStatics::ApplyDamage(Target,1,nullptr,this,UPiedmontKnifeDamage::StaticClass());
  return;
 }
 const bool InRange=bGunman?Distance<2200:Distance<105;
 if(InRange&&!Blocked&&AttackCooldown<=0){bWindingUp=true;Windup=bGunman?1.2f:.65f;AimPoint=Target->GetActorLocation()+FVector(0,0,25);GetCharacterMovement()->StopMovementImmediately();return;}
 if(!InRange||Blocked){
  FVector Direction=Delta.GetSafeNormal2D();FHitResult Front;
  if(GetWorld()->LineTraceSingleByChannel(Front,GetActorLocation(),GetActorLocation()+Direction*100,ECC_Visibility,Q)&&Front.GetActor()!=Target)Direction=GetActorRightVector();
  AddMovementInput(Direction,1);
 }
}
float APiedmontThreat::TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer){
 if(bDead||Amount<=0)return 0;
 Health-=Amount;APiedmontBlood::Burst(GetWorld(),GetActorLocation()+FVector(0,0,35),Causer?(GetActorLocation()-Causer->GetActorLocation()).GetSafeNormal():FVector::UpVector);
 if(Health<=0){bDead=true;bWindingUp=false;GetCharacterMovement()->StopMovementImmediately();GetCharacterMovement()->DisableMovement();Weapon->SetVisibility(false);SetLifeSpan(15);}
 return Amount;
}
