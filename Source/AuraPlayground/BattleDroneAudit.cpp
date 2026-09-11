#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleDrone.h"
#include "Engine/DamageEvents.h"
#include "BattleRider.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
void ABattleMacController::TickDroneAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5||DroneStage==99)return;DroneClock+=Dt;
 auto* Person=Cast<ABattleRider>(GetPawn());auto* Bike=Person?Person->ParkedBike.Get():Cast<ABattleBike>(GetPawn());if(!Bike)return;
 auto* Drone=Cast<ABattleDrone>(AuditDrone.Get());
 auto Finish=[&](bool Pass,const TCHAR* Why){UE_LOG(LogTemp,Display,TEXT("BattleDroneAudit: {\"passed\":%s,\"stage\":%d,\"reason\":\"%s\",\"health\":%.2f}"),Pass?TEXT("true"):TEXT("false"),DroneStage,Why,Bike->RiderHealth);DroneStage=99;ConsoleCommand(TEXT("quit"));};
#define DCHECK(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Next=[&](){DroneStage++;DroneClock=0;};
 if(DroneStage==0){
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  Bike->DamageGrace=0;AuditDrone=GetWorld()->SpawnActor<ABattleDrone>(Bike->GetActorLocation()+FVector(-600,0,400),FRotator::ZeroRotator);Next();
 }else if(DroneStage==1&&DroneClock>1){DCHECK(Drone&&Drone->bWarning&&Bike->RiderHealth==100,"Missing warning or premature hit");Next();}
 else if(DroneStage==2&&Person){DCHECK(Drone&&Drone->RiderHits==1&&Bike->RiderHealth==85&&Bike->StunRemaining>0&&!Person->MountBike()&&!Bike->ApplyDroneStrike(),"Swept hit, harm, dismount or repeat guard failed");Next();}
 else if(DroneStage==3&&DroneClock>2.2f){
  DCHECK(Person&&Bike->StunRemaining==0&&Person->MountBike(),"Local recovery/remount failed");if(Drone)Drone->Destroy();
  const FVector B=Bike->GetActorLocation();auto* Wall=GetWorld()->SpawnActor<AStaticMeshActor>(B+FVector(-300,0,220),FRotator::ZeroRotator);auto* M=Wall->GetStaticMeshComponent();M->SetMobility(EComponentMobility::Movable);M->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));M->SetWorldScale3D(FVector(.3,8,8));M->SetCollisionProfileName(TEXT("BlockAll"));AuditDroneWall=Wall;
  AuditDrone=GetWorld()->SpawnActor<ABattleDrone>(B+FVector(-600,0,400),FRotator::ZeroRotator);Next();
 }else if(DroneStage==4&&DroneClock>2.7f){
  DCHECK(Drone&&Drone->bSpent&&Drone->RiderHits==0&&!Bike->bParked,"Drone passed through obstruction");
  const float Health=Drone->Health;FDamageEvent Damage;DCHECK(Drone->TakeDamage(34,Damage,this,Bike)==34&&Drone->Health==Health-34,"Drone cannot be shot");DCHECK(Drone->TakeDamage(34,Damage,this,Bike)==6&&Drone->IsActorBeingDestroyed(),"Drone cannot be destroyed");
  if(AuditDroneWall.IsValid())AuditDroneWall->Destroy();Finish(true,TEXT("Warning, swept rider hit, 15 harm, knockoff/recovery/remount, wall obstruction and destruction pass"));
 }
 if(DroneClock>8&&DroneStage!=99)Finish(false,TEXT("Drone audit timeout"));
#undef DCHECK
#endif
}
