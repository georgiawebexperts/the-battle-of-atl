#include "BattleScooterScene.h"
#include "PiedmontPedestrian.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
void TickBattleScooterSceneAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 static int Stage=0;static float Age=0;static TWeakObjectPtr<ABattleScooterScene> Selected,Skipped;static TWeakObjectPtr<ACameraActor> Camera;static bool HiddenWait=false;
 if(!PC||!PC->GetPawn()||PC->GetWorld()->GetTimeSeconds()<5||Stage==99)return;Age+=Dt;
 const FVector Site(30349.800013,114057.877225,1110.508188);
 auto Finish=[&](bool Passed,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("ScooterSceneAudit: {\"passed\":%s,\"reason\":\"%s\",\"offscreen_wait\":%s,\"participants\":%d,\"ready\":%s,\"skipped_empty\":%s}"),Passed?TEXT("true"):TEXT("false"),Reason,HiddenWait?TEXT("true"):TEXT("false"),Selected.IsValid()?Selected->Participants.Num():-1,Selected.IsValid()&&Selected->bSceneReady?TEXT("true"):TEXT("false"),Skipped.IsValid()&&!Skipped->bSelected&&Skipped->Participants.IsEmpty()?TEXT("true"):TEXT("false"));Stage=99;PC->ConsoleCommand(TEXT("quit"));};
 if(Stage==0){auto* C=PC->GetWorld()->SpawnActor<ACameraActor>(Site+FVector(5000,0,200),FRotator(0,180,0));Camera=C;
  auto* A=PC->GetWorld()->SpawnActor<ABattleScooterScene>(Site,FRotator::ZeroRotator);A->AppearanceChance=1;Selected=A;
  auto* B=PC->GetWorld()->SpawnActor<ABattleScooterScene>(Site,FRotator::ZeroRotator);B->AppearanceChance=0;Skipped=B;Stage=1;Age=0;
 }
 if(Camera.IsValid())PC->SetViewTarget(Camera.Get());
 if(!Selected.IsValid()||!Skipped.IsValid()){Finish(false,TEXT("Scene aborted during mapped placement"));return;}
 if(Stage==1&&Age>1){HiddenWait=Selected->bSelected&&!Selected->bSceneReady&&Selected->Participants.IsEmpty();Camera->SetActorRotation(FRotator::ZeroRotator);Stage=2;Age=0;return;}
 if(Stage==2&&Age>5){TArray<UStaticMeshComponent*> Parts;Selected->GetComponents(Parts);
  const bool Pass=HiddenWait&&Selected->bSceneReady&&Selected->Participants.Num()==3&&Selected->Participants[0]->bIncidentPosing&&Selected->Participants[1]->bIncidentPosing&&!Selected->Participants[2]->bIncidentPosing&&Parts.Num()==11&&!Skipped->bSelected&&Skipped->Participants.IsEmpty();
  FString RenderPath;if(Pass&&FParse::Value(FCommandLine::Get(),TEXT("BattleScooterSceneRender="),RenderPath)){Camera->SetActorLocation(Site+FVector(550,0,300));Camera->SetActorRotation((Site+FVector(0,0,60)-Camera->GetActorLocation()).Rotation());Stage=3;Age=0;return;}
  Finish(Pass,TEXT("Actual Krog site: visible wait then offscreen assembly, two interactive poses, bystander and eleven scooter parts"));}
 if(Stage==3&&Age>.8f){FString Path;FParse::Value(FCommandLine::Get(),TEXT("BattleScooterSceneRender="),Path);FScreenshotRequest::RequestScreenshot(Path,false,false);Stage=4;Age=0;}
 if(Stage==4&&Age>1)Finish(true,TEXT("Runtime scene captured; visual review pending"));
#endif
}
