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
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

void TickBattleSwimAudit(ABattleMacController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 auto* W=PC->GetWorld();if(W->GetTimeSeconds()<5)return;
 struct FState{int Stage=0,Edge=0;float EntryBankDistance=0,NextIdleLog=0,Clock=0,SwimDistance=0,Drift=0,MinHand=MAX_flt,MaxHand=-MAX_flt;TWeakObjectPtr<ABattleBike> Bike;FVector Bank,Start,RouteLast;TArray<FVector> Route;TArray<bool> RouteSwim;int RouteIndex=0;float RouteDistance=0;bool TaserPassed=false,AwayShore=false,Shot=false;};static FState S;
 if(S.Stage<0)return;S.Clock+=Dt;
 auto Key=[&](FKey K,bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.:0.,false,0));};
 auto End=[&](bool Pass,const TCHAR* Reason){Key(EKeys::W,false);
  if(!Pass&&S.Stage==7&&PC->GetPawn()){
   APawn* Pawn=PC->GetPawn();const FVector Here=Pawn->GetActorLocation();FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(SwimRouteBlock),false,Pawn);Q.AddIgnoredActor(S.Bike.Get());for(TActorIterator<AActor> It(W);It;++It)if(It->ActorHasTag(TEXT("RideWater")))Q.AddIgnoredActor(*It);
   const FVector Ahead=Here+(S.Route[S.RouteIndex]-Here).GetSafeNormal2D()*300;
   W->SweepSingleByChannel(Hit,Here,Ahead,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,88),Q);
   UE_LOG(LogTemp,Display,TEXT("SwimRouteBlock: here=%s velocity=%s target=%s actor=%s component=%s impact=%s normal=%s"),*Here.ToString(),*Pawn->GetVelocity().ToString(),*S.Route[S.RouteIndex].ToString(),Hit.GetActor()?*Hit.GetActor()->GetActorNameOrLabel():TEXT("none"),*GetNameSafe(Hit.GetComponent()),*Hit.ImpactPoint.ToString(),*Hit.ImpactNormal.ToString());
   FString Dir;if(FParse::Value(FCommandLine::Get(),TEXT("BattleSwimReviewDir="),Dir))FScreenshotRequest::RequestScreenshot(Dir/TEXT("route-blocked.png"),false,false);
  }UE_LOG(LogTemp,Display,TEXT("BattleSwimAudit: {\"passed\":%s,\"reason\":\"%s\",\"stage\":%d,\"swim_cm\":%.2f,\"bike_drift_cm\":%.3f,\"stroke_cm\":%.2f,\"route_index\":%d,\"route_points\":%d,\"route_distance_cm\":%.2f,\"alternate_shore\":%s,\"taser_recovery\":%s,\"entry_bank_distance_cm\":%.2f}"),Pass?TEXT("true"):TEXT("false"),Reason,S.Stage,S.SwimDistance,S.Drift,S.MinHand<MAX_flt?S.MaxHand-S.MinHand:0,S.RouteIndex,S.Route.Num(),S.RouteDistance,S.AwayShore?TEXT("true"):TEXT("false"),S.TaserPassed?TEXT("true"):TEXT("false"),S.EntryBankDistance);S.Stage=-1;PC->ConsoleCommand(TEXT("quit"));};
 auto Capture=[&](const TCHAR* Name){FString Dir;if(FParse::Value(FCommandLine::Get(),TEXT("BattleSwimReviewDir="),Dir))FScreenshotRequest::RequestScreenshot(Dir/(FString(Name)+TEXT(".png")),false,false);};
#define SWIM_CHECK(C,R) if(!(C)){End(false,TEXT(R));return;}
 if(S.Clock>35){End(false,TEXT("Timed out reaching shore or swimming"));return;}
 if(S.Stage==0){
  auto* B=Cast<ABattleBike>(PC->GetPawn());SWIM_CHECK(B,"Missing bike");S.Bike=B;
  if(auto* Mode=Cast<ABattleParkMode>(W->GetAuthGameMode());Mode&&Mode->Enemies)Mode->Enemies->bFreezeSpawns=true;
  FCollisionQueryParams Q(SCENE_QUERY_STAT(BattleSwimBank),true,B);
  for(TActorIterator<AActor> It(W);It;++It)if(It->ActorHasTag(TEXT("RideWater")))Q.AddIgnoredActor(*It);
  bool Found=false;FVector Dry,Wet;int32 Offset=0;FParse::Value(FCommandLine::Get(),TEXT("BattleSwimEdge="),Offset);
  for(TActorIterator<APiedmontWaterHazard> It(W);It&&!Found;++It){auto* Lake=*It;
   for(int I=0;I<Lake->Polygon.Num()&&!Found;I++){
    const int Index=(I+FMath::Max(0,Offset))%Lake->Polygon.Num();
    const FVector A=Lake->GetActorTransform().TransformPosition(Lake->Polygon[Index]),BEdge=Lake->GetActorTransform().TransformPosition(Lake->Polygon[(Index+1)%Lake->Polygon.Num()]);
    const FVector Edge=(A+BEdge)*.5,Along=(BEdge-A).GetSafeNormal2D(),Normal(-Along.Y,Along.X,0);
    for(float Sign:{1.f,-1.f}){
     Dry=Edge-Normal*Sign*250;Wet=Edge+Normal*Sign*250;Dry.Z=Wet.Z=Lake->GetActorLocation().Z+98;
     if(Lake->ContainsBike(Dry)||!Lake->ContainsBike(Wet))continue;
     FHitResult Ground;if(!W->LineTraceSingleByChannel(Ground,Dry+FVector(0,0,1200),Dry-FVector(0,0,1200),ECC_Visibility,Q)||Ground.ImpactNormal.Z<.8f)continue;
     if(FMath::Abs(Ground.ImpactPoint.Z-Lake->GetActorLocation().Z)>90)continue;
     Dry=Ground.ImpactPoint+FVector(0,0,98);
     if(W->OverlapBlockingTestByChannel(Dry,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,95),Q))continue;
     S.Edge=Index;Found=true;break;
    }
   }
  }
  SWIM_CHECK(Found,"No accessible real shoreline fixture");
  const FRotator Heading=(Wet-Dry).Rotation();B->SetActorLocationAndRotation(Dry,FRotator(0,Heading.Yaw,0),false,nullptr,ETeleportType::TeleportPhysics);PC->SetControlRotation(FRotator(0,Heading.Yaw,0));
  B->Ride->StopMovementImmediately();B->Ride->Speed=0;B->Ride->bForceNextFloorCheck=true;
  UE_LOG(LogTemp,Display,TEXT("BattleSwimFixture: edge=%d dry=%s wet=%s"),S.Edge,*Dry.ToString(),*Wet.ToString());
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleSwimDeepDrop"))){
   bool DropFound=false;FVector Drop;
   for(TActorIterator<APiedmontWaterHazard> It(W);It&&!DropFound;++It)for(float Distance:{1400.f,2200.f,3000.f}){Drop=Wet+(Wet-Dry).GetSafeNormal2D()*Distance;Drop.Z=It->GetActorLocation().Z+35;
    if(!It->ContainsBike(Drop)||W->OverlapBlockingTestByChannel(Drop,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,35),Q))continue;
    DropFound=true;Drop.Z+=700;break;
   }
   SWIM_CHECK(DropFound,"No clear deep-water drop fixture");B->SetActorLocation(Drop,false,nullptr,ETeleportType::TeleportPhysics);B->Ride->bHasDryLocation=false;B->Ride->SetMovementMode(MOVE_Falling);Key(EKeys::W,false);
   UE_LOG(LogTemp,Display,TEXT("SwimDeepDrop: start=%s"),*Drop.ToString());
  }else Key(EKeys::W,true);
  S.Stage=1;S.Clock=0;return;
 }
 if(S.Stage==4){if(S.Clock>=S.NextIdleLog){S.NextIdleLog+=.25f;UE_LOG(LogTemp,Display,TEXT("SwimIdle: t=%.2f dt=%.3f w=%d up=%d pedal=%.1f speed=%.1f velocity=%s location=%s parked=%d"),S.Clock,Dt,PC->IsInputKeyDown(EKeys::W),PC->IsInputKeyDown(EKeys::Up),S.Bike->Ride->Pedal,S.Bike->Ride->Speed,*S.Bike->Ride->Velocity.ToString(),*S.Bike->GetActorLocation().ToString(),S.Bike->bParked);}SWIM_CHECK(PC->GetPawn()==S.Bike.Get()&&!S.Bike->bParked&&S.Bike->Ride->Recovery<=0,"Remount did not remain stable");SWIM_CHECK(S.Bike->Ride->Speed<5,"Bike accelerated after forward key release");if(S.Clock>2)End(true,TEXT("Shoreline entry, swim, fixed-bike return and stable remount pass"));return;}
 if(S.Stage==6){if(S.Clock<.2f)return;Key(EKeys::E,false);SWIM_CHECK(PC->GetPawn()==S.Bike.Get(),"E failed to remount at bank");Capture(TEXT("returned"));S.Stage=4;S.Clock=0;return;}
 auto* P=Cast<ABattleRider>(PC->GetPawn());
 if(S.Stage==1){if(!P)return;SWIM_CHECK(S.Bike.IsValid()&&S.Bike->bParked,"Lake did not park bike");S.Bank=S.Bike->GetActorLocation();S.Start=P->GetActorLocation();S.EntryBankDistance=FVector::Dist2D(S.Bank,S.Start);if(FParse::Param(FCommandLine::Get(),TEXT("BattleSwimDeepDrop"))){SWIM_CHECK(S.EntryBankDistance>600,"Deep drop fixture was too close to bank");}Key(EKeys::W,true);S.Stage=2;S.Clock=0;return;}
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
  Key(EKeys::W,true);S.Stage=3;S.Clock=0;
  FString RoutePath;if(FParse::Value(FCommandLine::Get(),TEXT("BattleSwimRoute="),RoutePath)){
   FString Json;TSharedPtr<FJsonObject> Root;SWIM_CHECK(FFileHelper::LoadFileToString(Json,*RoutePath)&&FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json),Root),"Cannot load exploration route");
   const TArray<TSharedPtr<FJsonValue>>* Points=nullptr;SWIM_CHECK(Root->TryGetArrayField(TEXT("points"),Points),"Missing exploration points");
   for(const auto& Value:*Points){const auto Point=Value->AsObject();SWIM_CHECK(Point.IsValid(),"Invalid route point");S.Route.Add(FVector(Point->GetNumberField(TEXT("x")),Point->GetNumberField(TEXT("y")),0));S.RouteSwim.Add(Point->GetBoolField(TEXT("swim")));}
   SWIM_CHECK(!S.Route.IsEmpty(),"Empty exploration route");S.RouteLast=P->GetActorLocation();S.Stage=7;
  }
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleSwimTaser"))){
   auto* Mode=Cast<ABattleLabMode>(W->GetAuthGameMode());SWIM_CHECK(Mode,"Missing timer mode");const float Before=Mode->TimeRemaining;
   SWIM_CHECK(S.Bike->ApplyTaser(),"Taser application rejected");SWIM_CHECK(FMath::Abs((Before-Mode->TimeRemaining)-10)<.1f,"Taser time penalty incorrect");
   S.Start=P->GetActorLocation();S.Shot=false;S.Stage=8;S.Clock=0;
  }
 }
 if(S.Stage==8){
  if(S.Bike->StunRemaining>0){SWIM_CHECK(P->GetCharacterMovement()->MovementMode==MOVE_None&&P->GetVelocity().Size()<1,"Tased swimmer still moving");SWIM_CHECK(!P->ToggleDrawWeapon()&&!P->MountBike(),"Tased swimmer allowed interaction");if(S.Clock>.5f&&!S.Shot){Capture(TEXT("tased"));S.Shot=true;}return;}
  SWIM_CHECK(P->bSwimming&&P->GetCharacterMovement()->MovementMode==MOVE_Flying,"Swim mode not restored after taser");
  if(S.Clock<4.2f)return;SWIM_CHECK(FVector::Dist2D(S.Start,P->GetActorLocation())>100,"Swimmer did not move after recovery");S.TaserPassed=true;S.Stage=3;S.Clock=0;
 }
 if(S.Stage==7){
  S.RouteDistance+=FVector::Dist2D(S.RouteLast,P->GetActorLocation());S.RouteLast=P->GetActorLocation();
  const FVector Delta=S.Route[S.RouteIndex]-P->GetActorLocation();PC->SetControlRotation(FRotator(0,Delta.Rotation().Yaw,0));
  if(Delta.Size2D()<80){
   SWIM_CHECK(P->bSwimming==S.RouteSwim[S.RouteIndex],"Route surface state does not match lake or shore");
   if(!P->bSwimming&&FVector::Dist2D(S.Bank,P->GetActorLocation())>1000)S.AwayShore=true;
   S.RouteIndex++;S.Clock=0;
   if(S.RouteIndex%25==0)UE_LOG(LogTemp,Display,TEXT("SwimExploration: point=%d/%d distance=%.1f"),S.RouteIndex,S.Route.Num(),S.RouteDistance);
   if(S.RouteIndex==S.Route.Num()){if(S.RouteSwim.Contains(false)){SWIM_CHECK(S.AwayShore,"Did not exit at a different shore");}S.Stage=3;}
  }
 }
 if(S.Stage==3){
  const FVector Delta=S.Bank-P->GetActorLocation();PC->SetControlRotation(FRotator(0,Delta.Rotation().Yaw,0));
  if(!P->bSwimming&&Delta.Size2D()<210){Key(EKeys::W,false);S.Stage=5;S.Clock=0;}
 }
 if(S.Stage==5&&S.Clock>.2f){SWIM_CHECK(!PC->IsInputKeyDown(EKeys::W),"Forward release not processed");Key(EKeys::E,true);S.Stage=6;S.Clock=0;}
#undef SWIM_CHECK
#endif
}
