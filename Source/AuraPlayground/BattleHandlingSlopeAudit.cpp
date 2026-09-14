#include "BattleBike.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
void TickBattleHandlingSlopeAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<AStaticMeshActor> Floor;int Phase=0,Samples=0,Ground=0;float Clock=0,MaxLateral=0,StartYaw=0;FVector StartPosition;bool Started=false,Ready=false,Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;
 auto* Bike=Cast<ABattleBike>(PC->GetPawn());if(!Bike)return;auto* Move=Bike->Ride.Get();
 auto Key=[&](FKey K,bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Finish=[&](bool Pass,const TCHAR* Why){for(FKey K:{EKeys::W,EKeys::D,EKeys::S,EKeys::SpaceBar})Key(K,false);S.Done=true;UE_LOG(LogTemp,Display,TEXT("HandlingSlopeAudit: {\"passed\":%s,\"reason\":\"%s\",\"phase\":%d}"),Pass?TEXT("true"):TEXT("false"),Why,S.Phase);PC->ConsoleCommand(TEXT("quit"));};
 if(!S.Started){
  for(FKey K:{EKeys::W,EKeys::D,EKeys::S,EKeys::SpaceBar})Key(K,false);
  if(!S.Floor.IsValid()){auto* Floor=PC->GetWorld()->SpawnActor<AStaticMeshActor>();S.Floor=Floor;Floor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);Floor->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Floor->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));Floor->SetActorScale3D(FVector(120,120,1));}
  auto* Floor=S.Floor.Get();Floor->Tags.Empty();Floor->Tags.Add((S.Phase==4||S.Phase>=6)?TEXT("RideGrass"):TEXT("RidePath"));Floor->SetActorLocationAndRotation(FVector(0,0,30000),FRotator(S.Phase>=6?35:S.Phase<2?10:0,0,0));
  FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(Bike);
  if(!PC->GetWorld()->LineTraceSingleByChannel(Hit,FVector(0,0,31000),FVector(0,0,29000),ECC_Visibility,Q)||Hit.GetActor()!=Floor){Finish(false,TEXT("Missing fixture floor"));return;}
  Move->StopMovementImmediately();Move->Speed=Move->ReverseSpeed=0;Move->Recovery=0;Move->SmoothedSteer=0;Move->bRealHandling=true;Move->Gear=S.Phase>=9?1:5;Move->SlideRemaining=Move->BoostRemaining=0;
  Bike->SetActorLocationAndRotation(Hit.ImpactPoint+FVector(0,0,98),FRotator(0,(S.Phase==1||S.Phase>=6)?180:0,0),false,nullptr,ETeleportType::TeleportPhysics);Move->SetMovementMode(MOVE_Walking);Move->bForceNextFloorCheck=true;
  S.Clock=0;S.Started=true;S.Ready=false;S.Samples=S.Ground=0;S.MaxLateral=0;return;
 }
 S.Clock+=Dt;
 if(!S.Ready){if(S.Clock<.3f)return;if(!Move->IsMovingOnGround()){Finish(false,TEXT("Did not settle on fixture"));return;}
  Move->Speed=(S.Phase==7||S.Phase==10)?0:S.Phase==3||S.Phase==4?1600:600;Move->Velocity=Bike->GetActorForwardVector()*Move->Speed;S.StartYaw=Bike->GetActorRotation().Yaw;S.StartPosition=Bike->GetActorLocation();
  if(S.Phase==2||S.Phase==6||S.Phase==7)Key(EKeys::SpaceBar,true);if(S.Phase==8)Key(EKeys::S,true);if(S.Phase==3||S.Phase==4)Key(EKeys::D,true);
  if(S.Phase>=9){Key(EKeys::W,true);Key(EKeys::SpaceBar,true);}
  if(S.Phase==5){Move->Velocity.Z=650;Move->SetMovementMode(MOVE_Falling);Key(EKeys::W,true);Key(EKeys::D,true);}
  S.Ready=true;S.Clock=0;return;
 }
 if(S.Phase==10&&S.Clock>1.f)Key(EKeys::SpaceBar,false);
 ++S.Samples;if(Move->IsMovingOnGround())++S.Ground;
 S.MaxLateral=FMath::Max(S.MaxLateral,FMath::Abs(Move->Speed*FMath::DegreesToRadians(Move->TurnRateDegrees)));
 if(S.Clock<(S.Phase==5?.2f:S.Phase>=6?2.f:1.f))return;
 const float Yaw=FMath::Abs(FMath::FindDeltaAngleDegrees(S.StartYaw,Bike->GetActorRotation().Yaw));
 const bool Pass=S.Phase==0?(Move->Speed<500&&Move->Speed>300):S.Phase==1?(Move->Speed>700&&Move->Speed<900):S.Phase==2?Move->Speed<10:S.Phase==3?(S.MaxLateral<=685&&Yaw>10):S.Phase==4?(S.MaxLateral<=355&&Yaw>5):S.Phase==5?(Move->IsFalling()&&FMath::Abs(Move->Speed-600)<1&&Yaw<.01f):S.Phase==10?(Move->Speed>100&&FVector::DotProduct(Bike->GetVelocity(),Bike->GetActorForwardVector())>100):S.Phase==8?(Move->Speed<1&&Move->ReverseSpeed>100&&FVector::DotProduct(Bike->GetVelocity(),Bike->GetActorForwardVector())<-100):(Move->Speed<1&&Move->ReverseSpeed<1&&Bike->GetVelocity().Size2D()<10&&(S.Phase!=7||FVector::Dist2D(S.StartPosition,Bike->GetActorLocation())<5));
 UE_LOG(LogTemp,Display,TEXT("HandlingSlopeSample: {\"phase\":%d,\"speed\":%.3f,\"max_lateral\":%.3f,\"yaw\":%.3f,\"ground\":%d,\"samples\":%d,\"passed\":%s}"),S.Phase,Move->Speed,S.MaxLateral,Yaw,S.Ground,S.Samples,Pass?TEXT("true"):TEXT("false"));
 if(!Pass||(S.Phase!=5&&S.Ground!=S.Samples)){Finish(false,TEXT("Slope braking grip or airborne behavior failed"));return;}
 if(S.Phase==10){Finish(true,TEXT("Slope coasting/grip, airborne, steep-grass braking/holding/reverse, pedal/brake priority and release passed"));return;}++S.Phase;S.Started=false;
#endif
}
