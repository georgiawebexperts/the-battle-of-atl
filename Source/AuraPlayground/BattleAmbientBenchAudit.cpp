#include "BattleParkFurniture.h"
#include "PiedmontPedestrian.h"
#include "PiedmontBike.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
void TickBattleAmbientBenchAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ABattleParkFurniture> Furniture;TWeakObjectPtr<APiedmontPedestrian> Visitor;float Clock=0;int Phase=0;bool Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){S.Done=true;UE_LOG(LogTemp,Display,TEXT("AmbientBenchAudit: {\"passed\":%s,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Reason);PC->ConsoleCommand(TEXT("quit"));};
#define CHECK_AMBIENT(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 CHECK_AMBIENT(S.Clock<24,"Timed out");
 auto* Mode=Cast<APiedmontRideMode>(UGameplayStatics::GetGameMode(PC));APawn* Player=PC->GetPawn();CHECK_AMBIENT(Mode&&Player,"Missing mode/player");
 if(S.Phase==0){
  for(TActorIterator<ABattleParkFurniture> It(PC->GetWorld());It;++It)if(It->Benches.Num()){S.Furniture=*It;break;}
  auto* F=S.Furniture.Get();CHECK_AMBIENT(F,"No furniture");
  const auto T=F->Benches[0];const FVector Bench=T.TransformPosition(FVector(0,55,90)),Position=Bench-T.TransformVectorNoScale(FVector(0,1,0))*1100;Player->SetActorLocation(Position,false,nullptr,ETeleportType::TeleportPhysics);
  auto* Camera=PC->GetWorld()->SpawnActor<ACameraActor>();Camera->SetActorLocation(Position+FVector(0,0,100));Camera->SetActorRotation((Bench-Camera->GetActorLocation()).Rotation());PC->SetViewTarget(Camera);PC->PlayerCameraManager->UpdateCamera(0);
  F->BenchFireRunChance=0;F->Tick(21);CHECK_AMBIENT(F->BenchEncounterAttempts==0,"Zero chance spawned encounter");
  ++Mode->RunNumber;F->BenchFireRunChance=1;F->Tick(1);CHECK_AMBIENT(F->BenchEncounterAttempts==0,"Quiet period skipped");
  // Advance the encounter's eligible-time clock, then let normal ticks run the interaction.
  F->Tick(20);CHECK_AMBIENT(F->BenchEncounterAttempts==1,"Eligible encounter did not spawn");
  for(TActorIterator<APiedmontPedestrian> It(PC->GetWorld());It;++It)if(It->ActorHasTag(TEXT("AmbientBenchIgniter"))){S.Visitor=*It;break;}
  CHECK_AMBIENT(S.Visitor.IsValid(),"No igniter");
  FVector Eye;FRotator View;PC->GetPlayerViewPoint(Eye,View);
  CHECK_AMBIENT(FVector::DotProduct((S.Visitor->GetActorLocation()-Eye).GetSafeNormal(),View.Vector())>.1f,"Encounter was not placed ahead");
  CHECK_AMBIENT(!S.Visitor->bBenchReaching&&S.Visitor->BenchesIgnited==0,"Ignition began before approach");
  F->Tick(.5f);CHECK_AMBIENT(S.Visitor->bBenchReaching,"Visible encounter did not start ignition");
  S.Phase=1;S.Clock=0;return;
 }
 if(S.Phase==1&&S.Clock>4){
  auto* F=S.Furniture.Get();CHECK_AMBIENT(S.Visitor.IsValid()&&S.Visitor->BenchesIgnited==1,"First visible encounter did not ignite");F->Tick(.5f);CHECK_AMBIENT(F->BenchFireEncountersCompleted==1,"First ignition was not recorded");
  const auto T=F->Benches[1];const FVector Bench=T.TransformPosition(FVector(0,55,90)),Position=Bench-T.TransformVectorNoScale(FVector(0,1,0))*1100;Player->SetActorLocation(Position,false,nullptr,ETeleportType::TeleportPhysics);PC->GetViewTarget()->SetActorLocation(Position+FVector(0,0,100));PC->GetViewTarget()->SetActorRotation((Bench-PC->GetViewTarget()->GetActorLocation()).Rotation());PC->PlayerCameraManager->UpdateCamera(0);
  F->Tick(40);CHECK_AMBIENT(F->BenchEncounterAttempts==2,"Second guaranteed encounter did not spawn");
  for(TActorIterator<APiedmontPedestrian> It(PC->GetWorld());It;++It)if(It->ActorHasTag(TEXT("AmbientBenchIgniter"))&&*It!=S.Visitor.Get()&&It->BenchesIgnited==0){S.Visitor=*It;break;}
  CHECK_AMBIENT(S.Visitor.IsValid(),"No second igniter");F->Tick(.5f);S.Phase=2;S.Clock=0;return;
 }
 if(S.Phase==2&&S.Clock>4){
  auto* F=S.Furniture.Get();CHECK_AMBIENT(S.Visitor.IsValid()&&S.Visitor->BenchesIgnited==1,"Second visible encounter did not ignite");F->Tick(.5f);CHECK_AMBIENT(F->BenchFireEncountersCompleted==2&&F->BenchEncounterAttempts==2,"Run did not complete exactly two encounters");
  Finish(true,TEXT("quiet period, ahead placement and two guaranteed visible ignitions passed"));
 }
#undef CHECK_AMBIENT
#endif
}
