#include "BattleMacController.h"
#include "BattleMurderK.h"
#include "BattleBike.h"
#include "BattleQuest.h"
#include "BattleZombie.h"
#include "BattleCheckpoints.h"
#include "Components/TextRenderComponent.h"
#include "Camera/CameraActor.h"
#include "BattleRoadCar.h"
#include "BattleScooterTraffic.h"
#include "BattleSpirit.h"
#include "PiedmontPedestrian.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealClient.h"

void ABattleMacController::TickMurderKAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleMurderKReview"))){
  if(GetWorld()->GetTimeSeconds()<5)return;
  FString ReviewDir;FParse::Value(FCommandLine::Get(),TEXT("BattleMurderKReviewDir="),ReviewDir);
  if(!MurderKReviewCamera){
   TActorIterator<ABattleMurderK> It(GetWorld());if(!It){ConsoleCommand(TEXT("quit"));return;}auto* Store=*It;
   const FVector Eye=Store->GetActorTransform().TransformPosition(FVector(0,2000,900));
   const FVector Target=Store->GetActorTransform().TransformPosition(FVector(300,-1350,1150));
   MurderKReviewCamera=GetWorld()->SpawnActor<ACameraActor>(Eye,(Target-Eye).Rotation());SetViewTarget(MurderKReviewCamera);
   if(auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this)))if(Mode->Quest&&Mode->Enemies){Mode->Quest->bCollected=true;const auto& A=BattleCheckpoints::Anchors[0];if(auto* Bike=Cast<ABattleBike>(GetPawn()))Bike->SetActorLocation(FVector(A.X,A.Y,A.Z+100));Mode->Enemies->TickMurderK(0);}
   if(!ReviewDir.IsEmpty())IFileManager::Get().MakeDirectory(*ReviewDir,true);
   // What the plaza actually contains, so "the area is too small" and "there is a
   // lot of traffic there" become numbers instead of impressions. Radius is
   // 200 m, which covers the plaza, Ponce de Leon and the BeltLine approach.
   int32 Cars=0,Scooters=0,Peds=0;const FVector Origin=Store->GetActorLocation();
   for(TActorIterator<ABattleRoadCar> C(GetWorld());C;++C)if(FVector::Dist2D(C->GetActorLocation(),Origin)<20000.f)++Cars;
   for(TActorIterator<ABattleScooterTraffic> S(GetWorld());S;++S)if(FVector::Dist2D(S->GetActorLocation(),Origin)<20000.f)++Scooters;
   for(TActorIterator<APiedmontPedestrian> P(GetWorld());P;++P)if(FVector::Dist2D(P->GetActorLocation(),Origin)<20000.f)++Peds;
   int32 SpiritPresent=0;for(TActorIterator<ABattleSpirit> B(GetWorld());B;++B)if(B->IsActorTickEnabled())++SpiritPresent;
   UE_LOG(LogTemp,Display,TEXT("MurderKReview: location=%s apron_scale=%s cars_200m=%d scooters_200m=%d pedestrians_200m=%d dancers=%d rioters=%d bodies=%d spirit=%d"),
    *Origin.ToString(),*Store->TrailApron->GetRelativeScale3D().ToString(),Cars,Scooters,Peds,Store->Dancers,Store->Rioters,Store->BodiesDown,SpiritPresent);
  }
  ++MurderKReviewFrames;
  // Three views in one run: the storefront as a rider sees it, the plaza from the
  // trail side, and an aerial of the approach so the road layout and the traffic
  // around it are both visible. Spaced by real seconds, not frames: a screenshot
  // request needs the next few frames to render before the next one lands.
  if(!ReviewDir.IsEmpty()){
   static float ReviewClock=0;ReviewClock+=Dt;
   auto* Store=[&]()->ABattleMurderK*{TActorIterator<ABattleMurderK> It(GetWorld());return It?*It:nullptr;}();
   static int32 Stage=0;
   if(Store&&Stage<3&&ReviewClock>1.f+1.4f*Stage){
    const TCHAR* Name=Stage==0?TEXT("murderk-storefront.png"):(Stage==1?TEXT("murderk-plaza.png"):TEXT("murderk-aerial.png"));
    const FVector LocalEye=Stage==0?FVector(0,2000,900):(Stage==1?FVector(0,5600,1900):FVector(-200,7400,7000));
    const FVector LocalTarget=Stage==0?FVector(300,-1350,1150):(Stage==1?FVector(200,-1400,700):FVector(200,-1200,0));
    const FVector Eye=Store->GetActorTransform().TransformPosition(LocalEye);
    const FVector Target=Store->GetActorTransform().TransformPosition(LocalTarget);
    MurderKReviewCamera->SetActorLocationAndRotation(Eye,(Target-Eye).Rotation());
    FScreenshotRequest::RequestScreenshot(ReviewDir/FString(Name),false,false);
    ++Stage;
   }
   if(ReviewClock>6.f)ConsoleCommand(TEXT("quit"));
  }else{
   if(MurderKReviewFrames==45)ConsoleCommand(TEXT("HighResShot 1280x720"));
   if(MurderKReviewFrames>90)ConsoleCommand(TEXT("quit"));
  }
  return;
 }
 if(bMurderKAuditDone||GetWorld()->GetTimeSeconds()<5)return;bMurderKAuditDone=true;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* Bike=Cast<ABattleBike>(GetPawn());int32 Checks=0,WallPunks=0,ToughWallPunks=0;
 TActorIterator<ABattleMurderK> StoreIt(GetWorld());ABattleMurderK* Store=StoreIt?*StoreIt:nullptr;
 auto End=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BattleMurderKAudit: {\"passed\":%s,\"checks\":%d,\"difficulty\":\"%s\",\"timer\":%.0f,\"shooters\":%d,\"punks\":%d,\"wall\":%d,\"bums\":%d,\"fights\":%d,\"ambient_police\":%d,\"knife\":%s,\"dancers\":%d,\"rioters\":%d,\"plaza_bodies\":%d,\"chant\":\"%s\",\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Checks,Mode?*Mode->DifficultyName.ToString():TEXT("none"),Mode?Mode->Difficulty.TimeLimitSeconds:0,Mode&&Mode->Enemies?Mode->Enemies->MurderKGunmenSpawned:-1,Mode&&Mode->Enemies?Mode->Enemies->MurderKPunksSpawned:-1,WallPunks,Mode&&Mode->Enemies?Mode->Enemies->MurderKBumsSpawned:-1,Mode&&Mode->Enemies?Mode->Enemies->MurderKFightSpots:-1,Mode&&Mode->Enemies?Mode->Enemies->MurderKAmbientPolice:-1,Mode&&Mode->Enemies&&Mode->Enemies->bMurderKKnifeSpawned?TEXT("true"):TEXT("false"),Store?Store->Dancers:-1,Store?Store->Rioters:-1,Store?Store->BodiesDown:-1,Store?*Store->ShoutText:TEXT(""),Reason);ConsoleCommand(TEXT("quit"));};
#define MKCHECK(C,R) if(!(C)){End(false,TEXT(R));return;}else{Checks++;}
 MKCHECK(Mode&&Bike&&Mode->Quest&&Mode->Enemies,"Missing live game state");
 const float Expected=Mode->DifficultyName==TEXT("Easy")?210.f:(Mode->DifficultyName==TEXT("Medium")?180.f:150.f);
 MKCHECK(FMath::IsNearlyEqual(Mode->Difficulty.TimeLimitSeconds,Expected),"Difficulty timer is stale");
 MKCHECK(Store&&Store->StoreSign&&Store->StoreSign->Text.ToString()==TEXT("MURDER K"),"Murder K landmark or sign missing");
 MKCHECK(Store->StoreMass&&Store->StoreMass->GetRelativeLocation().Y<0,"Murder K is not on the east side of the trail");
 MKCHECK(Store->OfficeTower&&Store->GlassWing,"725 Ponce office tower or glass wing missing");
 MKCHECK(Store->TrailApron&&Store->TrailApron->GetRelativeScale3D().Y>=14.f,"Murder K riding plaza is not wide enough");
 MKCHECK(Store->BrickParts.Num()>=1&&Store->GlassParts.Num()>=50&&Store->ConcreteParts.Num()>=8,"Storefront detail is incomplete");
 // The plaza is the loudest stretch on the route. It must already be a party and
 // a riot when the rider arrives, not populated around him as he passes through.
 MKCHECK(Store->Dancers>=6&&Store->Rioters>=8&&Store->BodiesDown>=3,"Murder K plaza is not in riot");
 MKCHECK(!Store->ShoutText.IsEmpty(),"Murder K chant is silent");
 const auto& A=BattleCheckpoints::Anchors[0];Bike->SetActorLocation(FVector(A.X,A.Y,A.Z+100),false,nullptr,ETeleportType::TeleportPhysics);Mode->Quest->bCollected=true;Mode->bTutorialActive=false;Mode->StartCountdown=0;Bike->DamageGrace=0;Bike->RespawnRemaining=0;Mode->Enemies->TickMurderK(0);
 MKCHECK(Mode->Enemies->bMurderKActivated,"Murder K encounter did not activate");
 WallPunks=0;ToughWallPunks=0;for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)if(It->Tags.Contains(TEXT("BattlePunkWall"))){WallPunks++;if(It->Health>=200&&It->WeaponDropChance>=1.f)ToughWallPunks++;}
 MKCHECK(Mode->Enemies->Tags.Contains(TEXT("MurderKPunkWallActivated"))&&WallPunks==6&&ToughWallPunks==6,"Murder K punk wall missing, wrong size, or not shoot-through grade");
 const int32 WantShooters=FMath::Clamp(FMath::Max(Mode->Difficulty.KrogerShooters,Mode->DifficultyName==TEXT("Easy")?1:(Mode->DifficultyName==TEXT("Medium")?2:3)),0,3);
 MKCHECK(Mode->Enemies->MurderKGunmenSpawned==WantShooters,"Murder K shooter count does not match the Kroger floor");
 MKCHECK(Mode->Enemies->bMurderKKnifeSpawned==(Mode->Difficulty.KnifeBehavior>0),"Murder K knife behavior does not match difficulty");
 const int ExpectedPunks=Mode->DifficultyName==TEXT("Easy")?7:(Mode->DifficultyName==TEXT("Medium")?9:11);
 MKCHECK(Mode->Enemies->MurderKPunksSpawned==ExpectedPunks&&Mode->Enemies->MurderKFightSpots==3,"Murder K punk crowd or fight spots missing");
 MKCHECK(Mode->Enemies->MurderKBumsSpawned==3,"Murder K sleeping bums missing");
 MKCHECK(Mode->Enemies->MurderKAmbientPolice==(Mode->DifficultyName==TEXT("Easy")?2:4),"Murder K ambient police missing");
 End(true,TEXT("Landmark, four-minute timer family, a full plaza riot and dance party, three brawls, a six-punk shoot-through wall, ambient non-taser police and the difficulty encounter all pass"));
#undef MKCHECK
#endif
}
