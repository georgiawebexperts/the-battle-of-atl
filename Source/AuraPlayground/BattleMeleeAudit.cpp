#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleZombie.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void ABattleMacController::TickMeleeAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5)return;
 auto* Bike=Cast<ABattleBike>(GetPawn());auto* Person=Cast<ABattleRider>(GetPawn());if(Person)Bike=Person->ParkedBike;if(!Bike)return;
 MeleeAuditClock+=Dt;auto* Z=Cast<ABattleZombie>(MeleeTarget.Get());
 auto Finish=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BattleMeleeAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\",\"swings\":%d,\"hits\":%d,\"kills\":%d}"),Pass?TEXT("true"):TEXT("false"),MeleePhase,Reason,Person?Person->MeleeSwings:0,Person?Person->MeleeHits:0,Bike->EnemyKills);UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);};
#define CHECK_MELEE(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Place=[&](float Distance){Z->SetActorLocation(Person->GetActorLocation()+FVector(Distance,0,0),false,nullptr,ETeleportType::TeleportPhysics);Z->GetCharacterMovement()->StopMovementImmediately();SetControlRotation(FRotator::ZeroRotator);};
 auto Press=[&](){for(bool Down:{true,false})InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::F,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 if(MeleePhase==0){
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  CHECK_MELEE(Bike->Dismount(),"Dismount failed");Person=Cast<ABattleRider>(GetPawn());CHECK_MELEE(Person,"Missing FPS rider");
  Z=GetWorld()->SpawnActor<ABattleZombie>(Person->GetActorLocation()+FVector(140,0,0),FRotator::ZeroRotator);CHECK_MELEE(Z,"Target spawn failed");Z->SetActorTickEnabled(false);Z->GetCharacterMovement()->DisableMovement();Z->Emergence=0;MeleeTarget=Z;Place(140);MeleePhase=1;MeleeAuditClock=0;
 }
 else if(MeleePhase==1&&MeleeAuditClock>.2f){Press();MeleePhase=2;MeleeAuditClock=0;}
 else if(MeleePhase==2&&MeleeAuditClock>.1f){CHECK_MELEE(Person->MeleeSwings==1&&Z->Health==100,"F binding or windup failed");CHECK_MELEE(!Person->Melee()&&!Person->Fire(),"Swing allowed repeated melee or pistol fire");MeleePhase=3;}
 else if(MeleePhase==3&&MeleeAuditClock>.4f){CHECK_MELEE(Z->Health==50&&Person->MeleeHits==1&&!Z->bDead,"First swing did not deal 50 damage once");MeleePhase=4;}
 else if(MeleePhase==4&&MeleeAuditClock>.8f){Place(140);Press();MeleePhase=5;MeleeAuditClock=0;}
 else if(MeleePhase==5&&MeleeAuditClock>.8f){
  CHECK_MELEE(Z->bDead&&Person->MeleeHits==2&&Bike->EnemyKills==1&&Bike->Nitro==25,"Second swing kill/reward failed");Z->Destroy();
  Z=GetWorld()->SpawnActor<ABattleZombie>(Person->GetActorLocation()+FVector(140,0,0),FRotator::ZeroRotator);CHECK_MELEE(Z,"Occlusion target failed");Z->SetActorTickEnabled(false);Z->GetCharacterMovement()->DisableMovement();MeleeTarget=Z;
  auto* Wall=GetWorld()->SpawnActor<AStaticMeshActor>(Person->GetActorLocation()+FVector(70,0,30),FRotator::ZeroRotator);CHECK_MELEE(Wall,"Wall fixture failed");auto* Mesh=Wall->GetStaticMeshComponent();Mesh->SetMobility(EComponentMobility::Movable);Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Wall->SetActorScale3D(FVector(.1,2,3));Mesh->SetCollisionProfileName(TEXT("BlockAll"));MeleeWall=Wall;Place(140);Press();MeleePhase=6;MeleeAuditClock=0;
 }
 else if(MeleePhase==6&&MeleeAuditClock>.8f){CHECK_MELEE(Z->Health==100&&Person->MeleeHits==2,"Melee hit through wall");MeleeWall->Destroy();Place(400);Press();MeleePhase=7;MeleeAuditClock=0;}
 else if(MeleePhase==7&&MeleeAuditClock>.8f){
  CHECK_MELEE(Z->Health==100&&Person->MeleeHits==2,"Out of range melee hit");Person->Ammo=5;Person->Reload();CHECK_MELEE(Person->ReloadRemaining>0&&!Person->Melee(),"Melee bypassed reload");Person->ReloadRemaining=0;
  auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));CHECK_MELEE(Mode,"Missing mode");Mode->bRunEnded=true;CHECK_MELEE(!Person->Melee(),"Melee after end");Mode->bRunEnded=false;
  Bike->ApplyRiderDamage(1000);Person->Health=Bike->RiderHealth;CHECK_MELEE(!Person->Melee(),"Dead rider melee");Finish(true,TEXT("F input, windup, cooldown, 50 damage, kill reward, wall/range, reload and death guards pass"));return;
 }
 if(MeleeAuditClock>10)Finish(false,TEXT("Phase timeout"));
#undef CHECK_MELEE
#endif
}
