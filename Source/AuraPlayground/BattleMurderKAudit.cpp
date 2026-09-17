#include "BattleMacController.h"
#include "BattleMurderK.h"
#include "BattleBike.h"
#include "BattleQuest.h"
#include "BattleZombie.h"
#include "BattleCheckpoints.h"
#include "Components/TextRenderComponent.h"
#include "Camera/CameraActor.h"
#include "Misc/CommandLine.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

void ABattleMacController::TickMurderKAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleMurderKReview"))){
  if(GetWorld()->GetTimeSeconds()<5)return;
  if(!MurderKReviewCamera){
   TActorIterator<ABattleMurderK> It(GetWorld());if(!It){ConsoleCommand(TEXT("quit"));return;}auto* Store=*It;
   const FVector Eye=Store->GetActorTransform().TransformPosition(FVector(0,2000,900));
   const FVector Target=Store->GetActorTransform().TransformPosition(FVector(300,-1350,1150));
   MurderKReviewCamera=GetWorld()->SpawnActor<ACameraActor>(Eye,(Target-Eye).Rotation());SetViewTarget(MurderKReviewCamera);
   if(auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this)))if(Mode->Quest&&Mode->Enemies){Mode->Quest->bCollected=true;const auto& A=BattleCheckpoints::Anchors[0];if(auto* Bike=Cast<ABattleBike>(GetPawn()))Bike->SetActorLocation(FVector(A.X,A.Y,A.Z+100));Mode->Enemies->TickMurderK(0);}
  }
  MurderKReviewFrames++;if(MurderKReviewFrames==45)ConsoleCommand(TEXT("HighResShot 1280x720"));if(MurderKReviewFrames>90)ConsoleCommand(TEXT("quit"));return;
 }
 if(bMurderKAuditDone||GetWorld()->GetTimeSeconds()<5)return;bMurderKAuditDone=true;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* Bike=Cast<ABattleBike>(GetPawn());int32 Checks=0;
 auto End=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BattleMurderKAudit: {\"passed\":%s,\"checks\":%d,\"difficulty\":\"%s\",\"timer\":%.0f,\"shooters\":%d,\"punks\":%d,\"bums\":%d,\"fights\":%d,\"ambient_police\":%d,\"knife\":%s,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Checks,Mode?*Mode->DifficultyName.ToString():TEXT("none"),Mode?Mode->Difficulty.TimeLimitSeconds:0,Mode&&Mode->Enemies?Mode->Enemies->MurderKGunmenSpawned:-1,Mode&&Mode->Enemies?Mode->Enemies->MurderKPunksSpawned:-1,Mode&&Mode->Enemies?Mode->Enemies->MurderKBumsSpawned:-1,Mode&&Mode->Enemies?Mode->Enemies->MurderKFightSpots:-1,Mode&&Mode->Enemies?Mode->Enemies->MurderKAmbientPolice:-1,Mode&&Mode->Enemies&&Mode->Enemies->bMurderKKnifeSpawned?TEXT("true"):TEXT("false"),Reason);ConsoleCommand(TEXT("quit"));};
#define MKCHECK(C,R) if(!(C)){End(false,TEXT(R));return;}else{Checks++;}
 MKCHECK(Mode&&Bike&&Mode->Quest&&Mode->Enemies,"Missing live game state");
 const float Expected=Mode->DifficultyName==TEXT("Easy")?240.f:(Mode->DifficultyName==TEXT("Medium")?210.f:180.f);
 MKCHECK(FMath::IsNearlyEqual(Mode->Difficulty.TimeLimitSeconds,Expected),"Difficulty timer is stale");
 TActorIterator<ABattleMurderK> StoreIt(GetWorld());ABattleMurderK* Store=StoreIt?*StoreIt:nullptr;
 MKCHECK(Store&&Store->StoreSign&&Store->StoreSign->Text.ToString()==TEXT("MURDER K"),"Murder K landmark or sign missing");
 MKCHECK(Store->StoreMass&&Store->StoreMass->GetRelativeLocation().Y<0,"Murder K is not on the east side of the trail");
 MKCHECK(Store->OfficeTower&&Store->GlassWing,"725 Ponce office tower or glass wing missing");
 MKCHECK(Store->TrailApron&&Store->TrailApron->GetRelativeScale3D().Y>=14.f,"Murder K riding plaza is not wide enough");
 MKCHECK(Store->BrickParts.Num()>=1&&Store->GlassParts.Num()>=50&&Store->ConcreteParts.Num()>=8,"Storefront detail is incomplete");
 const auto& A=BattleCheckpoints::Anchors[0];Bike->SetActorLocation(FVector(A.X,A.Y,A.Z+100),false,nullptr,ETeleportType::TeleportPhysics);Mode->Quest->bCollected=true;Mode->bTutorialActive=false;Mode->StartCountdown=0;Bike->DamageGrace=0;Bike->RespawnRemaining=0;Mode->Enemies->TickMurderK(0);
 MKCHECK(Mode->Enemies->bMurderKActivated,"Murder K encounter did not activate");
 MKCHECK(Mode->Enemies->MurderKGunmenSpawned==Mode->Difficulty.KrogerShooters,"Murder K shooter count does not match difficulty");
 MKCHECK(Mode->Enemies->bMurderKKnifeSpawned==(Mode->Difficulty.KnifeBehavior>0),"Murder K knife behavior does not match difficulty");
 const int ExpectedPunks=Mode->DifficultyName==TEXT("Easy")?5:(Mode->DifficultyName==TEXT("Medium")?7:9);
 MKCHECK(Mode->Enemies->MurderKPunksSpawned==ExpectedPunks&&Mode->Enemies->MurderKFightSpots==2,"Murder K punk crowd or fight spots missing");
 MKCHECK(Mode->Enemies->MurderKBumsSpawned==3,"Murder K sleeping bums missing");
 MKCHECK(Mode->Enemies->MurderKAmbientPolice==(Mode->DifficultyName==TEXT("Easy")?1:2),"Murder K ambient police missing");
 End(true,TEXT("Landmark, four-minute timer family, dense punk/bum crowd, two fights, ambient non-taser police and difficulty encounter pass"));
#undef MKCHECK
#endif
}
