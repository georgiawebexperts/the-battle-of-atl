#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleZombie.h"
#include "PiedmontPedestrian.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
void TickBattleEntranceWalkAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState {TWeakObjectPtr<UWorld> World;TArray<TPair<FVector,FVector>> Routes;TWeakObjectPtr<ABattleRider> Rider;int32 Route=0,Leg=0,Completed=0;float Clock=0,Settle=0,Falling=0;bool Started=false,Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;
 auto Finish=[&](bool Pass,const TCHAR* Why){S.Done=true;if(S.Rider.IsValid())S.Rider->GetCharacterMovement()->StopMovementImmediately();UE_LOG(LogTemp,Display,TEXT("EntranceWalkAudit: {\"passed\":%s,\"reason\":\"%s\",\"completed_legs\":%d,\"route\":%d}"),Pass?TEXT("true"):TEXT("false"),Why,S.Completed,S.Route);PC->ConsoleCommand(TEXT("quit"));};
 if(!S.Started){
  FString Raw;TSharedPtr<FJsonObject> Data;
  if(!FFileHelper::LoadFileToString(Raw,*(FPaths::ProjectDir()/TEXT("Tests/Results/2026-09-12-irwin-approach-clearance.json")))||!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Raw),Data)||!Data.IsValid()){Finish(false,TEXT("Missing measured routes"));return;}
  for(auto Row:Data->GetArrayField(TEXT("routes"))){auto Samples=Row->AsObject()->GetArrayField(TEXT("samples"));auto Point=[](const TSharedPtr<FJsonValue>& V){auto A=V->AsObject()->GetArrayField(TEXT("xyz"));return FVector(A[0]->AsNumber(),A[1]->AsNumber(),A[2]->AsNumber());};S.Routes.Add(TPair<FVector,FVector>(Point(Samples.Last()),Point(Samples[0])));}
  if(S.Routes.Num()!=7){Finish(false,TEXT("Expected seven approaches"));return;}
  auto* Bike=Cast<ABattleBike>(PC->GetPawn());if(!Bike||!Bike->Dismount()){Finish(false,TEXT("Dismount failed"));return;}S.Rider=Cast<ABattleRider>(PC->GetPawn());if(!S.Rider.IsValid()){Finish(false,TEXT("Missing Ellison pawn"));return;}
  if(auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(PC)))if(Mode->Enemies)Mode->Enemies->bFreezeSpawns=true;
  for(TActorIterator<ABattleZombie> I(PC->GetWorld());I;++I)I->Destroy();for(TActorIterator<APiedmontPedestrian> I(PC->GetWorld());I;++I)I->Destroy();
  S.Started=true;
 }
 auto* P=S.Rider.Get();if(!P||PC->GetPawn()!=P){Finish(false,TEXT("Lost rider possession"));return;}
 auto* Move=P->GetCharacterMovement();
 if(S.Settle==0){
  // The dismount's step-off animation repositions the rider every tick while it
  // runs (BattleRider::Tick), which snaps the pawn back to the bike and made this
  // audit stall at the spawn with completed_legs 0 on both platforms. Wait it out
  // before placing the rider on the measured route.
  if(P->StepOffRemaining>0){Move->StopMovementImmediately();return;}
  Move->StopMovementImmediately();P->SetActorLocation(S.Routes[S.Route].Key+FVector(0,0,P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()+5),false,nullptr,ETeleportType::TeleportPhysics);Move->SetMovementMode(MOVE_Walking);S.Settle=.001f;S.Clock=0;S.Falling=0;}
 if(S.Settle<.5f){S.Settle+=Dt;return;}
 S.Clock+=Dt;S.Falling=Move->IsFalling()?S.Falling+Dt:0;
 if(S.Falling>.5f){Finish(false,TEXT("Approach lost sustained ground support"));return;}
 const FVector Goal=S.Leg==0?S.Routes[S.Route].Value:S.Routes[S.Route].Key;
 const FVector Delta=Goal-P->GetActorLocation();
 if(Delta.Size2D()<18&&Move->IsMovingOnGround()){
  Move->StopMovementImmediately();++S.Completed;S.Clock=0;
  if(S.Leg==0){S.Leg=1;return;}S.Leg=0;++S.Route;S.Settle=0;
  if(S.Route==S.Routes.Num())Finish(true,TEXT("Ellison walked seven approaches in both directions with ground support"));return;
 }
 if(S.Clock>6){UE_LOG(LogTemp,Display,TEXT("EntranceWalkStuck: p=%s goal=%s"),*P->GetActorLocation().ToString(),*Goal.ToString());Finish(false,TEXT("Walking leg stalled"));return;}
 PC->SetControlRotation(Delta.GetSafeNormal2D().Rotation());P->AddMovementInput(Delta.GetSafeNormal2D(),1.f);
#endif
}
