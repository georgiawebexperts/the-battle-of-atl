#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleQuest.h"
#include "BattleHome.h"
#include "BattleHomeData.h"
#include "BattleRunRecords.h"
#include "BattleGhostRider.h"
#include "BattleZombie.h"
#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
#include "Containers/Ticker.h"
void ABattleMacController::TickFinishAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(FinishAuditDone||GetWorld()->GetTimeSeconds()<5)return;FinishAuditDone=true;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* Bike=Cast<ABattleBike>(GetPawn());ABattleHome* Home=nullptr;for(TActorIterator<ABattleHome> It(GetWorld());It;++It)Home=*It;
 auto End=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BattleFinishAudit: {\"passed\":%s,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Reason);UGameplayStatics::DeleteGameInSlot(BattleRecords::Slot(),0);ConsoleCommand(TEXT("quit"));};
#define FCHECK(C,R) if(!(C)){End(false,TEXT(R));return;}
 FCHECK(Mode&&Bike&&Home&&Mode->Quest&&Mode->Quest->bReady,"Missing ready quest or home");
 for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);if(Mode->Enemies)Mode->Enemies->bFreezeSpawns=true;
 for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)It->Destroy();
 Bike->Ride->Recovery=0;Bike->Ride->Speed=0;Bike->Ride->StopMovementImmediately();
 auto Place=[&](FVector P){Bike->SetActorLocation(P,false,nullptr,ETeleportType::TeleportPhysics);};
 Place(BattleHomeData::Gate+FVector(0,0,98));FCHECK(!Home->TryFinish()&&!Mode->bWon,"Won without Artifact");
 Place(Mode->Quest->ArtifactLocation+FVector(0,0,53));Mode->Quest->Tick(.016f);FCHECK(Mode->Quest->bCollected,"Real Artifact collection failed");
 Place(BattleHomeData::Gate+FVector(0,0,98));Home->bTunnelExited=true;FCHECK(!Home->TryFinish(),"Won before checkpoints");Home->bTunnelExited=false;
 for(const FVector& C:Mode->Quest->CheckpointLocations){Place(C+FVector(0,0,98));Mode->Quest->Tick(.016f);}FCHECK(Mode->Quest->NextCheckpoint==2,"Real checkpoint collection failed");
 // The tunnel crossing is recorded rather than required now: BattleHomeData's
 // TunnelEntry sits 46 m off the authored course, so requiring it left a real
 // rider stranded at the party with no result screen. That the course still
 // reaches the patio and commits the win is proved end to end by
 // -BattlePatioAudit; here we keep proving the tunnel is tracked in order.
 Place(BattleHomeData::Gate+FVector(0,0,98));
 Place(BattleHomeData::TunnelExit+FVector(0,0,98));Home->Tick(.016f);FCHECK(!Home->bTunnelExited,"Tunnel exit accepted before entrance");
 Place(BattleHomeData::TunnelEntry+FVector(0,0,98));Home->Tick(.016f);Place(BattleHomeData::TunnelExit+FVector(0,0,98));Home->Tick(.016f);FCHECK(Home->bTunnelEntered&&Home->bTunnelExited&&!Mode->bWon,"Ordered tunnel traversal failed");
 Place(BattleHomeData::Gate+FVector(0,0,500));FCHECK(!Home->TryFinish(),"Won above gate");Place(BattleHomeData::Gate+FVector(0,0,98));
 Bike->RiderHealth=0;FCHECK(!Home->TryFinish(),"Dead rider won");Bike->RiderHealth=100;Mode->StartCountdown=1;FCHECK(!Home->TryFinish(),"Countdown won");Mode->StartCountdown=0;
 SetPause(true);FCHECK(!Home->TryFinish(),"Paused run won");SetPause(false);
 Mode->TimeRemaining=0;FCHECK(!Home->TryFinish(),"Expired timer won");Mode->TimeRemaining=333;
 UGameplayStatics::DeleteGameInSlot(BattleRecords::Slot(),0);
 TArray<FVector> SeedRoute;for(int i=0;i<21;i++)SeedRoute.Add(BattleHomeData::Gate+FVector(-1200+i*60,0,98));
 FCHECK(BattleRecords::Record(TEXT("Easy"),200,SeedRoute)&&BattleRecords::Record(TEXT("Hard"),190)&&BattleRecords::Record(TEXT("Easy"),250)&&BattleRecords::Best(TEXT("Easy"))==200&&BattleRecords::Best(TEXT("Hard"))==190&&BattleRecords::BestRoute(TEXT("Easy")).Num()==21,"Record persistence, ghost route persistence or difficulty isolation failed");
 for(int i=0;i<5;i++)FCHECK(BattleRecords::AddWin(Mode->DifficultyName),"Ghost win seeding failed");
 Mode->SpawnGhost();FCHECK(Mode->Ghost&&Mode->bGhostRiding,"Ghost rider did not ride at 5 wins with a stored route");
 Mode->Ghost->Advance(5.f);FCHECK(FVector::Dist2D(Mode->Ghost->GetActorLocation(),BattleHomeData::Gate+FVector(-600,0,98))<250,"Ghost rider fell off its recorded route");
 Mode->RunElapsed=123.4f;Mode->RunTopSpeed=1500;Bike->EnemyKills=7;Bike->Ride->Wipeouts=2;Bike->NearMisses=9;Mode->RouteSamples.Reset();
 const FVector RealArrival=BattleHomeData::Gate+FVector(420,240,98);
 if(Mode->DifficultyName==TEXT("Hard")){Place(BattleHomeData::Gate-BattleHomeData::South*350+FVector(0,0,98));FCHECK(Bike->Dismount(),"Cannot dismount for foot finish");auto* Foot=Cast<ABattleRider>(GetPawn());FCHECK(Foot,"Missing foot rider");Foot->SetActorLocation(RealArrival,false,nullptr,ETeleportType::TeleportPhysics);}else Place(RealArrival);
 Home->Tick(.016f);FCHECK(Mode->bWon&&Mode->bRunEnded&&Mode->bRecordSaved&&!Home->TryFinish(),"Real patio arrival did not commit win once");
 FCHECK(FMath::IsNearlyEqual(BattleRecords::Best(Mode->DifficultyName),123.4f)&&Mode->FinishKills==7&&Mode->FinishWipeouts==2&&Mode->FinishNearMisses==9,"Win stats or record incorrect");
 FCHECK(BattleRecords::Wins(Mode->DifficultyName)==6&&(Mode->DifficultyName==TEXT("Easy")?BattleRecords::Wins(TEXT("Hard"))==0:BattleRecords::Wins(TEXT("Easy"))==0),"Win count persistence or difficulty isolation failed");
 FCHECK(Mode->bGhostBeaten,"Beating the ghost on a new best did not register");
 const float Before=Mode->TimeRemaining;Mode->Tick(.2f);FCHECK(Mode->TimeRemaining==Before&&!Mode->AdjustRunTime(30,TEXT("late reward")),"Finished timer changed");
 const bool CelebrationReview=FParse::Param(FCommandLine::Get(),TEXT("BattleCelebrationReview"));
 ShowMenu(CelebrationReview?TEXT("Celebration"):TEXT("Win"));FCHECK(IsPaused()&&Menu.IsValid(),"Win menu missing");
 if(CelebrationReview){
  FCHECK(CelebrationArt&&bCelebrating,"Celebration art or state missing");
  FString Dir;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Dir);
  auto Capture=[Dir](const TCHAR* Name,float Delay){const FString File=Dir/Name;FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([File](float){FScreenshotRequest::RequestScreenshot(File,true,false);return false;}),Delay);};
  Capture(TEXT("celebration.png"),.5f);Capture(TEXT("morgan.png"),5.5f);Capture(TEXT("cheers.png"),9.f);Capture(TEXT("credits-title.png"),12.5f);Capture(TEXT("credits-developer.png"),17.f);Capture(TEXT("credits-future.png"),21.f);Capture(TEXT("win.png"),25.5f);
  FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Weak=TWeakObjectPtr<ABattleMacController>(this)](float){
   const bool Pass=Weak.IsValid()&&!Weak->bCelebrating&&!Weak->bCredits&&Weak->Menu.IsValid()&&Weak->IsPaused();
   UE_LOG(LogTemp,Display,TEXT("BattleFinishAudit: {\"passed\":%s,\"reason\":\"Celebration, Version 1 credits and automatic results transition checked\"}"),Pass?TEXT("true"):TEXT("false"));
   UGameplayStatics::DeleteGameInSlot(BattleRecords::Slot(),0);if(Weak.IsValid())Weak->ConsoleCommand(TEXT("quit"));return false;
  }),27.f);return;
 }
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleFinishReview"))){FString Dir;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Dir);FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Dir](float){FScreenshotRequest::RequestScreenshot(Dir/TEXT("win.png"),true,false);return false;}),.5f);FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Weak=TWeakObjectPtr<ABattleMacController>(this)](float){UE_LOG(LogTemp,Display,TEXT("BattleFinishAudit: {\"passed\":true,\"reason\":\"Gate guards, real Artifact/checkpoints, ordered tunnel crossing, win stats/menu, frozen clock and saved records pass\"}"));UGameplayStatics::DeleteGameInSlot(BattleRecords::Slot(),0);if(Weak.IsValid())Weak->ConsoleCommand(TEXT("quit"));return false;}),2.f);return;}
 End(true,TEXT("Gate guards, real Artifact/checkpoints, ordered tunnel crossing, win stats/menu, frozen clock and saved records pass"));
#undef FCHECK
#endif
}
