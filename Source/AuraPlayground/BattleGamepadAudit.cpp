#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleInput.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"

/**
 * Proves a controller can play this game, and does it without a controller.
 *
 * Elliott asked how hard a joystick would be. The input model was the answer:
 * the riding controls are polled as raw keys, the one-shot actions were bound key
 * by key, and Config/DefaultInput.ini had no gamepad mappings at all - only
 * dead-zone entries - so a pad did nothing. BattleInput.h now answers for both
 * devices; this audit is what stops that from being a comment.
 *
 * It is two functions because of one measured trap. A first version injected the
 * stick from the controller's Tick and the bike never saw it, while the same
 * audit read 1.000 straight back out of GetInputAnalogKeyState - and a keyboard
 * control in the same audit showed W reaching the pedal fine. The cause is frame
 * order: pawns tick before the controller, UPlayerInput::ProcessInputStack runs
 * before both, and a gamepad axis has ShouldUpdateAxisWithoutSamples set, so a
 * frame with no fresh sample zeroes it again. A real pad samples every frame
 * ahead of that processing; an injection from Tick lands after it. So injection
 * happens in TickActor before Super, and the judging happens in Tick, after the
 * bike has had its say.
 */
namespace {
struct FGamepadProbe{
 int32 Phase=0,Sub=0,Shots=0,Gear=0,Checked=0;
  float Clock=0,PressClock=0,BaseYaw=0,Steer=0,StickSpeed=0,KeySpeed=0,WalkCm=0,LookDeg=0;
  FVector WalkFrom=FVector::ZeroVector;
 bool bPressIssued=false,bReleaseIssued=false,bGearDown=false;
 bool bPedal=false,bSteer=false,bBrake=false,bShot=false,bHop=false,bGear=false,bBoost=false,bHorn=false,bWalk=false,bLook=false;
};
FGamepadProbe G;
}

/** Runs in TickActor, before the input stack processes this frame - see above. */
void GamepadAuditInject(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 if(!PC)return;
 const FInputDeviceId Device=IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
 auto Key=[&](FKey K,bool bDown){PC->InputKey(FInputKeyEventArgs(nullptr,Device,K,bDown?IE_Pressed:IE_Released,bDown?1.f:0.f,false,0));};
 // The axis constructor, not the event one: it carries NumSamples, and
 // UPlayerInput only moves a key's accumulator into its value when there was a
 // sample. Injecting through the event constructor leaves NumSamples at zero and
 // the stick never becomes readable - a silent failure this audit hit.
 auto Axis=[&](FKey K,float V){PC->InputKey(FInputKeyEventArgs(nullptr,Device,K,V,Dt,1,0));};
 auto Tap=[&](FKey K){
  if(!G.bPressIssued){Key(K,true);G.bPressIssued=true;}
  else if(!G.bReleaseIssued&&G.PressClock>.15f){Key(K,false);G.bReleaseIssued=true;}
  G.PressClock+=Dt;
 };
 switch(G.Phase){
  // 0: the pedal, then - only if that fails - the same test with W held, so a
  //    failure says whether it was the pad or the harness.
  case 0:if(G.Sub==0)Axis(EKeys::Gamepad_LeftY,1.f);else Key(EKeys::W,true);break;
  case 1:Axis(EKeys::Gamepad_LeftX,.5f);break;
  case 2:Axis(EKeys::Gamepad_LeftY,-1.f);break;
  case 3:Axis(EKeys::Gamepad_RightTriggerAxis,1.f);break;
  // Get rolling first: UBattleBikeMovement::Hop refuses a stationary bike, and
  // by this point the audit has just braked to a stop to test the brake.
  case 4:if(G.Sub==0)Axis(EKeys::Gamepad_LeftY,1.f);else Tap(EKeys::Gamepad_FaceButton_Bottom);break;
  // Shift whichever way has room: at the top gear there is none upward, and a
  // test that only ever presses "up" fails on a bike already in fifth.
  case 5:if(G.Sub>0)Tap(G.bGearDown?EKeys::Gamepad_LeftShoulder:EKeys::Gamepad_RightShoulder);break;
  case 6:if(G.Sub>0)Tap(EKeys::Gamepad_LeftTrigger);break;
  case 7:Tap(EKeys::Gamepad_FaceButton_Top);break;
  case 8:if(G.Sub==0)Axis(EKeys::Gamepad_LeftY,1.f);else Axis(EKeys::Gamepad_RightX,1.f);break;
  default:break;
 }
#endif
}

/** Runs in Tick: advances the phases and judges what the game did with them. */
void TickGamepadAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 if(!PC||PC->GetWorld()->GetTimeSeconds()<5)return;
 auto* Bike=Cast<ABattleBike>(PC->GetPawn());
 // Every phase but the last is ridden, so the bike is required - but only until
 // the on-foot phase, which dismounts on purpose. Checking it unconditionally
 // made the audit fail its own last phase with "Not on a bike"; the wait is
 // there because the pawn is not always the bike on the first frame either.
 if(!Bike){
  if(G.Phase<8){
   if(PC->GetWorld()->GetTimeSeconds()<12.f)return;
   UE_LOG(LogTemp,Display,TEXT("BattleGamepadAudit: {\"passed\":false,\"reason\":\"Not on a bike\"}"));
   PC->ConsoleCommand(TEXT("quit"));return;
  }
 }
 if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(PC))){Mode->bTutorialActive=false;Mode->StartCountdown=0;}
 // Sub has to reset with the phase: it is the phase's own step counter, and
 // leaving it set meant the hop phase started in its second step and skipped the
 // run-up that a stationary bike needs - which failed as "A did not hop".
 auto Advance=[&](int32 Next){G.Phase=Next;G.Sub=0;G.Clock=0;G.PressClock=0;G.bPressIssued=false;G.bReleaseIssued=false;};
 auto End=[&](bool Pass,const TCHAR* Why){
  UE_LOG(LogTemp,Display,TEXT("BattleGamepadAudit: {\"passed\":%s,\"checks\":%d,\"reason\":\"%s\",\"pedal\":%s,\"steer_analog\":%.2f,\"brake_reverse\":%s,\"trigger_fired\":%s,\"a_hop\":%s,\"shoulder_gear\":%s,\"left_trigger_boost\":%s,\"y_horn\":%s,\"stick_walk_cm\":%.0f,\"stick_look_deg\":%.0f}"),
   Pass?TEXT("true"):TEXT("false"),G.Checked,Why,
   G.bPedal?TEXT("true"):TEXT("false"),G.Steer,G.bBrake?TEXT("true"):TEXT("false"),
   G.bShot?TEXT("true"):TEXT("false"),G.bHop?TEXT("true"):TEXT("false"),G.bGear?TEXT("true"):TEXT("false"),
   G.bBoost?TEXT("true"):TEXT("false"),G.bHorn?TEXT("true"):TEXT("false"),G.WalkCm,G.LookDeg);
  PC->ConsoleCommand(TEXT("quit"));
 };
 auto Reach=[&](bool Condition,const TCHAR* Why){if(!Condition){End(false,Why);return true;}G.Checked++;return false;};
 G.Clock+=Dt;

 // 0. Left stick forward pedals the bike. Sub-phase 1 is the keyboard control.
 if(G.Phase==0){
  if(G.Sub==0&&G.Clock>1.2f){
   G.StickSpeed=Bike->Ride->Speed;
   G.Sub=1;G.Clock=0;return;
  }
  if(G.Sub==1&&G.Clock>1.2f){
   G.KeySpeed=Bike->Ride->Speed;
   G.bPedal=G.StickSpeed>15.f;
   if(Reach(G.bPedal,G.KeySpeed>15.f?TEXT("Left stick forward did not pedal the bike while W did"):TEXT("Neither the left stick nor W pedalled the bike, so this is the harness and not the pad")))return;
   Advance(1);
  }
  return;
 }
 // 1. The stick steers by angle, not by full lock - the reason a pad is better
 //    than A and D here.
 if(G.Phase==1){
  if(G.Clock<=Dt*2)G.BaseYaw=Bike->GetActorRotation().Yaw;
  if(G.Clock>1.2f){
   G.Steer=Bike->Ride->Steer;
   const float Turn=FMath::Abs(FMath::FindDeltaAngleDegrees(G.BaseYaw,Bike->GetActorRotation().Yaw));
   // Not "equals the injected 0.5": UPlayerInput massages an axis through the
   // AxisConfig dead zone (0.25 here) and rescales, so half a stick arrives as
   // about a third. What matters is that it arrives as a fraction at all - the
   // keyboard can only send +/-1, and a full lock for half a stick is the thing
   // this check exists to catch.
   if(Reach(G.Steer>0.2f&&G.Steer<0.8f&&Turn>.3f,TEXT("Half a stick did not arrive as a part-lock turn")))return;
   G.Shots=Bike->ShotsFired;
   Advance(2);
  }
  return;
 }
 // 2. Pulling the stick back brakes, and asks for reverse once stopped - the
 //    same job S does, without a second key.
 if(G.Phase==2){if(G.Clock>1.f){G.bBrake=Bike->Ride->Brake==1&&Bike->Ride->bReverseRequested;if(Reach(G.bBrake,TEXT("Stick back did not brake and request reverse")))return;Advance(3);}return;}
 // 3. The right trigger fires, on the path the left mouse button uses.
 if(G.Phase==3){if(G.Clock>1.f){G.bShot=Bike->ShotsFired>G.Shots;if(Reach(G.bShot,TEXT("Right trigger did not fire")))return;Advance(4);}return;}
 // 4. A hops - after a run-up, because a stationary hop is refused by design.
 if(G.Phase==4){
  if(G.Sub==0&&G.Clock>1.5f){
   G.bPressIssued=false;G.bReleaseIssued=false;G.PressClock=0;
   G.Sub=1;G.Clock=0;
   return;
  }
  if(G.Sub==1&&G.Clock>1.4f){
   G.bHop=Bike->Ride->IsFalling()||Bike->Ride->AirPeak>20.f;
   if(Reach(G.bHop,TEXT("A did not hop the bike")))return;
   Advance(5);
  }
  return;
 }
 // 5. A shoulder shifts a gear - after the bike is back on the ground, since a
 //    mid-air shift is refused.
 if(G.Phase==5){
  if(G.Sub==0&&(Bike->Ride->IsMovingOnGround()||G.Clock>3.f)){
   G.Gear=Bike->Ride->Gear;
   G.bGearDown=G.Gear>=5;
   G.bPressIssued=false;G.bReleaseIssued=false;G.PressClock=0;
   G.Sub=1;G.Clock=0;
   return;
  }
  if(G.Sub==1&&G.Clock>.9f){
   G.bGear=G.bGearDown?Bike->Ride->Gear<G.Gear:Bike->Ride->Gear>G.Gear;
   if(Reach(G.bGear,TEXT("A shoulder did not shift gear")))return;
   Advance(6);
  }
  return;
 }
 // 6. The left trigger boosts. A fresh bike has no nitro and no charges, and
 //    Boost() refuses without one of them, so the meter is filled first - this
 //    checks the input path, not the pickup economy.
 if(G.Phase==6){
  if(G.Sub==0){
   Bike->Nitro=100.f;
   if(G.Clock>.3f){G.bPressIssued=false;G.bReleaseIssued=false;G.PressClock=0;G.Sub=1;G.Clock=0;}
   return;
  }
  if(G.Sub==1&&G.Clock>.9f){
   G.bBoost=Bike->Ride->BoostRemaining>0.f;
   if(Reach(G.bBoost,TEXT("Left trigger did not boost")))return;
   Advance(7);
  }
  return;
 }
 // 7. Y sounds the horn.
 if(G.Phase==7){if(G.Clock>1.f){G.bHorn=Bike->HornCount>0;if(Reach(G.bHorn,TEXT("Y did not sound the horn")))return;Advance(8);}return;}
 // 8. On foot: the game's own dismount, then walk on the left stick and turn on
 //    the right one.
 if(G.Phase==8){
  if(G.Sub==0){
   if(Bike)Bike->Dismount();
   auto* Rider=Cast<ABattleRider>(PC->GetPawn());
   if(!Rider){if(G.Clock>3.f){End(false,TEXT("Dismount left no rider to test on foot"));return;}return;}
   G.WalkFrom=Rider->GetActorLocation();
   G.LookDeg=PC->GetControlRotation().Yaw;
   G.Sub=1;G.Clock=0;
   return;
  }
  if(G.Sub==1&&G.Clock>1.4f){
   auto* Rider=Cast<ABattleRider>(PC->GetPawn());
   if(!Rider){End(false,TEXT("Lost the rider mid-walk"));return;}
   // Displacement, not distance from the world origin: the first version
   // measured |position| and would have read almost nothing if the rider had
   // walked toward the origin.
   G.WalkCm=(Rider->GetActorLocation()-G.WalkFrom).Size2D();
   G.Sub=2;G.Clock=0;
   return;
  }
  if(G.Sub==2&&G.Clock>1.2f){
   G.LookDeg=FMath::Abs(FMath::FindDeltaAngleDegrees(G.LookDeg,PC->GetControlRotation().Yaw));
   G.bWalk=G.WalkCm>120.f;G.bLook=G.LookDeg>3.f;
   if(Reach(G.bWalk,TEXT("Left stick did not walk the rider")))return;
   if(Reach(G.bLook,TEXT("Right stick did not turn the rider")))return;
   End(true,TEXT("Left stick pedals, steers by angle, brakes and reverses and walks on foot; the right trigger fires; A hops; a shoulder shifts gear; the left trigger boosts; Y sounds the horn; the right stick turns the head"));
  }
  return;
 }
#endif
}
