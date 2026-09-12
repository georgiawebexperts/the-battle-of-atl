#include "BattleMacController.h"
#include "BattleRunRecords.h"
#include "BattleBike.h"
#include "BattleQuest.h"
#include "BattleRider.h"
#include "PiedmontTrafficDirector.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "TimerManager.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Containers/Ticker.h"

ABattleMacController::ABattleMacController(){
 static ConstructorHelpers::FObjectFinder<UTexture2D> Art(TEXT("/Game/BattleForTheA/Story/T_EstoriaCelebration.T_EstoriaCelebration"));CelebrationArt=Art.Object;
}

void ABattleMacController::BeginPlay(){
 Super::BeginPlay();
 if(GetWorld()->WorldType==EWorldType::Game){
  const auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));
  if(Mode&&UGameplayStatics::HasOption(Mode->OptionsString,TEXT("AutoStart")))ResumeRide();else ShowMenu();
#if !UE_BUILD_SHIPPING
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleAudit"))){
   FTimerHandle AuditHandle;
   GetWorld()->GetTimerManager().SetTimer(AuditHandle,this,&ABattleMacController::RunDevelopmentAudit,12,false);
  }
#endif
 }
}
void TickBattleSleeperAudit(APlayerController* PC,float Dt);
void TickBattleSleeperChaseAudit(APlayerController* PC,float Dt);
void TickBattleSleeperSettleAudit(APlayerController* PC,float Dt);
void TickBattleAmbientSleeperAudit(APlayerController* PC,float Dt);
void TickBattleSleeperTriggerAudit(APlayerController* PC,float Dt);
void TickBattleBenchFireReview(APlayerController* PC,float Dt);
void TickBattleBenchFireAudit(APlayerController* PC,float Dt);
void TickBattleBenchReachAudit(APlayerController* PC,float Dt);
void TickBattleBenchIgnitionAudit(APlayerController* PC,float Dt);
void TickBattleAmbientBenchAudit(APlayerController* PC,float Dt);
void TickBattleCarReview(APlayerController* PC,float Dt);
void TickBattleRoadCarAudit(APlayerController* PC,float Dt);
void TickBattleRoadLaneAudit(APlayerController* PC,float Dt);
void TickBattleRoadCrossingAudit(APlayerController* PC,float Dt);
void TickBattleCrossingReservationAudit(APlayerController* PC,float Dt);
void TickBattleTrafficSignalReview(APlayerController* PC,float Dt);
void ABattleMacController::PlayerTick(float Dt){
 Super::PlayerTick(Dt);
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleSpiritRouteAudit"))||FParse::Param(FCommandLine::Get(),TEXT("BattleHomeDriveAudit"))||FParse::Param(FCommandLine::Get(),TEXT("BattleConnectorAudit"))||FParse::Param(FCommandLine::Get(),TEXT("BattleEastsideAudit"))||FParse::Param(FCommandLine::Get(),TEXT("BattleKrogAudit")))TickConnectorAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleSleeperSettleAudit")))TickBattleSleeperSettleAudit(this,Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleAmbientSleeperAudit")))TickBattleAmbientSleeperAudit(this,Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleSleeperTriggerAudit")))TickBattleSleeperTriggerAudit(this,Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleBenchFireReview")))TickBattleBenchFireReview(this,Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleCarReview")))TickBattleCarReview(this,Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleRoadCarAudit")))TickBattleRoadCarAudit(this,Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleRoadLaneAudit")))TickBattleRoadLaneAudit(this,Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleRoadCrossingAudit")))TickBattleRoadCrossingAudit(this,Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleCrossingReservationAudit")))TickBattleCrossingReservationAudit(this,Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleTrafficSignalReview")))TickBattleTrafficSignalReview(this,Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleAmbientBenchAudit")))TickBattleAmbientBenchAudit(this,Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleBenchIgnitionAudit")))TickBattleBenchIgnitionAudit(this,Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleBenchReachAudit")))TickBattleBenchReachAudit(this,Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleBenchFireAudit")))TickBattleBenchFireAudit(this,Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleSleeperChaseAudit")))TickBattleSleeperChaseAudit(this,Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleSleeperAudit")))TickBattleSleeperAudit(this,Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleGeographyAudit")))TickGeographyAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleHealthAudit")))TickHealthAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattlePickupAudit")))TickPickupAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleDiscAudit")))TickDiscAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleLocomotionReview")))TickLocomotionReview(Dt);
 else if(FParse::Param(FCommandLine::Get(),TEXT("BattleHUDReview")))TickHUDReview(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleFrisbeeAudit")))TickFrisbeeAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleAmmoAudit")))TickAmmoAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleTroubleAudit")))TickTroubleAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleFurnitureAudit")))TickFurnitureAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleDroneAudit")))TickDroneAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleJumpAudit")))TickJumpAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleSkateAudit")))TickSkateAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleSkaterAudit")))TickSkaterAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleHornAudit")))TickHornAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleFinishAudit")))TickFinishAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleSteeringAudit")))TickSteeringAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleTimeAudit")))TickTimeAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleInventoryAudit")))TickInventoryAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleMeleeAudit")))TickMeleeAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleZombieAudit")))TickZombieAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleZombiePopulationAudit")))TickZombiePopulationAudit(Dt);
#endif
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleFootAudit")))TickFootAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleSpiritAudit")))TickSpiritAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleKnifeAudit")))TickKnifeAudit(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleMemorialReview")))TickMemorialReview(Dt);
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleTutorialAudit")))TickTutorialAudit(Dt);
#endif
 if(bStarted&&!Menu.IsValid()&&!bOpeningSeen)if(const auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this)))if(M->bTutorialActive)BeginOpening();
 if(bStarted&&!Menu.IsValid())if(const auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this)))if(Mode->bRunEnded)ShowMenu(Mode->bWon?(bCelebrationSeen?TEXT("Win"):TEXT("Celebration")):TEXT("Loss"));
}
void ABattleMacController::StartDifficulty(FName Name){
 SetPause(false);
 UGameplayStatics::OpenLevel(this,TEXT("/Game/PiedmontRide/Maps/PiedmontWorld"),true,TEXT("Difficulty=")+Name.ToString()+TEXT("?AutoStart=1"));
}
void ABattleMacController::SetupInputComponent(){
 Super::SetupInputComponent();
 auto& Binding=InputComponent->BindKey(EKeys::Escape,IE_Pressed,this,&ABattleMacController::ToggleMenu);
 Binding.bExecuteWhenPaused=true;
 auto& Skip=InputComponent->BindKey(EKeys::Enter,IE_Pressed,this,&ABattleMacController::SkipOpening);Skip.bExecuteWhenPaused=true;
 InputComponent->BindKey(EKeys::F1,IE_Pressed,this,&ABattleMacController::TogglePracticeHelp);
}
void ABattleMacController::TogglePracticeHelp(){if(auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this)))M->bTutorialHelp=!M->bTutorialHelp;}
void ABattleMacController::RemoveMenu(){
 if(Menu.IsValid()&&GetWorld()&&GetWorld()->GetGameViewport())GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(Menu.ToSharedRef());
 Menu.Reset();
}
void ABattleMacController::EndPlay(const EEndPlayReason::Type Reason){if(bOpeningActive&&GetWorld())GetWorld()->bIsCameraMoveableWhenPaused=bOpeningOldCameraMoveable;bOpeningActive=false;RemoveMenu();Super::EndPlay(Reason);}
void ABattleMacController::ResumeRide(){
 SetViewTarget(GetPawn());
 RemoveMenu();bStarted=true;SetPause(false);bShowMouseCursor=false;ResetIgnoreMoveInput();ResetIgnoreLookInput();SetInputMode(FInputModeGameOnly());
 FlushPressedKeys();
 UE_LOG(LogTemp,Display,TEXT("BattleMac: ride resumed"));
}
void ABattleMacController::ToggleMenu(){
 if(bOpeningActive){FinishOpening();return;}
 const auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));
 if(Mode&&Mode->bRunEnded){ShowMenu(Mode->bWon?TEXT("Win"):TEXT("Loss"));return;}
 if(Menu.IsValid()&&bStarted)ResumeRide();else ShowMenu();
}
void ABattleMacController::ShowMenu(FString Page){
 bCelebrating=Page==TEXT("Celebration");

 RemoveMenu();SetPause(true);bShowMouseCursor=true;ResetIgnoreMoveInput();ResetIgnoreLookInput();SetIgnoreMoveInput(true);SetIgnoreLookInput(true);
 TSharedRef<SVerticalBox> Items=SNew(SVerticalBox);
 auto Label=[&](FString Text,int Size,FLinearColor Color){Items->AddSlot().AutoHeight().Padding(0,6)[SNew(STextBlock).Text(FText::FromString(Text)).Font(FCoreStyle::GetDefaultFontStyle("Bold",Size)).ColorAndOpacity(Color).AutoWrapText(true)];};
 auto Button=[&](FString Text,TFunction<void()> Action){Items->AddSlot().AutoHeight().Padding(0,5)[SNew(SButton).ContentPadding(FMargin(18,10)).OnClicked_Lambda([Action](){Action();return FReply::Handled();})[SNew(STextBlock).Text(FText::FromString(Text)).Font(FCoreStyle::GetDefaultFontStyle("Bold",18))]];};
 Label(TEXT("THE BATTLE OF ATL"),42,FLinearColor(1,.12,.16));
 const auto* Park=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));
 const bool Ended=Park&&Park->bRunEnded;
 Label(Ended?(Park->bWon?TEXT("BATTLE WON"):TEXT("THE A WINS THIS TIME")):(bStarted?TEXT("PAUSED"):TEXT("PIEDMONT PARK / MAC PLAYTEST")),16,FLinearColor(1,.7,.35));
 if(Page==TEXT("Celebration")&&Park&&Park->bWon){
  bCelebrationSeen=true;CelebrationStart=FPlatformTime::Seconds();
  Items->AddSlot().AutoHeight().Padding(0,8)[SNew(STextBlock).Text_Lambda([this](){const double T=FPlatformTime::Seconds()-CelebrationStart;return FText::FromString(T<4?TEXT("Phone recovered. Party reached."):T<8?TEXT("Morgan: You made it, Ellison!"):TEXT("Ellison: Cheers, Morgan. What a ride."));}).Font(FCoreStyle::GetDefaultFontStyle("Bold",26)).ColorAndOpacity(FLinearColor::White).AutoWrapText(true)];
  Button(TEXT("CONTINUE TO RESULTS"),[this](){ShowMenu(TEXT("Win"));});
  FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Weak=TWeakObjectPtr<ABattleMacController>(this)](float){if(Weak.IsValid()&&Weak->bCelebrating)Weak->ShowMenu(TEXT("Win"));return false;}),12.f);
 }else if(Page==TEXT("Win")&&Park&&Park->bWon){
  Label(FString::Printf(TEXT("MADE THE PARTY  |  GRADE %s"),*Park->FinishGrade),30,FLinearColor(.3,1,.65));
  Label(FString::Printf(TEXT("TIME LEFT  %s\nRUN TIME  %s\nZOMBIES  %d   WIPEOUTS  %d\nTOP SPEED  %.0f MPH   NEAR MISSES  %d"),*BattleRecords::Format(Park->TimeRemaining),*BattleRecords::Format(Park->RunElapsed),Park->FinishKills,Park->FinishWipeouts,Park->RunTopSpeed*.0223694f,Park->FinishNearMisses),22,FLinearColor::White);
  Label(Park->bRecordSaved?FString::Printf(TEXT("BEST %s  %s"),*Park->DifficultyName.ToString(),*BattleRecords::Format(BattleRecords::Best(Park->DifficultyName))):TEXT("Best time could not be saved."),18,FLinearColor(1,.7,.35));
  Button(TEXT("RIDE AGAIN"),[this](){const auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));StartDifficulty(M?M->DifficultyName:FName(TEXT("Easy")));});Button(TEXT("LEVEL SELECT"),[this](){ShowMenu(TEXT("Levels"));});Button(TEXT("QUIT"),[this](){UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);});
 }else if(Page==TEXT("Loss")){
  Label(TEXT("Time ran out. Pick a difficulty and ride again."),18,FLinearColor::White);
  Button(TEXT("RETRY"),[this](){const auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));StartDifficulty(Mode?Mode->DifficultyName:FName(TEXT("Easy")));});
  Button(TEXT("LEVEL SELECT"),[this](){ShowMenu(TEXT("Levels"));});
  Button(TEXT("QUIT"),[this](){UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);});
 }else if(Page==TEXT("Levels")){
  Label(TEXT("Select a difficulty to start a new ride."),18,FLinearColor::White);
  if(auto* Table=LoadObject<UDataTable>(nullptr,TEXT("/Game/BattleForTheA/Data/DT_Difficulty.DT_Difficulty")))for(FName Name:{FName(TEXT("Easy")),FName(TEXT("Medium")),FName(TEXT("Hard"))}){
   if(const auto* Row=Table->FindRow<FBattleDifficultyRow>(Name,TEXT("Level select")))Button(FString::Printf(TEXT("%s  |  %.0f MIN  |  %s"),*Name.ToString(),Row->TimeLimitSeconds/60,*Row->Warning),[this,Name](){StartDifficulty(Name);});
   const float Best=BattleRecords::Best(Name);if(Best>0)Label(TEXT("BEST RUN  ")+BattleRecords::Format(Best),16,FLinearColor(1,.7,.35));
  }
  Label(TEXT("Timers and walking crowds scale now. The expanded enemy roster and complete route are still in development."),14,FLinearColor(.7,.72,.75));
  Button(TEXT("BACK"),[this,Ended](){const auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));ShowMenu(Ended?(M&&M->bWon?TEXT("Win"):TEXT("Loss")):TEXT("Home"));});
 }else if(Page==TEXT("Instructions")){
  Label(TEXT("Ellison dropped his cell phone playing frisbee in Piedmont Park. Use Find My Lost Phone on his watch to follow a broad compass direction. Recover it, then race past Murder K and Krog Street Market, through Krog Tunnel and right into 98 Estoria. Morgan is waiting for him at the Cabbagetown party. Make it before the clock runs out and celebrate with a beer."),16,FLinearColor::White);
  Label(TEXT("BIKE\nW/Up pedal | S/Down brake\nA/D or Left/Right steer | Q/R gears\nSpace brake/drift | J bike jump\nShift nitro | H horn (5 uses) | Tab camera\nE dismount | Left click pistol"),17,FLinearColor::White);
  Label(TEXT("ON FOOT\nWASD / arrows move | Mouse look\nShift sprint | Space jump | C/Control crouch\nG draw/holster weapon\nLeft click fire | Right click aim | R reload\n1 pistol | 2 shotgun | 3 SMG | 4 frisbee | 5 rifle\nFind weapon crates | Rifle: right click zoom\nF swing U-lock\nE near bike to remount | Esc pause"),17,FLinearColor::White);
  Button(TEXT("BACK"),[this](){ShowMenu();});
 }else if(Page==TEXT("Options")){
  Label(TEXT("Graphics presets target 1080p with a 60 FPS cap. Actual frame rate depends on the scene."),16,FLinearColor::White);
  for(int Quality:{1,2})Button(Quality==1?TEXT("PERFORMANCE / 1080p"):TEXT("BALANCED / 1080p"),[this,Quality](){if(auto* Settings=UGameUserSettings::GetGameUserSettings()){Settings->SetOverallScalabilityLevel(Quality);Settings->SetScreenResolution(FIntPoint(1920,1080));Settings->SetFullscreenMode(EWindowMode::Windowed);Settings->SetFrameRateLimit(60);Settings->ApplySettings(false);Settings->SaveSettings();}ShowMenu(TEXT("Options"));});
  Button(TEXT("BACK"),[this](){ShowMenu();});
 }else{
  if(!bStarted)Label(TEXT("Start at Ellison’s bungalow on 13th Street. Practice freely, then ride to the 14th Street gate to start the clock. Find the phone he dropped playing frisbee. Meet Morgan at 98 Estoria."),18,FLinearColor::White);
  Button(bStarted?TEXT("RESUME RIDE"):TEXT("START PARK RIDE"),[this](){ResumeRide();});
  Button(TEXT("LEVEL SELECT"),[this](){ShowMenu(TEXT("Levels"));});
  Button(TEXT("INSTRUCTIONS"),[this](){ShowMenu(TEXT("Instructions"));});
  Button(TEXT("OPTIONS"),[this](){ShowMenu(TEXT("Options"));});
  Button(TEXT("QUIT"),[this](){UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);});
  Label(TEXT("Mac playtest 042 | Web Experts\nPark riding and FPS test. Full route, enemies and campaign are not finished."),13,FLinearColor(.65,.68,.72));
 }
 if(bCelebrating&&CelebrationArt){
  CelebrationBrush.SetResourceObject(CelebrationArt);CelebrationBrush.ImageSize=FVector2D(CelebrationArt->GetSizeX(),CelebrationArt->GetSizeY());
  Menu=SNew(SOverlay)
   +SOverlay::Slot()[SNew(SScaleBox).Stretch(EStretch::ScaleToFill)[SNew(SImage).Image(&CelebrationBrush)]]
   +SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(36,20)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(.008,.012,.02,.84)).Padding(20)[SNew(SBox).WidthOverride(620)[Items]]];
 }else{
  const bool Opening=!bStarted&&Page==TEXT("Home");
  Menu=SNew(SOverlay)+SOverlay::Slot()[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(.008,.012,.02,Opening?.15f:.94f))]+SOverlay::Slot().HAlign(Opening?HAlign_Left:HAlign_Center).VAlign(VAlign_Center).Padding(Opening?30:0)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(.008,.012,.02,.85)).Padding(20)[SNew(SBox).WidthOverride(600)[Items]]];
 }

 if(auto* Viewport=GetWorld()->GetGameViewport())Viewport->AddViewportWidgetContent(Menu.ToSharedRef(),100);
 FInputModeGameAndUI Mode;Mode.SetWidgetToFocus(Menu);Mode.SetHideCursorDuringCapture(false);SetInputMode(Mode);
 UE_LOG(LogTemp,Display,TEXT("BattleMac: menu %s"),*Page);
}


// Opt-in native packaged integration probe; never active in normal play or Shipping.
void ABattleMacController::RunDevelopmentAudit(){
#if !UE_BUILD_SHIPPING
 FString Output;if(!FParse::Value(FCommandLine::Get(),TEXT("BattleAuditOutput="),Output))return;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));
 auto* Bike=Cast<ABattleBike>(GetPawn());
 int Desired=0,Live=0;
 for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It){Desired+=It->DesiredPopulation;Live+=It->LivePopulation;}
 const bool Timer=Mode&&Mode->TimeRemaining<Mode->Difficulty.TimeLimitSeconds-5&&Mode->TimeRemaining>Mode->Difficulty.TimeLimitSeconds-20;
 const bool Population=Mode&&Desired==Mode->Difficulty.Walkers+Mode->Difficulty.Joggers&&Live==Desired;
 const bool Dismounted=Bike&&Bike->Dismount();
 auto* Person=Cast<ABattleRider>(GetPawn());
 const int Ammo=Person?Person->Ammo:0;
 if(Person)Person->Fire();
 const bool Shot=Person&&Person->Ammo==Ammo-1;
 const bool Mounted=Bike&&Person&&Bike->Remount(Person)&&GetPawn()==Bike;
 auto* Quest=Mode?Mode->Quest.Get():nullptr;
 const bool QuestReady=Quest&&Quest->bReady&&Quest->Artifact&&Quest->CandidateCount>5&&Quest->RadarSegmentCount>100;
 const bool Radar=QuestReady&&!Quest->ArtifactVisibleOnRadar(Quest->ArtifactLocation)&&!Quest->ArtifactVisibleOnRadar(Quest->ArtifactLocation+FVector(Quest->RadarRange+1,0,0));
 bool Placement=false;
 if(QuestReady){
  FVector P=Quest->ArtifactLocation,Start=Quest->StartLocation,End=Quest->ExitLocation;P.Z=Start.Z=End.Z=0;
  Placement=FVector::Distance(P,Start)>=5000&&FMath::PointDistToSegment(P,Start,End)>=1200;
 }
 FVector2D ClipA(200,0),ClipB(-200,0),OutsideA(200,200),OutsideB(300,300);
 const bool Clipping=ABattleQuest::ClipToCircle(ClipA,ClipB,105)&&FMath::IsNearlyEqual(ClipA.X,105.,.001)&&FMath::IsNearlyEqual(ClipB.X,-105.,.001)&&!ABattleQuest::ClipToCircle(OutsideA,OutsideB,105);
 bool BlockedPickup=false,CollectorMode=false;const bool OnFoot=Mode&&Mode->DifficultyName==FName(TEXT("Medium"));
 if(QuestReady&&Mounted){
  if(OnFoot)Bike->Dismount();
  APawn* Collector=GetPawn();CollectorMode=OnFoot?Collector->IsA<ABattleRider>():Collector->IsA<ABattleBike>();Collector->SetActorLocation(Quest->ArtifactLocation+FVector(120,0,12),false,nullptr,ETeleportType::TeleportPhysics);
  auto* Wall=GetWorld()->SpawnActor<AStaticMeshActor>(Quest->ArtifactLocation+FVector(60,0,0),FRotator::ZeroRotator);
  auto* Mesh=Wall->GetStaticMeshComponent();Mesh->SetMobility(EComponentMobility::Movable);Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Wall->SetActorScale3D(FVector(.08,.8,1.5));
  Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);Mesh->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
  Quest->Tick(.01f);BlockedPickup=!Quest->bCollected;
  Wall->SetActorEnableCollision(false);Wall->Destroy();
  Collector->SetActorLocation(Quest->ArtifactLocation+FVector(0,0,12),false,nullptr,ETeleportType::TeleportPhysics);Quest->Tick(.01f);
 }
 const bool Pickup=Quest&&Quest->bCollected&&Mode->bItemCollected&&!Quest->Artifact;
 const bool Route=Pickup&&Quest->RoutePoints.Num()>1&&Quest->EastsideRoutePointCount>1000&&Quest->RoutePoints.Last().Equals(Quest->RouteTargetLocation,1);
 UE_LOG(LogTemp,Display,TEXT("BattleQuestAudit: ready=%d radar=%d placement=%d clipping=%d pickup=%d route=%d blocked=%d onFoot=%d"),QuestReady,Radar,Placement,Clipping,Pickup,Route,BlockedPickup,OnFoot&&CollectorMode);
 ToggleMenu();const bool Paused=IsPaused()&&Menu.IsValid();
 ToggleMenu();const bool Resumed=!IsPaused()&&!Menu.IsValid();
 if(Mode){Mode->TimeRemaining=.01f;Mode->Tick(.02f);}
 PlayerTick(0);
 const bool Loss=Mode&&Mode->bRunEnded&&IsPaused()&&Menu.IsValid();
 const bool Passed=Timer&&Population&&Dismounted&&Shot&&Mounted&&Paused&&Resumed&&Loss&&QuestReady&&Radar&&Placement&&Clipping&&Pickup&&Route&&BlockedPickup&&CollectorMode;
 const FString Json=FString::Printf(TEXT("{\"difficulty\":\"%s\",\"timerSeconds\":%.0f,\"desiredCrowd\":%d,\"liveCrowd\":%d,\"timer\":%s,\"population\":%s,\"dismount\":%s,\"fire\":%s,\"remount\":%s,\"pause\":%s,\"resume\":%s,\"timeout\":%s,\"passed\":%s}"),
  Mode?*Mode->DifficultyName.ToString():TEXT("Missing"),Mode?Mode->Difficulty.TimeLimitSeconds:0,Desired,Live,Timer?TEXT("true"):TEXT("false"),Population?TEXT("true"):TEXT("false"),Dismounted?TEXT("true"):TEXT("false"),Shot?TEXT("true"):TEXT("false"),Mounted?TEXT("true"):TEXT("false"),Paused?TEXT("true"):TEXT("false"),Resumed?TEXT("true"):TEXT("false"),Loss?TEXT("true"):TEXT("false"),Passed?TEXT("true"):TEXT("false"));
 FFileHelper::SaveStringToFile(Json,*Output);
 UE_LOG(LogTemp,Display,TEXT("BattleAudit: %s"),*Json);
 UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);
#endif
}
