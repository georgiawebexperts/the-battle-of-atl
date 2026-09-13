#include "BattleBike.h"
#include "PiedmontDarkZone.h"
#include "Components/BoxComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
void TickBattleLightBoundaryAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<APiedmontDarkZone> Zone;int Stage=0;float Age=0;bool BodyLit=false,HeadLit=false,Held=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}
 if(S.Stage==99||PC->GetWorld()->GetTimeSeconds()<5)return;
 auto* B=Cast<ABattleBike>(PC->GetPawn());if(!B)return;S.Age+=Dt;
 auto Finish=[&](bool Pass){UE_LOG(LogTemp,Display,TEXT("LightBoundaryAudit: {\"passed\":%s,\"body_only_lit\":%s,\"head_only_lit\":%s,\"exit_delay\":%s,\"daylight_off\":%s}"),Pass?TEXT("true"):TEXT("false"),S.BodyLit?TEXT("true"):TEXT("false"),S.HeadLit?TEXT("true"):TEXT("false"),S.Held?TEXT("true"):TEXT("false"),!B->bLightsOn?TEXT("true"):TEXT("false"));S.Stage=99;PC->ConsoleCommand(TEXT("quit"));};
 if(S.Stage==0){
  for(TActorIterator<APiedmontDarkZone> It(PC->GetWorld());It;++It)It->Destroy();
  for(TActorIterator<ADirectionalLight> It(PC->GetWorld());It;++It)It->SetActorRotation(FRotator(-45,0,0));
  B->Ride->StopMovementImmediately();B->Ride->DisableMovement();B->SetActorLocationAndRotation(FVector(0,0,30000),FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);
  S.Zone=PC->GetWorld()->SpawnActor<APiedmontDarkZone>(B->GetActorLocation(),FRotator::ZeroRotator);S.Zone->Bounds->SetBoxExtent(FVector(20));S.Stage=1;S.Age=0;return;
 }
 auto Lit=[&](){return B->bLightsOn&&B->Headlight->IsVisible()&&B->TailLight->IsVisible();};
 if(S.Stage==1&&S.Age>.6f){S.BodyLit=S.Zone->Contains(B->GetActorLocation())&&!S.Zone->Contains(B->Headlight->GetComponentLocation())&&Lit();S.Zone->SetActorLocation(B->Headlight->GetComponentLocation());S.Stage=2;S.Age=0;return;}
 if(S.Stage==2&&S.Age>2.f){S.HeadLit=!S.Zone->Contains(B->GetActorLocation())&&S.Zone->Contains(B->Headlight->GetComponentLocation())&&Lit();S.Zone->SetActorLocation(FVector(0,0,35000));S.Stage=3;S.Age=0;return;}
 if(S.Stage==3&&S.Age>.5f){S.Held=Lit();S.Stage=4;return;}
 if(S.Stage==4&&S.Age>2.f)Finish(S.BodyLit&&S.HeadLit&&S.Held&&!B->bLightsOn&&!B->Headlight->IsVisible()&&!B->TailLight->IsVisible());
#endif
}
