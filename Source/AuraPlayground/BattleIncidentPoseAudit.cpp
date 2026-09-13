#include "PiedmontPedestrian.h"
#include "Animation/AnimSequence.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
void TickBattleIncidentPoseAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 static int Stage=0;static float Age=0;static TWeakObjectPtr<APiedmontPedestrian> Visitor;
 static FVector Pelvis;static bool Started=false,Held=false,Recovered=false;static float Drift=0;static bool BodyHit=false,EmptyClear=false,BikeSweep=false,BlockedRecovery=false;static TWeakObjectPtr<AActor> Ceiling;static bool Impact=false,Damage=false,TimedStarted=false;
 if(!PC||!PC->GetPawn()||PC->GetWorld()->GetTimeSeconds()<5)return;Age+=Dt;
 auto Spawn=[&](){
  FVector At=PC->GetPawn()->GetActorLocation()+FVector(450,0,0);FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(PC->GetPawn());
  if(!PC->GetWorld()->LineTraceSingleByChannel(Hit,At+FVector(0,0,500),At-FVector(0,0,1000),ECC_WorldStatic,Q))return (APiedmontPedestrian*)nullptr;
  FTransform T(FRotator::ZeroRotator,Hit.ImpactPoint+FVector(0,0,100));
  auto* P=PC->GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),T,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
  P->CityAppearanceVariant=0;UGameplayStatics::FinishSpawningActor(P,T);P->PauseRemaining=30;return P;
 };
 auto* Clip=LoadObject<UAnimSequence>(nullptr,TEXT("/Game/BattleRetarget/City/Male/M_ragdoll_getup_stand_B"));
 if(Stage==0){Visitor=Spawn();Stage=1;Age=0;return;}
 auto* P=Visitor.Get();if(!P||!Clip){UE_LOG(LogTemp,Error,TEXT("BattleIncidentPoseAudit: {\"passed\":false,\"spawn_or_clip\":false}"));PC->ConsoleCommand(TEXT("quit"));return;}
 if(Stage==1&&Age>1){Started=P->BeginIncidentPose(Clip,Clip->GetPlayLength()*.08f,20);Pelvis=P->Body->GetBoneLocation(TEXT("pelvis"));Stage=2;Age=0;return;}
 if(Stage==2&&Age>.6f){Pelvis=P->Body->GetBoneLocation(TEXT("pelvis"));Stage=20;Age=0;return;}
 if(Stage==20&&Age>1){Drift=FVector::Dist(Pelvis,P->Body->GetBoneLocation(TEXT("pelvis")));Held=P->bIncidentPosing&&Drift<1.f;
  const FVector Side=P->GetActorRightVector()*100;FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(IncidentPoseHit),true);Q.AddIgnoredActor(PC->GetPawn());
  const FVector Head=P->Body->GetBoneLocation(TEXT("head"));
  BodyHit=PC->GetWorld()->LineTraceSingleByChannel(Hit,Head-Side,Head+Side,ECC_Visibility,Q)&&Hit.GetActor()==P;
  const FVector Empty=P->GetActorLocation()+FVector(0,0,65);
  EmptyClear=(!PC->GetWorld()->LineTraceSingleByChannel(Hit,Empty-Side,Empty+Side,ECC_Visibility,Q)||Hit.GetActor()!=P)&&P->GetCapsuleComponent()->GetCollisionEnabled()==ECollisionEnabled::NoCollision;
  Q.bTraceComplex=false;
  BikeSweep=PC->GetWorld()->SweepSingleByObjectType(Hit,Pelvis-Side,Pelvis+Side,FQuat::Identity,FCollisionObjectQueryParams(ECC_Pawn),FCollisionShape::MakeSphere(20),Q)&&Hit.GetActor()==P;
  auto* C=PC->GetWorld()->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(C);C->SetRootComponent(Box);C->AddInstanceComponent(Box);Box->SetBoxExtent(FVector(70,70,10));Box->SetCollisionProfileName(TEXT("BlockAll"));Box->RegisterComponent();C->SetActorLocation(Empty);Ceiling=C;
  P->HearHorn(PC->GetPawn());Stage=25;Age=0;return;}
 if(Stage==25&&Age>.5f){BlockedRecovery=P->bIncidentPosing&&FVector::Dist(Pelvis,P->Body->GetBoneLocation(TEXT("pelvis")))<1.f;if(Ceiling.IsValid())Ceiling->Destroy();Stage=3;Age=0;return;}
 if(Stage==3&&Age>Clip->GetPlayLength()+2){
  Recovered=!P->bIncidentPosing&&!P->bDead;
  const bool Again=P->BeginIncidentPose(Clip,Clip->GetPlayLength()*.3f,20);P->BikeImpact(600,FVector::ForwardVector);
  Impact=Again&&!P->bIncidentPosing&&P->KnockdownPhase>0;
  auto* D=Spawn();const bool DamageStart=D&&D->BeginIncidentPose(Clip,Clip->GetPlayLength()*.08f,20);
  if(D){FDamageEvent E;D->TakeDamage(10,E,PC,PC->GetPawn());}
  Damage=DamageStart&&D->bDead&&!D->bIncidentPosing;
  P->Destroy();if(D)D->Destroy();Visitor=Spawn();
  TimedStarted=Visitor.IsValid()&&Visitor->BeginIncidentPose(Clip,Clip->GetPlayLength()*.3f,.25f);Stage=4;Age=0;return;
 }
 if(Stage==4&&Age>Clip->GetPlayLength()+2){
  const bool Timed=TimedStarted&&!P->bIncidentPosing&&!P->bDead;
  UE_LOG(LogTemp,Display,TEXT("BattleIncidentPoseAudit: {\"passed\":%s,\"started\":%s,\"held\":%s,\"pelvis_drift_cm\":%.3f,\"horn_recovered\":%s,\"bike_interrupt\":%s,\"damage_interrupt\":%s,\"timed_recovered\":%s,\"posed_head_hit\":%s,\"standing_space_clear\":%s,\"posed_bike_sweep\":%s,\"blocked_getup_waited\":%s}"),Started&&Held&&Recovered&&Impact&&Damage&&Timed&&BodyHit&&EmptyClear&&BikeSweep&&BlockedRecovery?TEXT("true"):TEXT("false"),Started?TEXT("true"):TEXT("false"),Held?TEXT("true"):TEXT("false"),Drift,Recovered?TEXT("true"):TEXT("false"),Impact?TEXT("true"):TEXT("false"),Damage?TEXT("true"):TEXT("false"),Timed?TEXT("true"):TEXT("false"),BodyHit?TEXT("true"):TEXT("false"),EmptyClear?TEXT("true"):TEXT("false"),BikeSweep?TEXT("true"):TEXT("false"),BlockedRecovery?TEXT("true"):TEXT("false"));
  Stage=5;PC->ConsoleCommand(TEXT("quit"));
 }
#endif
}
