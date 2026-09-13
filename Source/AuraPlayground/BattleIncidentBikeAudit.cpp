#include "BattleBike.h"
#include "PiedmontPedestrian.h"
#include "Animation/AnimSequence.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
void TickBattleIncidentBikeAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 static int Phase=0;static float Age=0;static TWeakObjectPtr<APiedmontPedestrian> Person;static FVector Target;static bool NearMiss=false,ResetDone=false;static TWeakObjectPtr<ABattleBike> OriginalBike;static int InitialWipeouts=0;
 if(!PC||PC->GetWorld()->GetTimeSeconds()<5||Phase==99)return;Age+=Dt;
 auto* Bike=OriginalBike.IsValid()?OriginalBike.Get():Cast<ABattleBike>(PC->GetPawn());if(!Bike)return;OriginalBike=Bike;auto* M=Bike->Ride.Get();
 auto Key=[&](bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::W,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Finish=[&](bool Passed,const TCHAR* Reason){Key(false);UE_LOG(LogTemp,Display,TEXT("IncidentBikeAudit: {\"passed\":%s,\"reason\":\"%s\",\"near_miss\":%s,\"contacts\":%d,\"pose_active\":%s,\"knockdown\":%d,\"wipeouts\":%d,\"phase\":%d}"),Passed?TEXT("true"):TEXT("false"),Reason,NearMiss?TEXT("true"):TEXT("false"),Person.IsValid()?Person->BikeContacts:-1,Person.IsValid()&&Person->bIncidentPosing?TEXT("true"):TEXT("false"),Person.IsValid()?Person->KnockdownPhase:-1,M->Wipeouts-InitialWipeouts,Phase);Phase=99;PC->ConsoleCommand(TEXT("quit"));};
 if(Phase==0){
  Key(false);auto* Floor=PC->GetWorld()->SpawnActor<AStaticMeshActor>();auto* C=Floor->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube")));C->SetCollisionProfileName(TEXT("BlockAll"));Floor->SetActorScale3D(FVector(60,60,1));Floor->SetActorLocation(FVector(0,0,30000));Floor->Tags.Add(TEXT("RidePath"));
  FTransform T(FRotator::ZeroRotator,FVector(0,0,30140));auto* P=PC->GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),T);P->CityAppearanceVariant=0;UGameplayStatics::FinishSpawningActor(P,T);P->PauseRemaining=60;Person=P;InitialWipeouts=M->Wipeouts;Phase=1;Age=0;return;
 }
 if(!Person.IsValid()){Finish(false,TEXT("Lost encounter pedestrian"));return;}
 auto* P=Person.Get();
 if(Phase==1&&Age>1){auto* Clip=LoadObject<UAnimSequence>(nullptr,TEXT("/Game/BattleRetarget/City/Male/M_ragdoll_getup_stand_B"));if(!Clip||!P->BeginIncidentPose(Clip,Clip->GetPlayLength()*.08f,60)){Finish(false,TEXT("Could not hold injured pose"));return;}Phase=2;Age=0;return;}
 if(Phase==2&&Age>.7f){Target=P->Body->GetBoneLocation(TEXT("pelvis"));Phase=3;Age=0;}
 if((Phase==3||Phase==5)&&!ResetDone){Key(false);M->StopMovementImmediately();M->Speed=0;M->Recovery=0;M->Gear=3;M->SmoothedSteer=0;M->SlideRemaining=0;Bike->SetActorLocationAndRotation(FVector(Target.X-800,Target.Y+(Phase==3?300:0),30148),FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);M->SetMovementMode(MOVE_Walking);M->bForceNextFloorCheck=true;ResetDone=true;return;}
 if((Phase==3||Phase==5)&&Age>.35f){Key(true);Phase++;Age=0;}
 if(Phase==4&&Bike->GetActorLocation().X>Target.X+400){NearMiss=P->BikeContacts==0&&P->bIncidentPosing&&M->Wipeouts==InitialWipeouts;if(!NearMiss){Finish(false,TEXT("Near miss caused contact"));return;}Key(false);Phase=5;ResetDone=false;Age=0;return;}
 if(Phase==6&&P->BikeContacts>0){Finish(!P->bIncidentPosing&&P->KnockdownPhase>0&&M->Wipeouts>InitialWipeouts,TEXT("Actual bike movement contacted posed pedestrian"));return;}
 if(Age>8){Finish(false,TEXT("Traversal timed out without expected contact"));return;}
#endif
}
