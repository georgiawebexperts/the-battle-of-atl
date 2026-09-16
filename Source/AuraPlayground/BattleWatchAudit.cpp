#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleQuest.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"

void TickBattleWatchAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;float Clock=0;bool Setup=false,Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done)return;S.Clock+=Dt;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(PC));APawn* Pawn=PC->GetPawn();auto* Quest=Mode?Mode->Quest.Get():nullptr;if(!Pawn||!Quest||!Quest->bReady||S.Clock<1)return;
 const float Range=FMath::Min(6000.f,Quest->RadarRange*.5f);const FVector Far=Quest->ArtifactLocation+FVector(Range*1.2f,0,0),Near=Quest->ArtifactLocation+FVector(Range*.7f,0,0),Hot=Quest->ArtifactLocation+FVector(FMath::Max(400.f,Range*.12f),0,0);
 const float FarSignal=Quest->WatchSignalStrength(Far),NearSignal=Quest->WatchSignalStrength(Near),HotSignal=Quest->WatchSignalStrength(Hot),NearHz=Quest->WatchFlashHz(Near),HotHz=Quest->WatchFlashHz(Hot);
 FString ReviewDir;const bool Review=FParse::Value(FCommandLine::Get(),TEXT("BattleWatchReviewDir="),ReviewDir);
 if(Review&&!S.Setup){Pawn->SetActorLocation(Hot+FVector(0,0,110),false,nullptr,ETeleportType::TeleportPhysics);S.Setup=true;S.Clock=0;return;}
 if(Review&&S.Clock<1.2f)return;if(Review&&!ReviewDir.IsEmpty())FScreenshotRequest::RequestScreenshot(ReviewDir/TEXT("watch-phone-signal.png"),false,false);
 const bool Pass=FarSignal==0&&NearSignal>0&&HotSignal>NearSignal&&HotHz>NearHz&&NearHz>1.2f;
 UE_LOG(LogTemp,Display,TEXT("BattleWatchAudit: {\"passed\":%s,\"far_signal\":%.3f,\"near_signal\":%.3f,\"hot_signal\":%.3f,\"near_hz\":%.2f,\"hot_hz\":%.2f,\"exact_location_hidden\":%s}"),Pass?TEXT("true"):TEXT("false"),FarSignal,NearSignal,HotSignal,NearHz,HotHz,Quest->ArtifactVisibleOnRadar(Hot)?TEXT("false"):TEXT("true"));S.Done=true;FTimerHandle Quit;PC->GetWorldTimerManager().SetTimer(Quit,[PC](){UKismetSystemLibrary::QuitGame(PC,PC,EQuitPreference::Quit,false);},Review?.8f:.05f,false);
#endif
}
