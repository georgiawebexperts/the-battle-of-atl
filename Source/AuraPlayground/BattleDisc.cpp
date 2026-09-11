#include "BattleDisc.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleZombie.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
ABattleDisc::ABattleDisc(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("DiscRoot"));
 Disc=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlyingDisc"));Disc->SetupAttachment(RootComponent);static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));Disc->SetStaticMesh(Cylinder.Object);Disc->SetRelativeScale3D(FVector(.28,.28,.035));Disc->SetCollisionEnabled(ECollisionEnabled::NoCollision);Disc->SetCanEverAffectNavigation(false);Disc->SetCastShadow(false);
}
void ABattleDisc::BeginPlay(){Super::BeginPlay();Disc->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_DiscGlow.M_DiscGlow")));}
ABattleDisc* ABattleDisc::Launch(APawn* Shooter,USceneComponent* Gun){
 auto* Person=Cast<ABattleRider>(Shooter);auto* PC=Person?Cast<APlayerController>(Person->GetController()):nullptr;if(!PC||!Person->ParkedBike||!Gun)return nullptr;
 FVector Eye;FRotator View;PC->GetPlayerViewPoint(Eye,View);const FVector Wanted=Gun->GetComponentLocation()+View.Vector()*35;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(DiscMuzzle),false,Shooter);Q.AddIgnoredActor(Person->ParkedBike);FHitResult Wall;
 // Never spawn a disc on the far side of a wall touching the weapon.
 const bool Blocked=Shooter->GetWorld()->SweepSingleByChannel(Wall,Eye,Wanted,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(12),Q);
 const FVector Start=Blocked?Eye:Wanted;
 auto* Projectile=Shooter->GetWorld()->SpawnActorDeferred<ABattleDisc>(StaticClass(),FTransform(Start),Person->ParkedBike,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
 if(!Projectile)return nullptr;Projectile->Bike=Person->ParkedBike;Projectile->LaunchPawn=Shooter;Projectile->Velocity=View.Vector()*2600;Projectile->FinishSpawning(FTransform(Start));
 if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_DiscLaunch.S_DiscLaunch")))UGameplayStatics::PlaySoundAtLocation(Shooter,Sound,Start);PC->AddPitchInput(-.4f);return Projectile;
}
void ABattleDisc::Tick(float Dt){
 Super::Tick(Dt);Age+=Dt;auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));if(Age>8||!Bike||(Mode&&Mode->bRunEnded)){Destroy();return;}
 Disc->AddLocalRotation(FRotator(0,1100*Dt,0));float Distance=Velocity.Size()*Dt;
 for(int32 Step=0;Step<12&&Distance>.01f;Step++){
  const FVector Start=GetActorLocation(),Direction=Velocity.GetSafeNormal(),End=Start+Direction*Distance;
  FCollisionQueryParams Q(SCENE_QUERY_STAT(DiscFlight),false,this);Q.AddIgnoredActor(Bike);if(LaunchPawn.IsValid())Q.AddIgnoredActor(LaunchPawn.Get());
  // Ignore the current rider too if possession changed after launch.
  if(auto* P=Cast<ABattleRider>(UGameplayStatics::GetPlayerPawn(this,0)))if(P->ParkedBike==Bike)Q.AddIgnoredActor(P);
  for(const auto& HitActor:Struck)if(HitActor.IsValid())Q.AddIgnoredActor(HitActor.Get());FHitResult Hit;
  if(!GetWorld()->SweepSingleByChannel(Hit,Start,End,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(12),Q)){SetActorLocation(End);break;}
  Distance=FMath::Max(0.f,Distance-FVector::Dist(Start,Hit.Location));
  if(auto* Victim=Cast<APiedmontExplorer>(Hit.GetActor())){
   Struck.Add(Victim);const bool Alive=!Victim->bDead;const float Damage=UGameplayStatics::ApplyDamage(Victim,65,UGameplayStatics::GetPlayerController(this,0),this,UDamageType::StaticClass());
   if(Alive&&Damage>0){if(Mode)Mode->RecordPlayerShotHit(Victim);Hits++;Bike->HitFeedback=.25f;if(auto* P=Cast<ABattleRider>(UGameplayStatics::GetPlayerPawn(this,0)))if(P->ParkedBike==Bike)P->HitFeedback=.25f;
    if(Victim->bDead&&Victim->ActorHasTag(TEXT("PiedmontHostile"))){Kills++;Bike->AwardEnemyKill();}
    else if(auto* Z=Cast<ABattleZombie>(Victim)){Z->bTelegraphing=false;Z->LaunchCharacter(Direction*220,true,false);}
   }
   SetActorLocation(Hit.Location+Direction*2);Distance=FMath::Max(0.f,Distance-2);continue;
  }
  if(Hit.bStartPenetrating||Hit.ImpactNormal.IsNearlyZero()||Bounces>=4){Destroy();return;}
  Bounces++;Velocity=Velocity.MirrorByVector(Hit.ImpactNormal);SetActorLocation(Hit.Location+Hit.ImpactNormal*2);Distance=FMath::Max(0.f,Distance-2);
  if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_DiscBounce.S_DiscBounce")))UGameplayStatics::PlaySoundAtLocation(this,Sound,Hit.ImpactPoint,.7f);
 }
}
