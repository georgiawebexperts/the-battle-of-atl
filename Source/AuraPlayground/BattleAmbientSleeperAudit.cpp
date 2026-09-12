#include "PiedmontPedestrian.h"
#include "Components/PoseableMeshComponent.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

// Exercises the level-authored actor after runtime props and crowd have spawned.
void TickBattleAmbientSleeperAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState { TWeakObjectPtr<UWorld> World; TWeakObjectPtr<APiedmontPedestrian> Person; int Phase=0; float Clock=0,Total=0; bool Done=false; };
 static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}
 if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;
 S.Clock+=Dt;S.Total+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Why){S.Done=true;UE_LOG(LogTemp,Display,TEXT("BattleAmbientSleeperAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),S.Phase,Why);UKismetSystemLibrary::QuitGame(PC,PC,EQuitPreference::Quit,false);};
#define REQUIRE_AMBIENT(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 REQUIRE_AMBIENT(S.Total<25,"Timed out");
 if(S.Phase==0){
  for(TActorIterator<APiedmontPedestrian> It(PC->GetWorld());It;++It)if(It->ActorHasTag(TEXT("AmbientSleeperReview"))){S.Person=*It;break;}
  auto* P=S.Person.Get();REQUIRE_AMBIENT(P,"Placed sleeper missing");
  REQUIRE_AMBIENT(P->bAmbientSleeper&&P->SleepPhase==1&&!P->bDead,"Placed actor did not initialize asleep");
  P->Body->RefreshBoneTransforms();
  UE_LOG(LogTemp,Display,TEXT("AmbientSleeperLocation: actor=%s head=%s"),*P->GetActorLocation().ToString(),*P->Body->GetSocketLocation(TEXT("head")).ToString());
  REQUIRE_AMBIENT(P->WakeFromSleep(),"Runtime objects block wake");S.Phase=1;S.Clock=0;return;
 }
 auto* P=S.Person.Get();REQUIRE_AMBIENT(P&&!P->bDead,"Sleeper lost");
 if(S.Phase==1&&S.Clock>5.8f){
  REQUIRE_AMBIENT(P->SleepPhase==0,"Wake did not finish");
  REQUIRE_AMBIENT(P->BeginSettling(),"Runtime site rejects fall corridor");S.Phase=2;S.Clock=0;return;
 }
 if(S.Phase==2&&S.Clock>6.2f){
  REQUIRE_AMBIENT(P->SleepPhase==1,"Fall did not return to sleep");
  REQUIRE_AMBIENT(P->WakeFromSleep(),"Landing blocks second wake");S.Phase=3;S.Clock=0;return;
 }
 if(S.Phase==3&&S.Clock>5.8f){REQUIRE_AMBIENT(P->SleepPhase==0,"Second wake did not finish");Finish(true,TEXT("Placed ambient startup, wake, fall and landing re-wake passed"));}
#undef REQUIRE_AMBIENT
#endif
}
