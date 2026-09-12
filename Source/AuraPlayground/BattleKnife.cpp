#include "BattleKnife.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "PiedmontBlood.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
ABattleKnife::ABattleKnife(){
 PrimaryActorTick.TickGroup=TG_PostPhysics;
 static ConstructorHelpers::FObjectFinder<UPhysicsAsset> Collision(TEXT("/Game/BattleForTheA/Rider/Physics/PA_EllisonCrashCandidateV6.PA_EllisonCrashCandidateV6"));DeathAsset=Collision.Object;
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Top(TEXT("/Game/BattleForTheA/Hostiles/Knife/M_KnifeTop.M_KnifeTop"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Shorts(TEXT("/Game/BattleForTheA/Hostiles/Knife/M_KnifeShorts.M_KnifeShorts"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Hair(TEXT("/Game/BattleForTheA/Hostiles/Knife/M_KnifeHair.M_KnifeHair"));
 if(Top.Succeeded())Body->SetMaterial(Body->GetMaterialIndex(TEXT("Purple")),Top.Object);
 if(Shorts.Succeeded())Body->SetMaterial(Body->GetMaterialIndex(TEXT("LightBlue")),Shorts.Object);
 if(Hair.Succeeded())Body->SetMaterial(Body->GetMaterialIndex(TEXT("Hair")),Hair.Object);
 Tags.Add(TEXT("PiedmontHostile"));Tags.Add(TEXT("BattleHostile"));Tags.Add(TEXT("BattleKnife"));
 AIControllerClass=AAIController::StaticClass();AutoPossessAI=EAutoPossessAI::PlacedInWorldOrSpawned;bUseControllerRotationYaw=false;
 GetCharacterMovement()->bOrientRotationToMovement=true;GetCharacterMovement()->MaxWalkSpeed=680;GetCharacterMovement()->RotationRate=FRotator(0,420,0);
 Blade=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Knife"));Blade->SetupAttachment(GetCapsuleComponent());Blade->SetCollisionEnabled(ECollisionEnabled::NoCollision);Blade->SetCanEverAffectNavigation(false);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Knife(TEXT("/Game/PiedmontRide/Bike/SM_Knife.SM_Knife"));Blade->SetStaticMesh(Knife.Object);
}
void ABattleKnife::BeginPlay(){Super::BeginPlay();Weapon->SetVisibility(false);}
float ABattleKnife::SpawnChance(bool HasPhone,float Trouble){return (HasPhone?.15f:.03f)+FMath::Clamp(Trouble,0.f,12.f)*(.10f/12.f);}
bool ABattleKnife::CanReach(APawn* Target) const {
 if(!Target)return false;const FVector D=Target->GetActorLocation()-GetActorLocation();if(D.Size2D()>145||FMath::Abs(D.Z)>110)return false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(KnifeReach),false,this);FHitResult Hit;
 return !GetWorld()->LineTraceSingleByChannel(Hit,GetActorLocation()+FVector(0,0,30),Target->GetActorLocation()+FVector(0,0,20),ECC_Visibility,Q)||Hit.GetActor()==Target;
}
FVector ABattleKnife::AdjustVisitorHand(int32 Side,FVector Target) const {
 if(Side==1||bDead||bEscaped)return Target;
 if(StrikePose>0)return FMath::Lerp(FVector(-25,10,110),FVector(-18,58,118),FMath::Sin(FMath::Clamp(1-StrikePose/.35f,0.f,1.f)*PI));
 if(bWindingUp)return FMath::Lerp(FVector(-25,10,110),FVector(-24,-5,145),FMath::Clamp(1-WindupRemaining/.85f,0.f,1.f));
 return FMath::Lerp(Target,FVector(-25,12,100),.65f);
}
void ABattleKnife::Escape(){bEscaped=true;bWindingUp=false;StrikePose=0;if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();SetLifeSpan(5);}
void ABattleKnife::OnRemounted(ABattleBike* Bike){if(!Bike)return;for(TActorIterator<ABattleKnife> It(Bike->GetWorld());It;++It)if(It->Stabs>0&&It->VictimBike==Bike&&!It->bDead)It->Escape();}
bool ABattleKnife::ResolveStrike(){
 auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* P=UGameplayStatics::GetPlayerPawn(this,0);
 if(bDead||bEscaped||bSwimming||!bWindingUp||WindupRemaining>0||!M||M->bTutorialActive||M->StartCountdown>0||M->bRunEnded||UGameplayStatics::IsGamePaused(this))return false;
 bWindingUp=false;StrikePose=.35f;Cooldown=Stabs==0?3.5f:2;
 if(!CanReach(P))return false;
 auto* B=Cast<ABattleBike>(P);if(auto* Foot=Cast<ABattleRider>(P)){if(Foot->bSwimming)return false;B=Foot->ParkedBike;}
 if(!B||!B->ApplyKnifeStab(Stabs>0))return false;
 VictimBike=B;Stabs++;APiedmontBlood::Burst(GetWorld(),P->GetActorLocation()+FVector(0,0,15),(P->GetActorLocation()-GetActorLocation()).GetSafeNormal());
 if(B->RiderHealth<=0)Escape();return true;
}
void ABattleKnife::Tick(float Dt){
 Super::Tick(Dt);Weapon->SetVisibility(false);
 if(bDead){Blade->SetVisibility(false);if(DeathPhysics){MirrorDeathPose();return;}DeathTime+=Dt;Body->SetRelativeRotation(FRotator(0,-90,FMath::Min(90.f,DeathTime*120)));return;}
 StrikePose=FMath::Max(0.f,StrikePose-Dt);
 const FVector Direction=bWindingUp?FVector::UpVector:GetActorForwardVector();const FVector Hand=Body->GetBoneLocationByName(TEXT("Hand_R"),EBoneSpaces::WorldSpace);
 Blade->SetWorldLocationAndRotation(Hand+Direction*5,FQuat::FindBetweenNormals(FVector::UpVector,Direction));
 auto* AI=Cast<AAIController>(GetController());auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* P=UGameplayStatics::GetPlayerPawn(this,0);
 if(bEscaped||bSwimming||!M||!P||M->bTutorialActive||M->StartCountdown>0||M->bRunEnded){bWindingUp=false;if(AI)AI->StopMovement();return;}
 auto* B=Cast<ABattleBike>(P);if(auto* Foot=Cast<ABattleRider>(P)){if(Foot->bSwimming){Escape();return;}B=Foot->ParkedBike;}
 if(!B||B->RiderHealth<=0||B->RespawnRemaining>0){Escape();return;}
 // Keep the attacker clear of the get-up pose, then allow a reaction window.
 if(B->bCrashActive){bWindingUp=false;Cooldown=FMath::Max(Cooldown,1.5f);if(AI)AI->StopMovement();return;}
 const float Distance=FVector::Dist2D(P->GetActorLocation(),GetActorLocation());FarTime=Distance>2200?FarTime+Dt:0;if(FarTime>5){Escape();return;}
 Cooldown=FMath::Max(0.f,Cooldown-Dt);PathDelay-=Dt;
 if(bWindingUp){if(AI)AI->StopMovement();SetActorRotation(FRotator(0,(P->GetActorLocation()-GetActorLocation()).Rotation().Yaw,0));WindupRemaining-=Dt;if(WindupRemaining<=0)ResolveStrike();return;}
 if(Cooldown<=0&&B->StunRemaining<=0&&B->DamageGrace<=0&&CanReach(P)){bWindingUp=true;WindupRemaining=.85f;if(AI)AI->StopMovement();return;}
 if(AI&&PathDelay<=0){PathDelay=.4f;PathRequests++;AI->MoveToActor(P,70,true,true,true,nullptr,true);}
}
float ABattleKnife::TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer){
 if(bDead||!FMath::IsFinite(Amount)||Amount<=0)return 0;
 const float Applied=FMath::Min(Health,Amount);Health-=Applied;bWindingUp=false;Cooldown=FMath::Max(Cooldown,.7f);
 APiedmontBlood::Burst(GetWorld(),GetActorLocation()+FVector(0,0,30),Causer?(GetActorLocation()-Causer->GetActorLocation()).GetSafeNormal():FVector::UpVector);
 if(Health<=0){bDead=true;if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();GetCharacterMovement()->DisableMovement();GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);Blade->SetVisibility(false);BeginDeathPhysics(Causer?(GetActorLocation()-Causer->GetActorLocation()).GetSafeNormal2D():GetActorForwardVector());SetLifeSpan(8);}
 return Applied;
}
