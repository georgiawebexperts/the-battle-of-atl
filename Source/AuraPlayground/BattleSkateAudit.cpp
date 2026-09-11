#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleSkatepark.h"
#include "BattlePickup.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "InputKeyEventArgs.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
void ABattleMacController::TickSkateAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5||SkateStage==99)return;
 auto* Bike=Cast<ABattleBike>(GetPawn());auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));ABattleSkatepark* Park=nullptr;if(TActorIterator<ABattleSkatepark> It(GetWorld());It)Park=*It;if(!Bike||!Mode)return;SkateClock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BattleSkateAudit: {\"passed\":%s,\"stage\":%d,\"reason\":\"%s\",\"air_rewards\":%d,\"air_peak_cm\":%.2f,\"x\":%.2f,\"z\":%.2f}"),Pass?TEXT("true"):TEXT("false"),SkateStage,Reason,Bike->Ride->AirRewards,Bike->Ride->AirPeak,Bike->GetActorLocation().X,Bike->GetActorLocation().Z);SkateStage=99;ConsoleCommand(TEXT("quit"));};
#define SCHECK(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Next=[&](){SkateStage++;SkateClock=0;};
 auto Key=[&](bool Down){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::W,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Place=[&](FVector P,float Yaw,float Speed){Bike->SetActorLocationAndRotation(P,FRotator(0,Yaw,0),false,nullptr,ETeleportType::TeleportPhysics);SetControlRotation(FRotator(0,Yaw,0));Bike->Ride->StopMovementImmediately();Bike->Ride->SetMovementMode(MOVE_Walking);Bike->Ride->bForceNextFloorCheck=true;Bike->Ride->Speed=Speed;Bike->Ride->Gear=4;};
 if(SkateStage==0){
  SCHECK(Park&&Park->Bonuses.Num()==3,"Missing park/bonus layout");
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  FCollisionQueryParams Q(SCENE_QUERY_STAT(SkateSurfaceAudit),false,Bike);
  for(FVector P:{FVector(38050,73650,470),FVector(38050,74400,510),FVector(40100,73500,950),FVector(39000,74000,650)}){FHitResult H;const bool Hit=GetWorld()->LineTraceSingleByChannel(H,P+FVector(0,0,300),P-FVector(0,0,150),ECC_Visibility,Q);UE_LOG(LogTemp,Display,TEXT("SkateSurface: expected=%s hit=%s actor=%s"),*P.ToString(),*H.ImpactPoint.ToString(),*GetNameSafe(H.GetActor()));SCHECK(Hit&&H.GetActor()==Park&&FMath::Abs(H.ImpactPoint.Z-P.Z)<4,"Bowl/ramp surface missing or obstructed");}
  Place(FVector(41600,74168,580),180,700);Key(true);Next();
 }else if(SkateStage==1&&SkateClock>2.2f){
  SCHECK(Bike->GetActorLocation().X<40750&&Bike->Ride->CurrentFloor.HitResult.GetActor()==Park&&Bike->Ride->Wipeouts==0,"Cannot ride from trail into skatepark");
  Place(FVector(39100,73500,748),0,1100);SkateTime=Mode->TimeRemaining;Next();
 }else if(SkateStage==2&&Bike->Ride->IsFalling()&&Bike->Ride->AirPeak>65){SCHECK(Bike->GetActorLocation().X>40000,"Unexpected takeoff before ramp lip");Next();}
 else if(SkateStage==3&&Bike->Ride->IsMovingOnGround()){
  SCHECK(Bike->Ride->AirRewards>=1&&Mode->TimeRemaining>SkateTime+5&&Bike->Ride->Wipeouts==0,"Ramp did not award clean airtime bonus");Key(false);Place(FVector(38050,73650,568),0,0);SkateTime=Mode->TimeRemaining;Next();
 }else if(SkateStage==4&&SkateClock>.3f){
  SCHECK(Park->Bonuses[0]->bConsumed&&Mode->TimeRemaining>SkateTime+29,"Bowl bonus did not grant thirty seconds");SCHECK(!Park->Bonuses[0]->TryCollect(Bike),"Bowl bonus repeated");SCHECK(Bike->Ride->CurrentFloor.HitResult.GetActor()==Park&&Bike->GetActorLocation().Z>560,"Bike fell through bowl");Place(FVector(39700,74168,748),0,700);Key(true);Next();
 }
 else if(SkateStage==5&&Bike->GetActorLocation().X>41810){Key(false);Bike->Ride->Speed=0;Bike->Ride->StopMovementImmediately();Next();}
 else if(SkateStage==6&&SkateClock>.3f){SCHECK(Bike->Ride->IsMovingOnGround()&&Bike->Ride->Wipeouts==0&&Bike->Ride->CurrentFloor.HitResult.GetActor()&&Bike->Ride->CurrentFloor.HitResult.GetActor()->ActorHasTag(TEXT("BattleEastsideRoute")),"Return from skatepark to trail failed");Finish(true,TEXT("Trail access and return, bowl/ramp collision, pedal-only airtime, +10 landing and single-use +30 bonus pass"));}
 if(SkateClock>8&&SkateStage!=99)Finish(false,TEXT("Skatepark phase timeout"));
#undef SCHECK
#endif
}
