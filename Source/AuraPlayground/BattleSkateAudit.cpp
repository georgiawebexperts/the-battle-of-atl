#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleSkatepark.h"
#include "BattlePickup.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "Engine/GameViewportClient.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Kismet/GameplayStatics.h"
#include "InputKeyEventArgs.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
void ABattleMacController::TickSkateAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5||SkateStage==99)return;
 auto* Bike=Cast<ABattleBike>(GetPawn());auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));ABattleSkatepark* Park=nullptr;if(TActorIterator<ABattleSkatepark> It(GetWorld());It)Park=*It;if(!Bike||!Mode)return;SkateClock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BattleSkateAudit: {\"passed\":%s,\"stage\":%d,\"reason\":\"%s\",\"air_rewards\":%d,\"air_peak_cm\":%.2f,\"x\":%.2f,\"z\":%.2f}"),Pass?TEXT("true"):TEXT("false"),SkateStage,Reason,Bike->Ride->AirRewards,Bike->Ride->AirPeak,Bike->GetActorLocation().X,Bike->GetActorLocation().Z);SkateStage=99;ConsoleCommand(TEXT("quit"));};
#define SCHECK(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Capture=[&](const TCHAR* Name){FString Dir;if(FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Dir))FScreenshotRequest::RequestScreenshot(Dir/(FString(Name)+TEXT(".png")),false,false);};
 auto Next=[&](){SkateStage++;SkateClock=0;};
 auto Key=[&](bool Down){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::W,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Place=[&](FVector P,float Yaw,float Speed){Bike->SetActorLocationAndRotation(P,FRotator(0,Yaw,0),false,nullptr,ETeleportType::TeleportPhysics);SetControlRotation(FRotator(0,Yaw,0));Bike->Ride->StopMovementImmediately();Bike->Ride->SetMovementMode(MOVE_Walking);Bike->Ride->bForceNextFloorCheck=true;Bike->Ride->Speed=Speed;Bike->Ride->Gear=4;};
 if(SkateStage==0){
  SCHECK(Park&&Park->Bonuses.Num()==3,"Missing park/bonus layout");
  for(auto Bonus:Park->Bonuses)Bonus->SetActorTickEnabled(false);
  Bike->Ride->bRealHandling=FParse::Param(FCommandLine::Get(),TEXT("BattleSkateReal"));
  UE_LOG(LogTemp,Display,TEXT("BattleSkateHandling: %s"),Bike->Ride->bRealHandling?TEXT("realistic"):TEXT("arcade"));
  // The review camera has to frame the whole park now: 57 x 46 m, five bowls,
  // the quarter pipe and the north hips, not the 36 x 24 m pad it started as.
  if(FParse::Param(FCommandLine::Get(),TEXT("RenderOffscreen"))){const FVector Focus=Park->GetActorLocation()+FVector(-500,-200,300),Eye=Focus+FVector(-2500,-7500,5200);if(auto* C=GetWorld()->SpawnActor<ACameraActor>(Eye,(Focus-Eye).Rotation()))SetViewTarget(C);}
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  FCollisionQueryParams Q(SCENE_QUERY_STAT(SkateSurfaceAudit),false,Bike);
  // Four probes for the park as it was, three for the bigger park: the deep
  // bowl, the transfer bowl and the pyramid cap. If the deck rises or a feature
  // moves, these fail here rather than as a rider falling through the world.
  for(FVector P:{FVector(38050,73650,470),FVector(38050,74400,510),FVector(40100,73500,950),FVector(39000,74000,650),FVector(36400,72500,350),FVector(37100,71950,520),FVector(39500,75750,850)}){FHitResult H;const bool Hit=GetWorld()->LineTraceSingleByChannel(H,P+FVector(0,0,300),P-FVector(0,0,150),ECC_Visibility,Q);UE_LOG(LogTemp,Display,TEXT("SkateSurface: expected=%s hit=%s actor=%s"),*P.ToString(),*H.ImpactPoint.ToString(),*GetNameSafe(H.GetActor()));SCHECK(Hit&&H.GetActor()==Park&&FMath::Abs(H.ImpactPoint.Z-P.Z)<4,"Bowl/ramp surface missing or obstructed");}
  Place(FVector(41600,74168,580),180,700);Key(true);Next();
 }else if(SkateStage==1&&SkateClock>2.2f){
  SCHECK(Bike->GetActorLocation().X<40750&&Bike->Ride->CurrentFloor.HitResult.GetActor()==Park&&Bike->Ride->Wipeouts==0,"Cannot ride from trail into skatepark");
  Capture(TEXT("overview"));Place(FVector(39100,73500,748),0,1100);SkateTime=Mode->TimeRemaining;Next();
 }else if(SkateStage==2&&Bike->Ride->IsFalling()&&Bike->Ride->AirPeak>65){SCHECK(Bike->GetActorLocation().X>40000,"Unexpected takeoff before ramp lip");Capture(TEXT("airborne"));Next();}
 else if(SkateStage==3&&Bike->Ride->IsMovingOnGround()){
  SCHECK(Bike->Ride->AirRewards==1&&!Park->Bonuses[2]->bConsumed&&Mode->TimeRemaining>SkateTime+5&&Bike->Ride->Wipeouts==0,"Ramp did not award clean airtime bonus");Key(false);Place(FVector(38050,73650,568),0,0);SkateTime=Mode->TimeRemaining;Next();
 }else if(SkateStage==4&&SkateClock>.3f){
  SCHECK(Park->Bonuses[0]->TryCollect(Bike)&&Park->Bonuses[0]->bConsumed&&Mode->TimeRemaining>SkateTime+29,"Bowl bonus did not grant thirty seconds");SCHECK(!Park->Bonuses[0]->TryCollect(Bike),"Bowl bonus repeated");SCHECK(Bike->Ride->CurrentFloor.HitResult.GetActor()==Park&&Bike->GetActorLocation().Z>560,"Bike fell through bowl");Place(FVector(39700,74168,748),0,700);Key(true);Next();
 }
 else if(SkateStage==5&&Bike->GetActorLocation().X>41810){Key(false);Bike->Ride->Speed=0;Bike->Ride->StopMovementImmediately();Next();}
 else if(SkateStage==6&&SkateClock>.3f){SCHECK(Bike->Ride->IsMovingOnGround()&&Bike->Ride->Wipeouts==0&&Bike->Ride->CurrentFloor.HitResult.GetActor()&&Bike->Ride->CurrentFloor.HitResult.GetActor()->ActorHasTag(TEXT("BattleEastsideRoute")),"Return from skatepark to trail failed");Place(FVector(37100,75100,748),180,750);Key(true);Next();}
 else if(SkateStage==7&&(Bike->GetActorLocation().X<36150||SkateClock>2.6f)){SCHECK(Bike->GetActorLocation().X<36150&&Bike->Ride->Wipeouts==0&&Bike->Ride->CurrentFloor.HitResult.GetActor()==Park,"The west half of the bigger park is not rideable deck");Capture(TEXT("west_half"));Key(false);Place(FVector(35850,74800,748),0,1150);SkateTime=float(Bike->Ride->AirRewards);Key(true);Next();}
 else if(SkateStage==8&&Bike->Ride->IsFalling()&&Bike->Ride->AirPeak>65){SCHECK(Bike->GetActorLocation().X>36300,"Unexpected takeoff before the west launch bank's lip");Next();}
 else if(SkateStage==9&&Bike->Ride->IsMovingOnGround()){Key(false);SCHECK(Bike->Ride->AirRewards>int32(SkateTime)&&Bike->Ride->Wipeouts==0&&Bike->Ride->CurrentFloor.HitResult.GetActor()==Park,"The west launch bank did not award a clean landing");Finish(true,TEXT("Trail access and return, bowl/ramp collision, pedal-only airtime, +10 landing and single-use +30 bonus, plus the new west half and its launch bank pass"));}
 if(SkateClock>8&&SkateStage!=99)Finish(false,TEXT("Skatepark phase timeout"));
#undef SCHECK
#endif
}
