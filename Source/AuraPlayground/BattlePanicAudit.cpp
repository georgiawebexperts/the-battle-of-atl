#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleQuest.h"
#include "BattleZombie.h"
#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealClient.h"
#include "Misc/CommandLine.h"

void TickBattlePanicAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<APiedmontPedestrian> People[2];FVector Starts[2],Source;TWeakObjectPtr<ACameraActor> Camera;int Stage=0;float Clock=0,Travel[2]={0,0};bool Done=false;};static FState S;
 auto* World=PC->GetWorld();if(S.World!=World){S=FState();S.World=World;}if(S.Done||World->GetTimeSeconds()<5)return;
 auto Finish=[&](bool Pass,const TCHAR* Why){S.Done=true;UE_LOG(LogTemp,Display,TEXT("BattlePanicAudit: {\"passed\":%s,\"stage\":%d,\"reason\":\"%s\",\"male_travel_cm\":%.2f,\"female_travel_cm\":%.2f}"),Pass?TEXT("true"):TEXT("false"),S.Stage,Why,S.Travel[0],S.Travel[1]);PC->ConsoleCommand(TEXT("quit"));};
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(PC));if(!Mode)return;
 auto* Rider=Cast<ABattleRider>(PC->GetPawn());auto* Bike=Rider?Rider->ParkedBike.Get():Cast<ABattleBike>(PC->GetPawn());if(!Bike){Finish(false,TEXT("Missing player"));return;}
 auto Capture=[&](const TCHAR* Name){FString Dir;if(FParse::Value(FCommandLine::Get(),TEXT("BattlePanicReviewDir="),Dir))FScreenshotRequest::RequestScreenshot(Dir/Name,false,false);};
 if(S.Stage==0){
  for(TActorIterator<APiedmontTrafficDirector> It(World);It;++It){It->DesiredPopulation=0;It->SetActorTickEnabled(false);}
  for(TActorIterator<APiedmontPedestrian> It(World);It;++It)It->Destroy();
  if(Mode->Enemies)Mode->Enemies->bFreezeSpawns=true;
  if(!Bike->Dismount()){Finish(false,TEXT("Dismount failed"));return;}
  Rider=Cast<ABattleRider>(PC->GetPawn());if(!Rider||!Rider->ToggleDrawWeapon()){Finish(false,TEXT("Draw failed"));return;}
  S.Source=Rider->GetActorLocation();auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
  for(int I=0;I<2;++I){FNavLocation Point;if(!Nav||!Nav->ProjectPointToNavigation(S.Source+FVector(700,I?220:-220,0),Point,FVector(350,350,500))){Finish(false,TEXT("Witness navigation missing"));return;}
   FTransform T(FRotator::ZeroRotator,Point.Location+FVector(0,0,98));auto* P=World->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),T,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
   P->CityAppearanceVariant=I;P->CityOutfitVariant=I?4:2;P->FinishSpawning(T);P->PauseRemaining=100;S.People[I]=P;S.Starts[I]=P->GetActorLocation();
  }
  PC->SetControlRotation(FRotator(65,180,0));S.Stage=1;S.Clock=0;return;
 }
 S.Clock+=Dt;
 for(int I=0;I<2;++I)if(!S.People[I].IsValid()){Finish(false,TEXT("Witness lost"));return;}
 if(S.Stage==1&&S.Clock>.7f){
  const int Ammo=Rider->Ammo;if(!Rider->Fire()||Rider->Ammo!=Ammo-1){Finish(false,TEXT("Actual player shot failed"));return;}
  for(auto& W:S.People)if(W->PanicRemaining<17.9f||W->PauseRemaining!=0){Finish(false,TEXT("Shot did not interrupt idle and trigger panic"));return;}
  const FVector Focus=(S.People[0]->GetActorLocation()+S.People[1]->GetActorLocation())*.5f;
  S.Camera=World->SpawnActor<ACameraActor>(Focus+FVector(0,-750,280),(Focus-(Focus+FVector(0,-750,280))).Rotation());PC->SetViewTarget(S.Camera.Get());S.Stage=2;S.Clock=0;
 }
 if(S.Camera.IsValid()&&S.Stage>=2){const FVector Focus=(S.People[0]->GetActorLocation()+S.People[1]->GetActorLocation())*.5f;const FVector Eye=Focus+FVector(0,-750,280);S.Camera->SetActorLocationAndRotation(Eye,(Focus-Eye).Rotation());}
 if(S.Stage==2&&S.Clock>1){Capture(TEXT("panic-running.png"));S.Stage=3;}
 if(S.Stage==3&&S.Clock>3){
  for(int I=0;I<2;++I){auto* P=S.People[I].Get();S.Travel[I]=FVector::Dist2D(P->GetActorLocation(),S.Starts[I]);
   if(S.Travel[I]<450||FVector::Dist2D(P->GetActorLocation(),S.Source)<FVector::Dist2D(S.Starts[I],S.Source)+300||P->PanicRemaining<14||P->GetCharacterMovement()->MaxWalkSpeed<400){Finish(false,TEXT("Live witness did not flee away and remain panicked"));return;}}
  Capture(TEXT("panic-away.png"));S.Stage=4;S.Clock=0;
 }
 // Let the requested screenshot render before switching back to the player.
 if(S.Stage==4&&S.Clock>.2f){PC->SetViewTarget(Rider);PC->SetControlRotation(FRotator(65,180,0));S.Stage=7;S.Clock=0;}
 if(S.Stage==7&&S.Clock>.2f){
  if(!Rider->Fire()){Finish(false,TEXT("Repeat gunshot failed"));return;}
  for(auto& W:S.People)if(W->PanicRemaining<17.9f){Finish(false,TEXT("Repeat shot did not refresh nearby panic"));return;}
  PC->SetViewTarget(S.Camera.Get());S.Stage=5;S.Clock=0;
 }
 if(S.Stage==5&&S.Clock>18.3f){
  for(auto& W:S.People)if(W->PanicRemaining>0||W->GetCharacterMovement()->MaxWalkSpeed!=135){Finish(false,TEXT("Panic did not expire and restore walking speed"));return;}
  Capture(TEXT("panic-recovered.png"));S.Stage=6;S.Clock=0;
 }
 if(S.Stage==6&&S.Clock>.3f)Finish(true,TEXT("Real shots trigger live male/female flight, sustained panic, refresh and walking-speed recovery"));
 if(S.Clock>25)Finish(false,TEXT("Panic audit timeout"));
#endif
}
