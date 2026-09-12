#include "PiedmontPedestrian.h"
#include "Components/BoxComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void TickBattleSleeperSettleAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<APiedmontPedestrian> Person;TWeakObjectPtr<AActor> Blocker;int32 Phase=0;float Clock=0,Total=0,Jump=0;FVector Start,Head;bool Done=false;};
 static FState S;if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}
 if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;S.Total+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){S.Done=true;UE_LOG(LogTemp,Display,TEXT("BattleSleeperSettleAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\",\"handoff_head_jump_cm\":%.4f}"),Pass?TEXT("true"):TEXT("false"),S.Phase,Reason,S.Jump);UKismetSystemLibrary::QuitGame(PC,PC,EQuitPreference::Quit,false);};
#define CHECK_SETTLE(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Box=[&](FVector P,FVector Extent){auto* Actor=PC->GetWorld()->SpawnActor<AActor>();auto* Shape=NewObject<UBoxComponent>(Actor);Actor->SetRootComponent(Shape);Actor->AddInstanceComponent(Shape);Shape->SetBoxExtent(Extent);Shape->SetCollisionProfileName(TEXT("BlockAll"));Shape->SetCanEverAffectNavigation(false);Shape->RegisterComponent();Shape->SetWorldLocation(P);return Actor;};
 auto Clear=[&](){if(auto* A=S.Blocker.Get()){A->SetActorEnableCollision(false);A->Destroy();}S.Blocker.Reset();};
 CHECK_SETTLE(S.Total<30,"Timed out");
 if(S.Phase==0){
  Box(FVector(0,0,9990),FVector(1000,1000,10));
  const FTransform T(FRotator::ZeroRotator,FVector(0,0,10100));
  auto* P=PC->GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),T,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  CHECK_SETTLE(P,"Spawn failed");P->CityAppearanceVariant=0;UGameplayStatics::FinishSpawningActor(P,T);P->PauseRemaining=100;S.Person=P;S.Phase=1;S.Clock=0;return;
 }
 auto* P=S.Person.Get();CHECK_SETTLE(P&&!P->bDead,"Sleeper lost");
 if(S.Phase==1&&S.Clock>.8f){
  S.Start=P->GetActorLocation();S.Blocker=Box(S.Start+FVector(20,100,0),FVector(40,40,80));
  CHECK_SETTLE(!P->BeginSettling()&&P->SleepPhase==0,"Blocked fall was accepted");Clear();
  CHECK_SETTLE(P->BeginSettling(),"Clear fall rejected");S.Phase=2;S.Clock=0;return;
 }
 if(S.Phase==2&&S.Clock>1){S.Blocker=Box(S.Start+FVector(39.753575,261.387456,0),FVector(40,40,80));S.Phase=3;S.Clock=0;return;}
 if(S.Phase==3&&S.Clock>5){
  CHECK_SETTLE(P->SleepPhase==4&&FVector::Dist(P->GetActorLocation(),S.Start)<1,"Blocked landing teleported or exited");
  P->Body->RefreshBoneTransforms();S.Head=P->Body->GetSocketLocation(TEXT("head"));Clear();S.Phase=4;S.Clock=0;return;
 }
 if(S.Phase==4&&P->SleepPhase==1){
  P->Body->RefreshBoneTransforms();S.Jump=FVector::Dist(S.Head,P->Body->GetSocketLocation(TEXT("head")));
  UE_LOG(LogTemp,Display,TEXT("SettleHandoff: before=%s after=%s actor=%s body=%s"),*S.Head.ToString(),*P->Body->GetSocketLocation(TEXT("head")).ToString(),*P->GetActorLocation().ToString(),*P->Body->GetRelativeLocation().ToString());
  CHECK_SETTLE(S.Jump<1,"Landing handoff jumped visually");
  CHECK_SETTLE(FVector::Dist2D(P->GetActorLocation(),S.Start+FVector(39.753575,261.387456,0))<1,"Wrong landing position");
  CHECK_SETTLE(P->WakeFromSleep(),"Cannot wake at landing");S.Phase=5;S.Clock=0;return;
 }
 if(S.Phase==5&&S.Clock>5.8f){
  P->Body->RefreshBoneTransforms();CHECK_SETTLE(P->SleepPhase==0&&P->Body->GetSocketLocation(TEXT("head")).Z>10130,"Landing wake failed");
  Finish(true,TEXT("blocked start, blocked landing, continuous handoff and re-wake passed"));
 }
#undef CHECK_SETTLE
#endif
}
