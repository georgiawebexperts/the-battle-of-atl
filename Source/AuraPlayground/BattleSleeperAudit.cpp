#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void TickBattleSleeperAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<APiedmontPedestrian> Person;int32 Phase=0;float Clock=0,Total=0;bool Done=false;};
 static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}
 if(S.Done||PC->GetWorld()->GetTimeSeconds()<5||!PC->GetPawn())return;
 S.Clock+=Dt;S.Total+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){S.Done=true;UE_LOG(LogTemp,Display,TEXT("BattleSleeperAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),S.Phase,Reason);UKismetSystemLibrary::QuitGame(PC,PC,EQuitPreference::Quit,false);};
#define CHECK_SLEEP(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 CHECK_SLEEP(S.Total<25,"Timed out");
 if(S.Phase==0){
  for(TActorIterator<APiedmontTrafficDirector> It(PC->GetWorld());It;++It)It->SetActorTickEnabled(false);
  FCollisionQueryParams Q(SCENE_QUERY_STAT(SleeperAuditFloor),false,PC->GetPawn());
  FVector SpawnPoint=FVector::ZeroVector;bool Found=false;
  for(int32 Attempt=0;Attempt<24&&!Found;++Attempt){
   const float Angle=(Attempt%8)*PI/4.f,Radius=350.f+(Attempt/8)*450.f;
   const FVector P=PC->GetPawn()->GetActorLocation()+FVector(FMath::Cos(Angle)*Radius,FMath::Sin(Angle)*Radius,0);
   FHitResult Hit;
   if(!PC->GetWorld()->LineTraceSingleByChannel(Hit,P+FVector(0,0,300),P-FVector(0,0,1000),ECC_Visibility,Q)||Hit.ImpactNormal.Z<.85f)continue;
   const FVector Candidate=Hit.ImpactPoint+FVector(0,0,90);
   if(!PC->GetWorld()->OverlapBlockingTestByChannel(Candidate,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,88),Q)){SpawnPoint=Candidate;Found=true;}
  }
  CHECK_SLEEP(Found,"No clear standing test placement");
  const FTransform T(FRotator::ZeroRotator,SpawnPoint);
  auto* Person=PC->GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),T,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  CHECK_SLEEP(Person,"Spawn failed");Person->CityAppearanceVariant=0;UGameplayStatics::FinishSpawningActor(Person,T);Person->PauseRemaining=100;S.Person=Person;S.Phase=1;S.Clock=0;return;
 }
 auto* Person=S.Person.Get();CHECK_SLEEP(Person,"Test character disappeared");
 if(S.Phase==1&&S.Clock>.5f){CHECK_SLEEP(Person->BeginSleeping(),"Sleep entry failed");S.Phase=2;S.Clock=0;return;}
 if(S.Phase==2&&S.Clock>1){
  Person->Body->RefreshBoneTransforms();
  const float Floor=Person->GetActorLocation().Z-Person->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
  UE_LOG(LogTemp,Display,TEXT("SleeperPose: head_height=%.3f root=%s"),Person->Body->GetSocketLocation(TEXT("head")).Z-Floor,*Person->Body->BoneSpaceTransforms[0].ToString());
  UE_LOG(LogTemp,Display,TEXT("SleeperPoseFrame: actor=%s body=%s pelvis_local=%s"),*Person->GetActorLocation().ToString(),*Person->Body->GetRelativeLocation().ToString(),*Person->Body->BoneSpaceTransforms[1].ToString());
  CHECK_SLEEP(Person->SleepPhase==1&&Person->GetVelocity().Size()<1,"Sleeper moved or exited");
  CHECK_SLEEP(Person->Body->GetSocketLocation(TEXT("head")).Z-Floor<60,"Sleeping head remains standing");
  TArray<FOverlapResult> Overlaps;FCollisionQueryParams OverlapQ(SCENE_QUERY_STAT(SleeperAuditClearance),false,Person);
  PC->GetWorld()->OverlapMultiByChannel(Overlaps,Person->GetActorLocation(),FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,86),OverlapQ);
  for(const auto& O:Overlaps)UE_LOG(LogTemp,Display,TEXT("SleeperOverlap: actor=%s component=%s blocking=%d"),*GetNameSafe(O.GetActor()),*GetNameSafe(O.GetComponent()),O.bBlockingHit);
  auto* Blocker=PC->GetWorld()->SpawnActor<AActor>();
  CHECK_SLEEP(Blocker,"Cannot create clearance fixture");
  auto* Roof=NewObject<UBoxComponent>(Blocker);Blocker->SetRootComponent(Roof);Blocker->AddInstanceComponent(Roof);
  Roof->SetBoxExtent(FVector(80,80,15));Roof->SetCollisionProfileName(TEXT("BlockAll"));Roof->SetCanEverAffectNavigation(false);Roof->RegisterComponent();
  Roof->SetWorldLocation(Person->GetActorLocation()+FVector(0,0,60));
  const bool Rejected=!Person->WakeFromSleep()&&Person->SleepPhase==1;
  Roof->SetCollisionEnabled(ECollisionEnabled::NoCollision);Blocker->Destroy();
  CHECK_SLEEP(Rejected,"Sleeper stood into overhead obstacle");
  CHECK_SLEEP(Person->WakeFromSleep(),"Wake entry failed");S.Phase=3;S.Clock=0;return;
 }
 if(S.Phase==3&&S.Clock>5.7f){
  Person->Body->RefreshBoneTransforms();
  const float Floor=Person->GetActorLocation().Z-Person->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
  CHECK_SLEEP(Person->SleepPhase==0,"Wake did not finish");
  CHECK_SLEEP(Person->Body->GetSocketLocation(TEXT("head")).Z-Floor>130,"Wake did not stand");
  CHECK_SLEEP(Person->BeginSleeping(),"Second sleep failed");S.Phase=4;S.Clock=0;return;
 }
 if(S.Phase==4&&S.Clock>.5f){Person->BikeImpact(100,FVector::ForwardVector);CHECK_SLEEP(Person->SleepPhase==0&&!Person->BeginSleeping(),"Impact failed to interrupt or allowed sleep during stumble");S.Phase=5;S.Clock=0;return;}
 if(S.Phase==5&&S.Clock>3.2f){CHECK_SLEEP(Person->BeginSleeping(),"Sleep after impact failed");FDamageEvent Damage;Person->TakeDamage(10,Damage,PC,PC->GetPawn());CHECK_SLEEP(Person->bDead&&Person->SleepPhase==0&&!Person->WakeFromSleep(),"Damage failed to cancel sleep");Finish(true,TEXT("sleep, blocked wake, clear wake, repeat, impact and death passed"));}
#undef CHECK_SLEEP
#endif
}
