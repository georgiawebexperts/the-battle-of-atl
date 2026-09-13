#include "BattleBike.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
void TickBattleRailScrapeAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 static int Phase=0;static float Clock=0;static bool Done=false;static float ForwardDistance=0;
 if(Done||PC->GetWorld()->GetTimeSeconds()<5)return;auto* B=Cast<ABattleBike>(PC->GetPawn());if(!B)return;auto* M=B->Ride.Get();Clock+=Dt;
 auto Key=[&](FKey K,bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Finish=[&](bool Pass){Done=true;Key(EKeys::W,false);Key(EKeys::S,false);UE_LOG(LogTemp,Display,TEXT("RailScrapeAudit: {\"passed\":%s,\"phase\":%d,\"forward_cm\":%.2f,\"reverse_cm\":%.2f}"),Pass?TEXT("true"):TEXT("false"),Phase,ForwardDistance,-B->GetActorLocation().X);PC->ConsoleCommand(TEXT("quit"));};
 auto Place=[&](float Yaw){M->StopMovementImmediately();M->Speed=M->ReverseSpeed=0;B->SetActorLocationAndRotation(FVector(0,60,30150),FRotator(0,Yaw,0),false,nullptr,ETeleportType::TeleportPhysics);M->SetMovementMode(MOVE_Walking);M->bForceNextFloorCheck=true;Clock=0;};
 if(Phase==0){
  auto Cube=[&](FVector P,FVector Scale){auto* A=PC->GetWorld()->SpawnActor<AStaticMeshActor>();auto* C=A->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));C->SetCollisionProfileName(TEXT("BlockAll"));A->SetActorScale3D(Scale);A->SetActorLocation(P);};
  Cube(FVector(0,0,30000),FVector(80,80,1));Cube(FVector(0,120,30200),FVector(40,.5,3));M->bRealHandling=FParse::Param(FCommandLine::Get(),TEXT("BattleRealHandlingAudit"));Place(10);Phase=1;
 }else if(Phase==1&&Clock>.4f){M->Speed=500;Key(EKeys::W,true);Clock=0;Phase=2;}
 else if(Phase==2&&Clock>2){ForwardDistance=B->GetActorLocation().X;if(ForwardDistance<600||B->GetActorLocation().Y>65||M->Recovery>0){Finish(false);return;}Key(EKeys::W,false);Place(-10);Phase=3;}
 else if(Phase==3&&Clock>.4f){Key(EKeys::S,true);Clock=0;Phase=4;}
 else if(Phase==4&&Clock>2){Finish(B->GetActorLocation().X<-240&&B->GetActorLocation().Y<=65&&M->Recovery==0);}
#endif
}
