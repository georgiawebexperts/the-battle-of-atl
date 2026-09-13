#include "BattleBike.h"
#include "BattleRider.h"
#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"

bool ABattleBike::EnterLake(const FVector& Impact,const FVector& Bank){
 auto* PC=Cast<APlayerController>(GetController());if(!PC||bParked||RiderHealth<=0)return false;
 APiedmontWaterHazard* Lake=nullptr;
 for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->ContainsBike(Impact)){Lake=*It;break;}
 if(!Lake)return false;
 FCollisionQueryParams Query(SCENE_QUERY_STAT(BattleLakeEntry),false,this);
 for(TActorIterator<AActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("RideWater")))Query.AddIgnoredActor(*It);
 FVector Entry;bool Found=false;
 const FVector Forward=GetActorForwardVector();
 for(float Distance:{120.f,240.f,360.f,0.f}){
  Entry=Impact+Forward*Distance;Entry.Z=Lake->GetActorLocation().Z+88;
  if(Lake->ContainsBike(Entry)&&!GetWorld()->OverlapBlockingTestByChannel(Entry,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,88),Query)){Found=true;break;}
 }
 if(!Found)return false;
 FActorSpawnParameters Spawn;Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
 auto* Person=GetWorld()->SpawnActor<ABattleRider>(Entry,GetActorRotation(),Spawn);if(!Person)return false;
 RideImpact(1.8f,true);
 SetActorLocation(Bank,false,nullptr,ETeleportType::TeleportPhysics);
 Ride->BoostRemaining=0;Ride->Speed=Ride->Pedal=Ride->Steer=Ride->Brake=0;Ride->StopMovementImmediately();Ride->DisableMovement();
 bParked=true;Visual->SetRelativeRotation(FRotator::ZeroRotator);Rider->SetVisibility(false,true);ReloadTimer=0;LeanAngle=0;Ride->SmoothedSteer=0;
 Person->ParkedBike=this;Person->Health=RiderHealth;Person->RestoreLoadout();Person->GetCapsuleComponent()->IgnoreActorWhenMoving(this,true);
 PC->Possess(Person);PC->SetControlRotation(GetActorRotation());
 UE_LOG(LogTemp,Display,TEXT("BattleLakeEntry: swimmer=%s bank=%s"),*Entry.ToString(),*Bank.ToString());
 return true;
}
