#include "BattleShot.h"
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
ABattleShotFX::ABattleShotFX(){
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("ShotOrigin"));InitialLifeSpan=.085f;
 Tracer=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisibleTracer"));Tracer->SetupAttachment(RootComponent);
 Flash=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MuzzleFlash"));Flash->SetupAttachment(RootComponent);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
 Tracer->SetStaticMesh(Cylinder.Object);Flash->SetStaticMesh(Sphere.Object);
 for(auto* M:{Tracer.Get(),Flash.Get()}){M->SetCollisionEnabled(ECollisionEnabled::NoCollision);M->SetCastShadow(false);M->SetCanEverAffectNavigation(false);}
 Light=CreateDefaultSubobject<UPointLightComponent>(TEXT("MuzzleLight"));Light->SetupAttachment(RootComponent);Light->SetIntensity(3000);Light->SetAttenuationRadius(240);Light->SetLightColor(FLinearColor(1,.48,.08));Light->SetCastShadows(false);
}
void ABattleShotFX::BeginPlay(){
 Super::BeginPlay();auto* Mat=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_ShotGlow.M_ShotGlow"));
 Tracer->SetMaterial(0,Mat);Flash->SetMaterial(0,Mat);SetActorLocation(Start);
 const FVector Delta=End-Start;Tracer->SetRelativeLocation(Delta*.5f);Tracer->SetRelativeRotation(FQuat::FindBetweenNormals(FVector::UpVector,Delta.GetSafeNormal()));Tracer->SetRelativeScale3D(FVector(.012,.012,Delta.Size()/100));
 Flash->SetRelativeScale3D(FVector(.18,.06,.06));
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
 if(Hit.GetActor()){const float Applied=UGameplayStatics::ApplyPointDamage(Hit.GetActor(),34,(Result.End-Muzzle).GetSafeNormal(),Hit,PC,Shooter,UPiedmontBulletDamage::StaticClass());Result.Damage=Hit.GetActor()->IsA<APawn>()?Applied:0;}
 Result.EnemyKilled=Alive&&Victim->bDead&&Victim->ActorHasTag(TEXT("PiedmontHostile"));
 if(auto* FX=Shooter->GetWorld()->SpawnActorDeferred<ABattleShotFX>(ABattleShotFX::StaticClass(),FTransform(Muzzle),Shooter,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn)){FX->Start=Muzzle;FX->End=Result.End;FX->FinishSpawning(FTransform(Muzzle));}
 if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/PiedmontRide/Audio/S_Gunshot.S_Gunshot")))UGameplayStatics::PlaySoundAtLocation(Shooter,Sound,Muzzle);
 PC->AddPitchInput(-.5f);return Result;
}
