#include "BattlePolice.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleShot.h"
#include "PiedmontBlood.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
ABattlePolice::ABattlePolice(){
 Tags.Add(TEXT("BattlePolice"));Tags.Add(TEXT("BattleHostile"));
 AIControllerClass=AAIController::StaticClass();AutoPossessAI=EAutoPossessAI::PlacedInWorldOrSpawned;bUseControllerRotationYaw=false;
 GetCharacterMovement()->bOrientRotationToMovement=true;GetCharacterMovement()->MaxWalkSpeed=460;
 static ConstructorHelpers::FObjectFinder<USkeletalMesh> OfficerMesh(TEXT("/Game/BattleForTheA/Police/Swat.Swat"));
 Body->SetSkinnedAssetAndUpdate(OfficerMesh.Object);
 for(int32 Side:{-1,1}){auto* Label=CreateDefaultSubobject<UTextRenderComponent>(Side>0?TEXT("PoliceFront"):TEXT("PoliceBack"));Label->SetupAttachment(GetCapsuleComponent());Label->SetRelativeLocation(FVector(Side*22,0,44));Label->SetRelativeRotation(FRotator(0,Side>0?0:180,0));Label->SetText(FText::FromString(TEXT("POLICE")));Label->SetWorldSize(9);Label->SetHorizontalAlignment(EHTA_Center);Label->SetTextRenderColor(FColor::White);Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);}
}
void ABattlePolice::BeginPlay(){Super::BeginPlay();Weapon->SetVisibility(false);}
bool ABattlePolice::CanReachTarget(APawn* Target) const{
 if(!Target||FVector::Dist(GetActorLocation(),Target->GetActorLocation())>800)return false;
 FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(PoliceTaser),false,this);
 return !GetWorld()->LineTraceSingleByChannel(Hit,GetActorLocation()+FVector(0,0,40),Target->GetActorLocation(),ECC_Visibility,Q)||Hit.GetActor()==Target;
}
bool ABattlePolice::FireTaser(){
 auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));auto* Target=UGameplayStatics::GetPlayerPawn(this,0);
 if(bDead||!Mode||Mode->StartCountdown>0||Mode->bRunEnded||UGameplayStatics::IsGamePaused(this)||!CanReachTarget(Target))return false;
 auto* Bike=Cast<ABattleBike>(Target);if(auto* Person=Cast<ABattleRider>(Target))Bike=Person->ParkedBike;if(!Bike)return false;
 const FVector End=Target->GetActorLocation(),Start=GetActorLocation()+FVector(20,0,40);const bool Hit=Bike->ApplyTaser();
 if(auto* FX=GetWorld()->SpawnActorDeferred<ABattleShotFX>(ABattleShotFX::StaticClass(),FTransform(Start),this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn)){
  FX->Start=Start;FX->End=End;FX->FinishSpawning(FTransform(Start));FX->SetLifeSpan(.3f);auto* M=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_Splash.M_Splash"));FX->Tracer->SetMaterial(0,M);FX->Flash->SetMaterial(0,M);FX->Light->SetLightColor(FLinearColor(.1,.5,1));
 }
 TaserShots++;Cooldown=15;bWarning=false;return Hit;
}
void ABattlePolice::Tick(float Dt){
 Super::Tick(Dt);Weapon->SetVisibility(false);if(bDead){Body->SetRelativeRotation(FRotator(0,-90,90));return;}
 auto* AI=Cast<AAIController>(GetController());auto* Target=UGameplayStatics::GetPlayerPawn(this,0);auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));
 if(!Target||!Mode||Mode->StartCountdown>0||Mode->bRunEnded){if(AI)AI->StopMovement();return;}
 auto* Bike=Cast<ABattleBike>(Target);if(auto* Person=Cast<ABattleRider>(Target))Bike=Person->ParkedBike;
 if(!Bike||Bike->RiderHealth<=0||Bike->RespawnRemaining>0||Bike->TaserGrace>0){bWarning=false;if(AI)AI->StopMovement();Cooldown=FMath::Max(0.f,Cooldown-Dt);return;}
 Cooldown=FMath::Max(0.f,Cooldown-Dt);PathDelay-=Dt;
 if(bWarning){if(AI)AI->StopMovement();SetActorRotation(FRotator(0,(Target->GetActorLocation()-GetActorLocation()).Rotation().Yaw,0));
  if(!CanReachTarget(Target)){bWarning=false;Cooldown=3;return;}WarningRemaining-=Dt;if(WarningRemaining<=0)FireTaser();return;}
 if(Cooldown<=0&&CanReachTarget(Target)){bWarning=true;WarningRemaining=1.25f;return;}
 if(AI&&PathDelay<=0){PathDelay=.6f;AI->MoveToActor(Target,550,true,true,true,nullptr,true);}
}
float ABattlePolice::TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer){
 if(bDead||!FMath::IsFinite(Amount)||Amount<=0)return 0;const float Applied=FMath::Min(Health,Amount);Health-=Applied;
 APiedmontBlood::Burst(GetWorld(),GetActorLocation()+FVector(0,0,30),Causer?(GetActorLocation()-Causer->GetActorLocation()).GetSafeNormal():FVector::UpVector);
 bWarning=false;Cooldown=FMath::Max(Cooldown,.5f);
 if(Health<=0){bDead=true;TInlineComponentArray<UTextRenderComponent*> Labels(this);for(auto* Label:Labels)Label->SetVisibility(false);if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();GetCharacterMovement()->DisableMovement();GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);SetLifeSpan(10);}
 return Applied;
}
