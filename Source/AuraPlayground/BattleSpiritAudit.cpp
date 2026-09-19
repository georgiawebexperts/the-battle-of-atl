#include "BattleMacController.h"
#include "BattleSpirit.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleQuest.h"
#include "BattleZombie.h"
#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BillboardComponent.h"
#include "Engine/StaticMesh.h"
#include "Camera/CameraActor.h"
#include "BattleSpiritData.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
void ABattleMacController::TickSpiritAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleSpiritReview"))){
  if(GetWorld()->GetTimeSeconds()<5)return;
  FString Dir;FParse::Value(FCommandLine::Get(),TEXT("BattleSpiritReviewDir="),Dir);
  if(!SpiritReviewCamera){
   TActorIterator<ABattleSpirit> It(GetWorld());auto* S=It?*It:nullptr;auto* B=Cast<ABattleBike>(GetPawn());if(!S||!B){ConsoleCommand(TEXT("quit"));return;}
   B->SetActorLocation(BattleSpiritData::Approach);S->SetActorLocation(BattleSpiritData::Chase[0]);S->TryApproach(0);S->AdvanceEncounter(2);
   const FVector Eye=S->GetActorLocation()+FVector(520,-620,280),Target=S->GetActorLocation()+FVector(0,0,95);
   SpiritReviewCamera=GetWorld()->SpawnActor<ACameraActor>(Eye,(Target-Eye).Rotation());SetViewTarget(SpiritReviewCamera);
   S->Tick(.01f);
  }
  // A scene capture onto a render target, not FScreenshotRequest: this is the
  // one capture path that works in a packaged build, and until it existed no
  // shipped build's look had ever been checked.
  //
  // What was wrong with the two that came before. A console HighResShot under
  // -RenderOffscreen never came back - this branch used to hang here instead of
  // producing a picture. FScreenshotRequest did produce pictures from the
  // editor, which is why it looked like the answer for a whole day, but the
  // editor was not the one writing them: its log opens with "Requested
  // channels: 'cpu,gpu,frame,log,bookmark,screenshot,region'", so the trace
  // screenshot channel was servicing the request. A cooked build has no trace
  // server - its log says "UTS: The Unreal Trace Server binary is not
  // available" - so build 149's review reached its 45th and 95th frame, logged
  // rendered=1, and left an empty directory. Capturing in-process removes the
  // dependency on anything being staged and enabled around the game.
  auto EnsureCapture=[&](){
   if(SpiritReviewCapture||Dir.IsEmpty()||!SpiritReviewCamera)return;
   SpiritReviewTarget=NewObject<UTextureRenderTarget2D>(this);
   SpiritReviewTarget->RenderTargetFormat=RTF_RGBA8;
   SpiritReviewTarget->InitAutoFormat(1280,720);
   SpiritReviewTarget->UpdateResourceImmediate(true);
   SpiritReviewCapture=NewObject<USceneCaptureComponent2D>(this);
   SpiritReviewCapture->TextureTarget=SpiritReviewTarget;
   // Final colour, not the scene colour: the review has to show what the
   // player sees, tone mapping and all, because the complaint being checked
   // was about how a pale body and its rim read on screen.
   SpiritReviewCapture->CaptureSource=ESceneCaptureSource::SCS_FinalColorLDR;
   // Every frame, and keeping its render state between them, because a one-shot
   // capture has no eye adaptation to work from: the first attempt at this wrote
   // a correctly framed picture of the right scene, about four stops dark - it
   // rendered as dusk what the player sees as midday, which would have been a
   // worse review than no review at all. Persistent state lets the capture's
   // exposure settle the way a viewport's does.
   SpiritReviewCapture->bCaptureEveryFrame=true;
   SpiritReviewCapture->bAlwaysPersistRenderingState=true;
   SpiritReviewCapture->bCaptureOnMovement=false;
   SpiritReviewCapture->RegisterComponent();
  };
  auto Capture=[&](const TCHAR* Name){
   if(Dir.IsEmpty()||!SpiritReviewTarget)return;
   UKismetRenderingLibrary::ExportRenderTarget(this,SpiritReviewTarget,Dir,Name);
   // A packaged Mac build runs sandboxed - its own savegames land under
   // ~/Library/Containers/com.webexperts.battleofatl/Data/... - so the first
   // cooked attempt at this wrote nothing and said so only as a Blueprint
   // warning: "ExportRenderTarget: FileWrite failed to create", because the
   // review directory the harness passes lives on the assets volume, outside the
   // container. Nothing in the editor can reproduce that, which is why it took a
   // packaged run to find. Fall back to a directory the app is allowed to write,
   // and say which one it used so the caller can collect from there.
   FString Written=Dir;
   if(!IFileManager::Get().FileExists(*FPaths::Combine(Dir,Name))){
    Written=FPaths::ProjectSavedDir()/TEXT("SpiritReview");
    IFileManager::Get().MakeDirectory(*Written,true);
    UKismetRenderingLibrary::ExportRenderTarget(this,SpiritReviewTarget,Written,Name);
   }
   UE_LOG(LogTemp,Display,TEXT("BattleSpiritReview: capture %s -> %s/%s"),Name,*Written,Name);
  };
  // Frame counts are the wrong clock for this review, and measuring that cost a
  // second pass: the trace-captured editor run and the packaged run both
  // exported "frame 45" and "frame 95", and the bear was pale in one and near
  // black in the same view in the other. Nothing about the material differed -
  // `state=3` (Active, so Reveal is pinned at 1) in both logs. What differed was
  // wall-clock time: the traced run's frames were far slower, so its frame 95
  // arrived seconds later, and the ghost's own light pulses on time
  // (`SetIntensity(950 * Reveal * Pulse)`). A review that samples an oscillating
  // light by frame number measures the machine. Seconds after setup instead.
  SpiritReviewFrames++;
  SpiritReviewClock+=Dt;
  // The capture draws every frame, so it follows the camera every frame - the
  // camera itself is moved at frame 70 for the approach view, and the capture
  // has to be standing where it is by the time that frame is exported.
  if(Dir.IsEmpty()==false&&SpiritReviewCamera){
   EnsureCapture();
   if(SpiritReviewCapture)SpiritReviewCapture->SetWorldLocationAndRotation(SpiritReviewCamera->GetActorLocation(),SpiritReviewCamera->GetActorRotation());
  }
  // Say what the capture is actually looking at. A review that writes two
  // pictures and no numbers cannot tell "the spirit is not there" from "the
  // spirit is there and invisible", which is exactly the mistake that cost a
  // pass over the bear's material.
  if(SpiritReviewClock>=3.0f&&SpiritReviewShot<1||SpiritReviewClock>=4.5f&&SpiritReviewShot<2){
   SpiritReviewShot++;
   TActorIterator<ABattleSpirit> It(GetWorld());auto* S=It?*It:nullptr;
   if(S&&S->BearBody){
    const FBoxSphereBounds B=S->BearBody->Bounds;
    UE_LOG(LogTemp,Display,TEXT("BattleSpiritReview: state=%d actor=%s body_visible=%d origin=%s extent=%s material=%s camera=%s rendered=%d"),
     int32(S->State),*S->GetActorLocation().ToCompactString(),S->BearBody->IsVisible()?1:0,
     *B.Origin.ToCompactString(),*B.BoxExtent.ToCompactString(),*GetNameSafe(S->BearBody->GetMaterial(0)),
     SpiritReviewCamera?*SpiritReviewCamera->GetActorLocation().ToCompactString():TEXT("none"),
     S->BearBody->WasRecentlyRendered(1.f)?1:0);
   }else{
    UE_LOG(LogTemp,Display,TEXT("BattleSpiritReview: no spirit actor or no bear body component"));
   }
  }
  if(SpiritReviewClock>=3.0f&&SpiritReviewShot==1&&!bSpiritReviewTookThreeQuarter){bSpiritReviewTookThreeQuarter=true;Capture(TEXT("spirit-three-quarter.png"));}
  if(!Dir.IsEmpty()&&SpiritReviewClock>=4.0f&&!bSpiritReviewMoved){
   bSpiritReviewMoved=true;
   // And the view that matters: off the approach, which is how the rider meets
   // it, rather than from the side the art was authored to be looked at.
   TActorIterator<ABattleSpirit> It(GetWorld());auto* Bear=It?*It:nullptr;
   if(Bear&&SpiritReviewCamera){
    const FVector P=Bear->GetActorLocation();
    // Square on to the bear's own flank: the silhouette is what has to read.
    const FVector From=P-Bear->GetActorForwardVector()*180.f+Bear->GetActorRightVector()*520.f+FVector(0,0,140);
    SpiritReviewCamera->SetActorLocationAndRotation(From,(P+FVector(0,0,80)-From).Rotation());
   }
  }
  if(SpiritReviewClock>=4.5f&&!bSpiritReviewTookApproach){bSpiritReviewTookApproach=true;Capture(TEXT("spirit-approach.png"));}
  if(SpiritReviewClock>6.f)ConsoleCommand(TEXT("quit"));return;
 }
 if(bSpiritAuditDone||GetWorld()->GetTimeSeconds()<5)return;bSpiritAuditDone=true;
 auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* B=Cast<ABattleBike>(GetPawn());int Checks=0;
 auto End=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BattleSpiritAudit: {\"passed\":%s,\"checks\":%d,\"reason\":\"%s\",\"presentation_enabled\":true}"),Pass?TEXT("true"):TEXT("false"),Checks,Reason);ConsoleCommand(TEXT("quit"));};
#define SPIRITCHECK(C,R) if(!(C)){End(false,TEXT(R));return;}else{Checks++;}
 SPIRITCHECK(M&&B&&M->Quest&&!M->bTutorialActive&&M->StartCountdown<=0,"Missing timed game");
 if(M->Enemies)M->Enemies->bFreezeSpawns=true;
 for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
 for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)It->Destroy();
 auto New=[&](){auto* S=GetWorld()->SpawnActor<ABattleSpirit>(B->GetActorLocation(),FRotator::ZeroRotator);S->SetActorTickEnabled(false);return S;};
  auto* S=New();SPIRITCHECK(S&&S->bPresentationReady&&S->SpiritBillboard&&S->SpiritBillboard->Sprite&&S->FigureParts.Num()>=9&&S->Wisps.Num()>=3&&!S->GetActorEnableCollision()&&!S->CanBeDamaged(),"Visible spectral figure presentation or pass-through safety missing");
 M->bTutorialActive=true;SPIRITCHECK(!S->TryApproach(0)&&S->State==EBattleSpiritState::Untried,"Tutorial consumed opportunity");M->bTutorialActive=false;
 M->StartCountdown=1;SPIRITCHECK(!S->TryApproach(0),"Countdown spawned spirit");M->StartCountdown=0;
 SPIRITCHECK(!S->TryApproach(-1)&&!S->TryApproach(1)&&S->State==EBattleSpiritState::Untried,"Invalid roll consumed opportunity");
 SPIRITCHECK(!S->TryApproach(.50f)&&S->State==EBattleSpiritState::Absent&&!S->TryApproach(0),"50/50 cutoff or repeated visit failed");S->Destroy();
 S=New();SPIRITCHECK(S->TryApproach(.4999f)&&S->State==EBattleSpiritState::Appearing&&!S->TryCatch(),"Reveal must precede reward");
 UGameplayStatics::SetGamePaused(this,true);S->AdvanceEncounter(3);SPIRITCHECK(S->Age==0&&!S->TryCatch(),"Paused encounter advanced");UGameplayStatics::SetGamePaused(this,false);
 S->AdvanceEncounter(1.6f);SPIRITCHECK(S->State==EBattleSpiritState::Active,"Reveal never became active");
 auto* Mesh=NewObject<UStaticMeshComponent>(S);Mesh->SetupAttachment(S->GetRootComponent());Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere.Sphere")));Mesh->SetCollisionProfileName(TEXT("BlockAll"));Mesh->RegisterComponent();
 FCollisionQueryParams Q(SCENE_QUERY_STAT(SpiritBullet),false,B);FHitResult H;const FVector From=S->GetActorLocation()+FVector(0,-100,0),To=S->GetActorLocation()+FVector(0,100,0);
 SPIRITCHECK(!GetWorld()->LineTraceSingleByChannel(H,From,To,ECC_Visibility,Q)||H.GetActor()!=S,"Projectile trace hit spirit mesh");
 S->SetActorEnableCollision(true);SPIRITCHECK(GetWorld()->LineTraceSingleByChannel(H,From,To,ECC_Visibility,Q)&&H.GetActor()==S,"Projectile test fixture was not blocking");S->SetActorEnableCollision(false);
 const FVector Phone=M->Quest->ArtifactLocation;const int Checkpoint=M->Quest->NextCheckpoint;const bool Collected=M->Quest->bCollected;const float Elapsed=M->RunElapsed;B->RiderHealth=35;M->TimeRemaining=50;
 SPIRITCHECK(S->TryCatch()&&B->RiderHealth==100&&FMath::IsNearlyEqual(M->TimeRemaining,M->Difficulty.TimeLimitSeconds)&&S->Rewards==1,"Mounted catch failed restore");
 SPIRITCHECK(M->Quest->ArtifactLocation==Phone&&M->Quest->NextCheckpoint==Checkpoint&&M->Quest->bCollected==Collected&&M->RunElapsed==Elapsed,"Catch reset quest or elapsed record");
 SPIRITCHECK(!S->TryCatch()&&!S->TryApproach(0),"Repeated reward or opportunity");S->AdvanceEncounter(2);SPIRITCHECK(S->State==EBattleSpiritState::Resolved,"Fade never resolved");S->Destroy();
 S=New();SPIRITCHECK(S->TryApproach(0),"Fresh run fixture failed");S->AdvanceEncounter(2);M->TimeRemaining=M->Difficulty.TimeLimitSeconds+30;SPIRITCHECK(S->TryCatch()&&M->TimeRemaining==M->Difficulty.TimeLimitSeconds+30,"Catch removed earned time");S->Destroy();
 S=New();SPIRITCHECK(S->TryApproach(0)&&B->Dismount(),"Dismount fixture failed");auto* P=Cast<ABattleRider>(GetPawn());
 SPIRITCHECK(P&&S->State==EBattleSpiritState::Fading&&!S->TryCatch(),"Dismount did not synchronously cancel");
 auto* FootOpportunity=New();SPIRITCHECK(!FootOpportunity->TryApproach(0)&&FootOpportunity->State==EBattleSpiritState::Absent,"Foot approach should consume chance");
 SPIRITCHECK(P->MountBike()&&!S->TryApproach(0)&&!S->TryCatch()&&!FootOpportunity->TryApproach(0),"Same-frame remount restored chance");S->Destroy();FootOpportunity->Destroy();
 S=New();SPIRITCHECK(S->TryApproach(0),"Expiry fixture failed");S->AdvanceEncounter(12);SPIRITCHECK(S->State==EBattleSpiritState::Fading&&!S->TryCatch(),"Expired opportunity rewarded");S->Destroy();
 S=New();SPIRITCHECK(S->TryApproach(0),"Stun fixture failed");B->StunRemaining=2;S->AdvanceEncounter(.1f);B->StunRemaining=0;SPIRITCHECK(S->State==EBattleSpiritState::Fading&&!S->TryApproach(0),"Knockoff/stun failed cancellation");S->Destroy();
 S=New();SPIRITCHECK(S->TryApproach(0),"End fixture failed");M->bRunEnded=true;S->AdvanceEncounter(.1f);M->bRunEnded=false;SPIRITCHECK(S->State==EBattleSpiritState::Fading&&!S->TryCatch(),"Run end restored opportunity");S->Destroy();
 S=New();SPIRITCHECK(S->TryApproach(0),"Death fixture failed");B->DamageGrace=0;B->ApplyRiderDamage(1000);SPIRITCHECK(S->State==EBattleSpiritState::Fading&&!S->TryCatch()&&!S->TryApproach(0),"Death failed immediate cancellation");
 End(true,TEXT("Visible spectral human figure, native lifecycle, mounted restore, quest preservation, rarity boundary, pause, expiry, dismount/remount, stun, death and projectile pass-through pass"));
#undef SPIRITCHECK
#endif
}
