#include "PiedmontPedestrian.h"
#include "Components/PoseableMeshComponent.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/HUD.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

// Exercises the level-authored actor after runtime props and crowd have spawned.
void TickBattleAmbientSleeperAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState { TWeakObjectPtr<UWorld> World; TWeakObjectPtr<APiedmontPedestrian> Person; int Phase=0; float Clock=0,Total=0; TWeakObjectPtr<ACameraActor> Camera; bool Done=false; };
 static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}
 if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;
 S.Clock+=Dt;S.Total+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Why){S.Done=true;UE_LOG(LogTemp,Display,TEXT("BattleAmbientSleeperAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),S.Phase,Why);UKismetSystemLibrary::QuitGame(PC,PC,EQuitPreference::Quit,false);};
#define REQUIRE_AMBIENT(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 const bool Render=FParse::Param(FCommandLine::Get(),TEXT("BattleAmbientSleeperRender"));
 auto Capture=[&](const TCHAR* Name){
  FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);
  if(Folder.IsEmpty())Folder=FPaths::ProjectSavedDir()/TEXT("AmbientSleeperReview");
  IFileManager::Get().MakeDirectory(*Folder,true);FScreenshotRequest::RequestScreenshot(Folder/Name,false,false);
 };
 REQUIRE_AMBIENT(S.Total<35,"Timed out");
 if(S.Phase==0){
  for(TActorIterator<APiedmontPedestrian> It(PC->GetWorld());It;++It)if(It->ActorHasTag(TEXT("AmbientSleeperReview"))){S.Person=*It;break;}
  auto* P=S.Person.Get();REQUIRE_AMBIENT(P,"Placed sleeper missing");
  REQUIRE_AMBIENT(P->bAmbientSleeper&&P->SleepPhase==1&&!P->bDead,"Placed actor did not initialize asleep");
  P->Body->RefreshBoneTransforms();
  UE_LOG(LogTemp,Display,TEXT("AmbientSleeperLocation: actor=%s head=%s"),*P->GetActorLocation().ToString(),*P->Body->GetSocketLocation(TEXT("head")).ToString());
  for(const TCHAR* Bone:{TEXT("head"),TEXT("pelvis"),TEXT("foot_l"),TEXT("foot_r"),TEXT("ball_l"),TEXT("ball_r")}){
   const FVector Point=P->Body->GetSocketLocation(Bone);FHitResult Hit;FCollisionQueryParams Query(SCENE_QUERY_STAT(AmbientSleeperContact),false,P);
   if(PC->GetWorld()->LineTraceSingleByChannel(Hit,Point+FVector(0,0,150),Point-FVector(0,0,150),ECC_Visibility,Query))
    UE_LOG(LogTemp,Display,TEXT("AmbientSleeperContact: bone=%s clearance_cm=%.4f surface=%s"),Bone,Point.Z-Hit.ImpactPoint.Z,*GetNameSafe(Hit.GetActor()));
  }

  if(Render){
   auto* Camera=PC->GetWorld()->SpawnActor<ACameraActor>();S.Camera=Camera;Camera->GetCameraComponent()->SetFieldOfView(50);
   const FVector Target=P->GetActorLocation()+FVector(0,0,-60),Offset(280,280,150);
   Camera->SetActorLocation(Target+Offset);Camera->SetActorRotation((-Offset).Rotation());PC->SetViewTarget(Camera);
   if(PC->GetHUD())PC->GetHUD()->bShowHUD=false;
   S.Phase=10;S.Clock=0;return;
  }
  REQUIRE_AMBIENT(P->WakeFromSleep(),"Runtime objects block wake");S.Phase=1;S.Clock=0;return;
 }
 auto* P=S.Person.Get();REQUIRE_AMBIENT(P&&!P->bDead,"Sleeper lost");
 if(S.Phase==10&&S.Clock>1){Capture(TEXT("sleeping.png"));S.Phase=11;S.Clock=0;return;}
 if(S.Phase==11&&S.Clock>1){REQUIRE_AMBIENT(P->WakeFromSleep(),"Runtime objects block wake");S.Phase=1;S.Clock=0;return;}

 if(S.Phase==1&&S.Clock>5.8f){
  REQUIRE_AMBIENT(P->SleepPhase==0,"Wake did not finish");
  if(Render)Capture(TEXT("standing.png"));
  REQUIRE_AMBIENT(P->BeginSettling(),"Runtime site rejects fall corridor");S.Phase=2;S.Clock=0;return;
 }
 if(S.Phase==2&&S.Clock>6.2f){
  REQUIRE_AMBIENT(P->SleepPhase==1,"Fall did not return to sleep");
  if(Render)Capture(TEXT("landed.png"));
  REQUIRE_AMBIENT(P->WakeFromSleep(),"Landing blocks second wake");S.Phase=3;S.Clock=0;return;
 }
 if(S.Phase==3&&S.Clock>5.8f){REQUIRE_AMBIENT(P->SleepPhase==0,"Second wake did not finish");Finish(true,TEXT("Placed ambient startup, wake, fall and landing re-wake passed"));}
#undef REQUIRE_AMBIENT
#endif
}
