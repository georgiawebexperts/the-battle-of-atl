#include "BattleBike.h"
#include "BattleRider.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
void TickBattleWallRecoveryAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{int Phase=0;float Clock=0,Yaw=0;FVector Contact,Start;bool Done=false;};static FState S;
 if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;
 const bool Vehicle=FParse::Param(FCommandLine::Get(),TEXT("BattleVehicleRecoveryAudit"));
 auto* B=Cast<ABattleBike>(PC->GetPawn());if(!B)return;auto* M=B->Ride.Get();S.Clock+=Dt;
 auto Key=[&](FKey K,bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Finish=[&](bool Passed,const TCHAR* Why){S.Done=true;for(FKey K:{EKeys::W,EKeys::S,EKeys::Right,EKeys::SpaceBar})Key(K,false);UE_LOG(LogTemp,Display,TEXT("WallRecoveryAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\",\"reverse_speed\":%.2f}"),Passed?TEXT("true"):TEXT("false"),S.Phase,Why,M->ReverseSpeed);PC->ConsoleCommand(TEXT("quit"));};
#define WCHECK(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Next=[&](){++S.Phase;S.Clock=0;};
 if(S.Phase==2&&S.Clock>.1f)Key(EKeys::W,false); // Brief nudge into the wall, below riding-impact speed.
 if(S.Phase==0){
  auto Cube=[&](FVector P,FVector Scale){auto* A=PC->GetWorld()->SpawnActor<AStaticMeshActor>();auto* Mesh=A->GetStaticMeshComponent();Mesh->SetMobility(EComponentMobility::Movable);Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Mesh->SetCollisionProfileName(TEXT("BlockAll"));A->SetActorScale3D(Scale);A->SetActorLocation(P);A->Tags.Add(TEXT("RidePath"));return A;};
  Cube(FVector(0,0,30000),FVector(80,80,1));auto* Obstacle=Cube(FVector(350,0,30200),FVector(1,10,3));if(Vehicle)Obstacle->Tags.Add(TEXT("RideVehicle"));
  M->StopMovementImmediately();M->Speed=M->ReverseSpeed=0;M->bRealHandling=FParse::Param(FCommandLine::Get(),TEXT("BattleRealHandlingAudit"));
  B->SetActorLocationAndRotation(FVector(240,0,30150),FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);M->SetMovementMode(MOVE_Walking);M->bForceNextFloorCheck=true;Next();
 }else if(S.Phase==1&&S.Clock>.4f){WCHECK(M->IsMovingOnGround(),"Fixture not grounded");Key(EKeys::W,true);Next();}
 else if(S.Phase==2&&S.Clock>1.5f){WCHECK(B->GetActorLocation().X>150&&B->GetActorLocation().X<300&&M->Recovery==0,"Did not hit wall without passing through it");S.Contact=B->GetActorLocation();Key(EKeys::W,false);Key(EKeys::S,true);Next();}
 else if(S.Phase==3&&S.Clock>1.2f){WCHECK(S.Contact.X-B->GetActorLocation().X>100&&M->ReverseSpeed>100,"S failed to back away from wall");S.Yaw=B->GetActorRotation().Yaw;Key(EKeys::Right,true);Next();}
 else if(S.Phase==4&&S.Clock>.8f){WCHECK(FMath::FindDeltaAngleDegrees(S.Yaw,B->GetActorRotation().Yaw)<-10,"Reverse steering did not turn away");Key(EKeys::S,false);Key(EKeys::Right,false);Key(EKeys::SpaceBar,true);Next();}
 else if(S.Phase==5&&S.Clock>.7f){WCHECK(M->ReverseSpeed<1&&M->Speed<1,"Space did not stop reverse movement");S.Start=B->GetActorLocation();Next();}
 else if(S.Phase==6&&S.Clock>.5f){WCHECK(FVector::Dist2D(S.Start,B->GetActorLocation())<5,"Space-only braking moved bike backward");Key(EKeys::SpaceBar,false);Key(EKeys::W,true);S.Start=B->GetActorLocation();Next();}
 else if(S.Phase==7&&S.Clock>1){WCHECK(M->Speed>150&&M->ReverseSpeed<1&&FVector::Dist2D(S.Start,B->GetActorLocation())>80,"Could not ride out after reversing");Finish(true,TEXT("Walking-speed wall contact, reverse escape, reverse steering, brake-only stop and forward departure pass"));}
 if(S.Clock>6)Finish(false,TEXT("Wall recovery timeout"));
#undef WCHECK
#endif
}
