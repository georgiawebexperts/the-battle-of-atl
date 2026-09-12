#include "BattleGunman.h"
#include "BattleZombie.h"
#include "BattleBike.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/PlatformMisc.h"
void TickBattleGunmanAudit(ABattleEnemyDirector* D,float Dt){
#if !UE_BUILD_SHIPPING
 auto* W=D->GetWorld();if(W->GetTimeSeconds()<5)return;
 static int Phase=0;static float Clock=0;static ABattleGunman* Gun=nullptr;static AStaticMeshActor* Cover=nullptr;static FVector Origin;
 auto* B=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(W,0));auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(W));if(!B||!M)return;
 auto End=[&](bool Pass,const TCHAR* Why){UE_LOG(LogTemp,Display,TEXT("GunmanAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Phase,Why);FPlatformMisc::RequestExit(false);};
#define GUN_CHECK(C,R) if(!(C)){End(false,TEXT(R));return;}
 Clock+=Dt;
 if(Phase==0){
  B->DamageGrace=0;B->Ride->StopMovementImmediately();Origin=B->GetActorLocation();
  Gun=W->SpawnActor<ABattleGunman>(Origin+FVector(500,0,0),FRotator(0,180,0));GUN_CHECK(Gun,"Gunman spawn failed");Gun->Cooldown=0;
  M->bTutorialActive=true;GUN_CHECK(!Gun->TryAim(B),"Gunman attacked during practice");M->bTutorialActive=false;M->StartCountdown=0;
  GUN_CHECK(Gun->TryAim(B)&&Gun->WindupRemaining>=1.8f,"No warning before shot");
  Cover=W->SpawnActor<AStaticMeshActor>((Gun->GetActorLocation()+Origin)*.5f,FRotator::ZeroRotator);GUN_CHECK(Cover,"Cover spawn failed");
  Cover->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);Cover->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Cover->SetActorScale3D(FVector(.5,4,4));Cover->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
  Phase=1;Clock=0;
 }else if(Phase==1&&Clock>2.1f){
  GUN_CHECK(Gun->ShotsFired==1&&B->RiderHealth==100,"Cover did not stop warned shot");Cover->Destroy();Gun->Cooldown=0;
  GUN_CHECK(Gun->TryAim(B),"Could not aim after cover removed");B->SetActorLocation(Origin+FVector(0,400,0),false,nullptr,ETeleportType::TeleportPhysics);Phase=2;Clock=0;
 }else if(Phase==2&&Clock>2.1f){
  GUN_CHECK(Gun->ShotsFired==2&&B->RiderHealth==100,"Shot tracked player after aim lock");
  B->SetActorLocation(Origin,false,nullptr,ETeleportType::TeleportPhysics);Gun->Cooldown=0;GUN_CHECK(Gun->TryAim(B),"Could not aim at stationary target");Phase=3;Clock=0;
 }else if(Phase==3&&Clock>1.9f){
  GUN_CHECK(Gun->ShotsFired==3&&B->RiderHealth==0&&B->Deaths==1,"Uncovered shot was not fatal");
  End(true,TEXT("Practice protected, warning precedes shot, cover blocks, movement dodges locked aim, stationary hit kills"));Phase=4;
 }
#undef GUN_CHECK
#endif
}
