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
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* Bike=Cast<ABattleBike>(GetPawn());int32 Checks=0,WallPunks=0;
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
 WallPunks=0;for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)if(It->Tags.Contains(TEXT("BattlePunkWall")))WallPunks++;
 MKCHECK(Mode->Enemies->Tags.Contains(TEXT("MurderKPunkWallActivated"))&&WallPunks==6,"Murder K punk wall missing or wrong size");
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
