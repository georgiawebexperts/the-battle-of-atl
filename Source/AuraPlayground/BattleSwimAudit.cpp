#include "BattleSwimAudit.h"
#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleZombie.h"
#include "Components/PoseableMeshComponent.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
#include "Engine/OverlapResult.h"

void TickBattleSwimAudit(ABattleMacController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 auto* W=PC->GetWorld();if(W->GetTimeSeconds()<5)return;
 struct FState{int Stage=0;float Clock=0,SwimDistance=0,Drift=0,MinHand=MAX_flt,MaxHand=-MAX_flt;TWeakObjectPtr<ABattleBike> Bike;FVector Bank,Start;bool Shot=false;};static FState S;
 if(S.Stage<0)return;S.Clock+=Dt;
 auto Key=[&](FKey K,bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.:0.,false,0));};
 auto End=[&](bool Pass,const TCHAR* Reason){Key(EKeys::W,false);UE_LOG(LogTemp,Display,TEXT("BattleSwimAudit: {\"passed\":%s,\"reason\":\"%s\",\"stage\":%d,\"swim_cm\":%.2f,\"bike_drift_cm\":%.3f,\"stroke_cm\":%.2f}"),Pass?TEXT("true"):TEXT("false"),Reason,S.Stage,S.SwimDistance,S.Drift,S.MinHand<MAX_flt?S.MaxHand-S.MinHand:0);S.Stage=-1;PC->ConsoleCommand(TEXT("quit"));};
 auto Capture=[&](const TCHAR* Name){FString Dir;if(FParse::Value(FCommandLine::Get(),TEXT("BattleSwimReviewDir="),Dir))FScreenshotRequest::RequestScreenshot(Dir/(FString(Name)+TEXT(".png")),false,false);};
#define SWIM_CHECK(C,R) if(!(C)){End(false,TEXT(R));return;}
 if(S.Clock>35){End(false,TEXT("Timed out reaching shore or swimming"));return;}
 if(S.Stage==0){
  auto* B=Cast<ABattleBike>(PC->GetPawn());SWIM_CHECK(B,"Missing bike");S.Bike=B;
  if(auto* Mode=Cast<ABattleParkMode>(W->GetAuthGameMode());Mode&&Mode->Enemies)Mode->Enemies->bFreezeSpawns=true;
  FCollisionQueryParams Q(SCENE_QUERY_STAT(BattleSwimBank),true,B);
  for(TActorIterator<AActor> It(W);It;++It)if(It->ActorHasTag(TEXT("RideWater")))Q.AddIgnoredActor(*It);
  bool Found=false;FVector Dry,Wet;
  for(TActorIterator<APiedmontWaterHazard> It(W);It&&!Found;++It){auto* Lake=*It;
   for(int I=0;I<Lake->Polygon.Num()&&!Found;I++){
    const FVector A=Lake->GetActorTransform().TransformPosition(Lake->Polygon[I]),BEdge=Lake->GetActorTransform().TransformPosition(Lake->Polygon[(I+1)%Lake->Polygon.Num()]);
    const FVector Edge=(A+BEdge)*.5,Along=(BEdge-A).GetSafeNormal2D(),Normal(-Along.Y,Along.X,0);
    for(float Sign:{1.f,-1.f}){
     Dry=Edge-Normal*Sign*250;Wet=Edge+Normal*Sign*250;Dry.Z=Wet.Z=Lake->GetActorLocation().Z+98;
     if(Lake->ContainsBike(Dry)||!Lake->ContainsBike(Wet))continue;
     FHitResult Ground;if(!W->LineTraceSingleByChannel(Ground,Dry+FVector(0,0,1200),Dry-FVector(0,0,1200),ECC_Visibility,Q)||Ground.ImpactNormal.Z<.8f)continue;
     if(FMath::Abs(Ground.ImpactPoint.Z-Lake->GetActorLocation().Z)>90)continue;
     Dry=Ground.ImpactPoint+FVector(0,0,98);
     if(W->OverlapBlockingTestByChannel(Dry,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,95),Q))continue;
     Found=true;break;
    }
   }
  }
  SWIM_CHECK(Found,"No accessible real shoreline fixture");
  const FRotator Heading=(Wet-Dry).Rotation();B->SetActorLocationAndRotation(Dry,FRotator(0,Heading.Yaw,0),false,nullptr,ETeleportType::TeleportPhysics);PC->SetControlRotation(FRotator(0,Heading.Yaw,0));
  B->Ride->StopMovementImmediately();B->Ride->Speed=0;B->Ride->bForceNextFloorCheck=true;
  UE_LOG(LogTemp,Display,TEXT("BattleSwimFixture: dry=%s wet=%s"),*Dry.ToString(),*Wet.ToString());
  Key(EKeys::W,true);S.Stage=1;S.Clock=0;return;
 }
 auto* P=Cast<ABattleRider>(PC->GetPawn());
 if(S.Stage==1){if(!P)return;SWIM_CHECK(S.Bike.IsValid()&&S.Bike->bParked,"Lake did not park bike");S.Bank=S.Bike->GetActorLocation();S.Start=P->GetActorLocation();S.Stage=2;S.Clock=0;return;}
 SWIM_CHECK(P&&S.Bike.IsValid(),"Lost swimmer or parked bike");
 S.Drift=FMath::Max(S.Drift,float(FVector::Distance(S.Bank,S.Bike->GetActorLocation())));
 SWIM_CHECK(S.Drift<1,"Parked bike moved during swimming");
 if(S.Stage==2){
  const FName Hand=FParse::Param(FCommandLine::Get(),TEXT("BattleDetailedRider"))?FName(TEXT("hand_l")):FName(TEXT("Hand_L"));
  const float Z=P->Body->GetBoneLocationByName(Hand,EBoneSpaces::ComponentSpace).Z;S.MinHand=FMath::Min(S.MinHand,Z);S.MaxHand=FMath::Max(S.MaxHand,Z);
  if(S.Clock>1&&!S.Shot){Capture(TEXT("swimming"));S.Shot=true;}
  if(S.Clock<2.5)return;
  S.SwimDistance=FVector::Dist2D(S.Start,P->GetActorLocation());
  SWIM_CHECK(P->bSwimming&&S.SwimDistance>250,"Did not swim away from shore");SWIM_CHECK(!P->ToggleDrawWeapon()&&!P->Fire()&&!P->MountBike()&&!P->FirstPersonArms->IsVisible(),"Swimming allowed gun or remount");
  TInlineComponentArray<UPoseableMeshComponent*> Parts(P);for(auto* Part:Parts)if(Part->GetFName()==TEXT("DetailedM1911"))SWIM_CHECK(!Part->IsVisible(),"Pistol visible in water");
  SWIM_CHECK(S.MaxHand-S.MinHand>15,"Swimming arms did not stroke");
  Key(EKeys::W,false);S.Stage=3;S.Clock=0;
 }
 if(S.Stage==3){
  const FVector Delta=S.Bank-P->GetActorLocation();PC->SetControlRotation(FRotator(0,Delta.Rotation().Yaw,0));Key(EKeys::W,true);
  if(!P->bSwimming&&Delta.Size2D()<210){Key(EKeys::W,false);Capture(TEXT("returned"));if(!P->MountBike()){UE_LOG(LogTemp,Display,TEXT("SwimRemountDebug: distance=%.1f stun=%.1f crash=%d health=%.1f swimmer=%s"),FVector::Distance(S.Bank,P->GetActorLocation()),S.Bike->StunRemaining,S.Bike->bCrashActive,P->Health,*P->GetActorLocation().ToString());TArray<FOverlapResult> Hits;FCollisionQueryParams Q;Q.AddIgnoredActor(P);Q.AddIgnoredActor(S.Bike.Get());W->OverlapMultiByChannel(Hits,S.Bank,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,95),Q);for(const auto& Hit:Hits)UE_LOG(LogTemp,Display,TEXT("SwimRemountOverlap: %s blocking=%d"),*FString::Printf(TEXT("%s label=%s component=%s water=%d"),*GetNameSafe(Hit.GetActor()),Hit.GetActor()?*Hit.GetActor()->GetActorNameOrLabel():TEXT("none"),*GetNameSafe(Hit.GetComponent()),Hit.GetActor()&&Hit.GetActor()->ActorHasTag(TEXT("RideWater"))),Hit.bBlockingHit);End(false,TEXT("Cannot remount at bank"));return;}End(true,TEXT("Real shoreline entry, swim away, return and fixed-bike remount pass"));}
 }
#undef SWIM_CHECK
#endif
}
