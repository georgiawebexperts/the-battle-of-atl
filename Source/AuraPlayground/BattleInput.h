#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"

/**
 * One place that answers "does the player mean this", for a keyboard or a pad.
 *
 * Elliott asked how hard a joystick would be. The honest answer in this codebase
 * was "a rewrite in one place, not nine": the riding input is polled directly
 * (`IsInputKeyDown`) rather than read from action mappings, and
 * Config/DefaultInput.ini had no gamepad mappings at all - only dead-zone
 * entries, which is why a controller did nothing but sit there. These helpers
 * are the join, so each call site stays one line and the same code answers for
 * both devices.
 *
 * Where the game only ever wanted on/off, a stick or trigger is treated as a
 * direction; where the value is a rate (steering), the stick is passed through
 * analog, which is a real improvement over the keyboard's +/-1. The dead zones
 * live here rather than in the config so that a resting stick cannot creep the
 * bike, and so an audit can inject a value and know what it means.
 */
namespace BattleInput {
constexpr float StickDeadZone=0.18f;
constexpr float TriggerDeadZone=0.12f;

inline bool Down(const APlayerController* PC,const FKey& Key){return PC&&PC->IsInputKeyDown(Key);}
inline float Analog(const APlayerController* PC,const FKey& Key){return PC?PC->GetInputAnalogKeyState(Key):0.f;}
inline float Axis(const APlayerController* PC,const FKey& Key,float DeadZone){const float V=Analog(PC,Key);return FMath::Abs(V)<DeadZone?0.f:V;}

inline float LeftX(const APlayerController* PC){return Axis(PC,EKeys::Gamepad_LeftX,StickDeadZone);}
inline float LeftY(const APlayerController* PC){return Axis(PC,EKeys::Gamepad_LeftY,StickDeadZone);}
inline float RightX(const APlayerController* PC){return Axis(PC,EKeys::Gamepad_RightX,StickDeadZone);}
inline float RightY(const APlayerController* PC){return Axis(PC,EKeys::Gamepad_RightY,StickDeadZone);}
inline float RightTrigger(const APlayerController* PC){return FMath::Max(0.f,Axis(PC,EKeys::Gamepad_RightTriggerAxis,TriggerDeadZone));}
inline float LeftTrigger(const APlayerController* PC){return FMath::Max(0.f,Axis(PC,EKeys::Gamepad_LeftTriggerAxis,TriggerDeadZone));}

// --- riding -----------------------------------------------------------------
// Steering is the one place a pad is better than the keyboard: the keyboard can
// only ask for full lock, the stick asks for the angle.
inline float Steer(const APlayerController* PC){
 const float Keys=(Down(PC,EKeys::D)||Down(PC,EKeys::Right)?1.f:0.f)-(Down(PC,EKeys::A)||Down(PC,EKeys::Left)?1.f:0.f);
 return Keys!=0.f?Keys:LeftX(PC);
}
// Left stick forward, the way every racing game has done it. The sign is the
// engine's, not a guess: every Unreal template maps Gamepad_LeftY with Scale=+1
// to "Move Forward / Backward" (Templates/TP_ThirdPerson/Config/config.ini and
// friends), so pushing the stick up reads positive. Getting this backwards is
// not a cosmetic bug either - the first version of this treated "+1" as back,
// which held the brake on, and a bike with the brake held produces no motor at
// all (BattleBikeMovement: `Motor = Brake > 0 ? 0 : Pedal * Accel[Gear-1]`), so
// the audit saw a pedalling bike that would not move.
inline float PadPedal(const APlayerController* PC){return FMath::Max(0.f,LeftY(PC));}
inline float PadBack(const APlayerController* PC){return FMath::Max(0.f,-LeftY(PC));}
inline bool KeyPedal(const APlayerController* PC){return Down(PC,EKeys::W)||Down(PC,EKeys::Up);}
inline bool KeyBack(const APlayerController* PC){return Down(PC,EKeys::S)||Down(PC,EKeys::Down);}
inline bool KeyBrake(const APlayerController* PC){return Down(PC,EKeys::SpaceBar);}

// Firing is the right trigger on a pad and the left button on a mouse; boost is
// the left trigger, so a rider can hold throttle, steer and shoot at once.
inline bool Fire(const APlayerController* PC){return Down(PC,EKeys::LeftMouseButton)||RightTrigger(PC)>0.f;}
inline bool Aim(const APlayerController* PC){return Down(PC,EKeys::RightMouseButton)||LeftTrigger(PC)>0.f;}

// --- on foot ---------------------------------------------------------------
inline float MoveForward(const APlayerController* PC){
 const float Keys=(Down(PC,EKeys::W)||Down(PC,EKeys::Up)?1.f:0.f)-(Down(PC,EKeys::S)||Down(PC,EKeys::Down)?1.f:0.f);
 return FMath::Clamp(Keys+LeftY(PC),-1.f,1.f);
}
inline float MoveSide(const APlayerController* PC){
 const float Keys=(Down(PC,EKeys::D)||Down(PC,EKeys::Right)?1.f:0.f)-(Down(PC,EKeys::A)||Down(PC,EKeys::Left)?1.f:0.f);
 return FMath::Clamp(Keys+LeftX(PC),-1.f,1.f);
}
inline bool Sprint(const APlayerController* PC){return Down(PC,EKeys::LeftShift)||Down(PC,EKeys::Gamepad_LeftThumbstick);}

// The stick is a rate, not a delta, so callers scale it by DeltaTime - see the
// look block in APiedmontExplorer::Tick.
inline float LookYawInput(const APlayerController* PC){return RightX(PC);}
inline float LookPitchInput(const APlayerController* PC){return RightY(PC);}
}
