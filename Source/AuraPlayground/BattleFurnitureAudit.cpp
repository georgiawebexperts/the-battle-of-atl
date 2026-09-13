#include "BattleMacController.h"
#include "BattleParkFurniture.h"
#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
void TickBattleCarLoopAudit(APlayerController* PC,float Dt);
void TickBattleLightBoundaryAudit(APlayerController* PC,float Dt);
void TickBattleScooterSceneAudit(APlayerController* PC,float Dt);
void TickBattleIncidentPoseAudit(APlayerController* PC,float Dt);
void ABattleMacController::TickFurnitureAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleCarLoopAudit"))){TickBattleCarLoopAudit(this,Dt);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleLightBoundaryAudit"))){TickBattleLightBoundaryAudit(this,Dt);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleScooterSceneAudit"))){TickBattleScooterSceneAudit(this,Dt);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleIncidentPoseAudit"))){TickBattleIncidentPoseAudit(this,Dt);return;}
 if(GetWorld()->GetTimeSeconds()<5||bFurnitureAudited)return;bFurnitureAudited=true;
 for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
 for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
 int32 Count=0,Blocking=0;for(TActorIterator<ABattleParkFurniture> It(GetWorld());It;++It){Count+=It->Benches.Num();
  for(const auto& T:It->Benches){FHitResult H;FCollisionQueryParams Q(SCENE_QUERY_STAT(BenchAudit),false,GetPawn());
   const FVector A=T.TransformPosition(FVector(0,180,70)),B=T.TransformPosition(FVector(0,-100,70));
   if(GetWorld()->SweepSingleByChannel(H,A,B,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeSphere(10),Q)&&H.GetActor()==*It)Blocking++;
  }
 }
 UE_LOG(LogTemp,Display,TEXT("BattleFurnitureAudit: {\"passed\":%s,\"benches\":%d,\"blocking_benches\":%d}"),Count>=12&&Blocking==Count?TEXT("true"):TEXT("false"),Count,Blocking);ConsoleCommand(TEXT("quit"));
#endif
}
