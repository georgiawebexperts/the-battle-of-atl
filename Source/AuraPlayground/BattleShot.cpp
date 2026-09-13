#include "BattleShot.h"
#include "BattleBike.h"
#include "BattleZombie.h"
#include "PiedmontCombat.h"
#include "PiedmontExplorer.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
static FString ConfirmedHitLabel(const APiedmontExplorer* Victim){
 if(!Victim)return TEXT("TARGET HIT");
 if(Victim->IsA<ABattleZombie>())return Victim->bDead?TEXT("ZOMBIE DOWN"):TEXT("ZOMBIE HIT");
 if(Victim->ActorHasTag(TEXT("BattlePolice")))return TEXT("POLICE HIT");
 if(Victim->ActorHasTag(TEXT("PiedmontHostile")))return Victim->bDead?TEXT("ATTACKER DOWN"):TEXT("ATTACKER HIT");
 return TEXT("PERSON HIT");
}
ABattleShotFX::ABattleShotFX(){
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("ShotOrigin"));InitialLifeSpan=.085f;
 Tracer=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisibleTracer"));Tracer->SetupAttachment(RootComponent);
 Flash=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MuzzleFlash"));Flash->SetupAttachment(RootComponent);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
 Tracer->SetStaticMesh(Cylinder.Object);Flash->SetStaticMesh(Sphere.Object);
 for(auto* M:{Tracer.Get(),Flash.Get()}){M->SetCollisionEnabled(ECollisionEnabled::NoCollision);M->SetCastShadow(false);M->SetCanEverAffectNavigation(false);}
 Light=CreateDefaultSubobject<UPointLightComponent>(TEXT("MuzzleLight"));Light->SetupAttachment(RootComponent);Light->SetIntensity(150);Light->SetAttenuationRadius(180);Light->SetLightColor(FLinearColor(1,.48,.08));Light->SetCastShadows(false);
}
void ABattleShotFX::BeginPlay(){
 Super::BeginPlay();auto* Mat=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_ShotGlow.M_ShotGlow"));
 Tracer->SetMaterial(0,Mat);Flash->SetMaterial(0,Mat);SetActorLocation(Start);
 const FVector Delta=End-Start;Tracer->SetRelativeLocation(Delta*.5f);Tracer->SetRelativeRotation(FQuat::FindBetweenNormals(FVector::UpVector,Delta.GetSafeNormal()));Tracer->SetRelativeScale3D(FVector(.012,.012,Delta.Size()/100));
 Flash->SetRelativeScale3D(FVector(.18,.06,.06));
 Flash->SetVisibility(bMuzzleFlash);Light->SetVisibility(bMuzzleFlash);
}
FBattleShotResult FireBattlePistol(APawn* Shooter,USceneComponent* Gun,float Spread){
 FBattleShotResult Result;if(!Shooter||!Gun)return Result;auto* PC=Cast<APlayerController>(Shooter->GetController());if(!PC)return Result;
 FVector Eye;FRotator View;PC->GetPlayerViewPoint(Eye,View);const FVector Aim=FMath::VRandCone(View.Vector(),FMath::DegreesToRadians(Spread));
 FCollisionQueryParams Q(SCENE_QUERY_STAT(BattlePistol),true,Shooter);FHitResult CameraHit,Hit;
 const FVector Far=Eye+Aim*15000;Shooter->GetWorld()->LineTraceSingleByChannel(CameraHit,Eye,Far,ECC_Visibility,Q);
 const FVector Target=CameraHit.bBlockingHit?CameraHit.ImpactPoint:Far;const FVector Muzzle=Gun->GetComponentLocation()+View.Vector()*14;
 if(!Shooter->GetWorld()->LineTraceSingleByChannel(Hit,Shooter->GetActorLocation()+FVector(0,0,42),Muzzle,ECC_Visibility,Q))Shooter->GetWorld()->LineTraceSingleByChannel(Hit,Muzzle,Target+(Target-Muzzle).GetSafeNormal()*3,ECC_Visibility,Q);
 Result.End=Hit.bBlockingHit?Hit.ImpactPoint:Target;
 auto* Victim=Cast<APiedmontExplorer>(Hit.GetActor());const bool Alive=Victim&&!Victim->bDead;
 if(Hit.GetActor()){const float Applied=UGameplayStatics::ApplyPointDamage(Hit.GetActor(),34,(Result.End-Muzzle).GetSafeNormal(),Hit,PC,Shooter,UPiedmontBulletDamage::StaticClass());Result.Damage=Hit.GetActor()->IsA<APawn>()?Applied:0;if(Alive&&Applied>0)if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(Shooter)))Mode->RecordPlayerShotHit(Victim);}
 if(Alive&&Result.Damage>0)Result.HitLabel=ConfirmedHitLabel(Victim);
 Result.EnemyKilled=Alive&&Victim->bDead&&Victim->ActorHasTag(TEXT("PiedmontHostile"));
 Result.Kills=Result.EnemyKilled?1:0;
 if(auto* FX=Shooter->GetWorld()->SpawnActorDeferred<ABattleShotFX>(ABattleShotFX::StaticClass(),FTransform(Muzzle),Shooter,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn)){FX->Start=Muzzle;FX->End=Result.End;FX->FinishSpawning(FTransform(Muzzle));}
 if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/PiedmontRide/Audio/S_Gunshot.S_Gunshot")))UGameplayStatics::PlaySoundAtLocation(Shooter,Sound,Muzzle);
 PC->AddPitchInput(-.5f);return Result;
}

#include "BattleZombie.h"
FBattleShotResult FireBattleLongGun(APawn* Shooter,USceneComponent* Gun,int32 Slot,bool Aiming){
 FBattleShotResult Result;auto* PC=Shooter?Cast<APlayerController>(Shooter->GetController()):nullptr;if(!PC||!Gun)return Result;
 FVector Eye;FRotator View;PC->GetPlayerViewPoint(Eye,View);const bool Shotgun=Slot==1;const int32 Pellets=Shotgun?8:1;
 const float Range=Shotgun?2400:Slot==4?15000:11000,Spread=Shotgun?(Aiming?3.f:5.f):(Aiming?(Slot==4?.15f:1.f):3.f);
 const FVector Muzzle=Shotgun?Gun->GetComponentTransform().TransformPosition(FVector(40,1.28f,8)):Gun->GetComponentLocation()+View.Vector()*30;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(BattleLongGun),true,Shooter);
 TSet<AActor*> TimedVictims;
 for(int32 I=0;I<Pellets;I++){
  const FVector Aim=FMath::VRandCone(View.Vector(),FMath::DegreesToRadians(Spread));FHitResult CameraHit,Hit;
  Shooter->GetWorld()->LineTraceSingleByChannel(CameraHit,Eye,Eye+Aim*Range,ECC_Visibility,Q);const FVector Target=CameraHit.bBlockingHit?CameraHit.ImpactPoint:Eye+Aim*Range;
  if(!Shooter->GetWorld()->LineTraceSingleByChannel(Hit,Eye,Muzzle,ECC_Visibility,Q))Shooter->GetWorld()->LineTraceSingleByChannel(Hit,Muzzle,Target+(Target-Muzzle).GetSafeNormal()*3,ECC_Visibility,Q);
  Result.End=Hit.bBlockingHit?Hit.ImpactPoint:Target;auto* Victim=Cast<APiedmontExplorer>(Hit.GetActor());const bool Alive=Victim&&!Victim->bDead;
  if(Hit.GetActor()){
   const float Falloff=Shotgun?FMath::GetMappedRangeValueClamped(FVector2D(400,2400),FVector2D(1,.15),FVector::Distance(Eye,Result.End)):1;
   const float Damage=UGameplayStatics::ApplyPointDamage(Hit.GetActor(),(Shotgun?18:Slot==4?28:14)*Falloff,Aim,Hit,PC,Shooter,UPiedmontBulletDamage::StaticClass());
   if(Alive&&Damage>0)Result.HitLabel=ConfirmedHitLabel(Victim);
   if(Alive&&Damage>0&&!TimedVictims.Contains(Victim)){TimedVictims.Add(Victim);if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(Shooter)))Mode->RecordPlayerShotHit(Victim);}
   if(Alive){Result.Damage+=Damage;if(Victim->bDead&&Victim->ActorHasTag(TEXT("PiedmontHostile")))Result.Kills++;else if(Shotgun)if(auto* Z=Cast<ABattleZombie>(Victim)){Z->bTelegraphing=false;Z->LaunchCharacter(View.Vector()*650+FVector(0,0,120),true,true);}}
  }
  // Pellets retain separate tracers, but a trigger pull has only one muzzle flash/light.
  if(auto* FX=Shooter->GetWorld()->SpawnActorDeferred<ABattleShotFX>(ABattleShotFX::StaticClass(),FTransform(Muzzle),Shooter,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn)){FX->Start=Muzzle;FX->End=Result.End;FX->bMuzzleFlash=I==0;FX->FinishSpawning(FTransform(Muzzle));}
 }
 if(TimedVictims.Num()>1)Result.HitLabel=FString::Printf(TEXT("%d TARGETS HIT"),TimedVictims.Num());
 Result.EnemyKilled=Result.Kills>0;
 const TCHAR* SoundPath=Shotgun?TEXT("/Game/BattleForTheA/Audio/S_Shotgun.S_Shotgun"):TEXT("/Game/BattleForTheA/Audio/S_SMG.S_SMG");
 if(auto* Sound=LoadObject<USoundBase>(nullptr,SoundPath))UGameplayStatics::PlaySoundAtLocation(Shooter,Sound,Muzzle);
 PC->AddPitchInput(Shotgun?-2.f:-.3f);return Result;
}
