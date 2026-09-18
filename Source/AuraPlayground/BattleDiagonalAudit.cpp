#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"

/**
 * "When I'm running around it doesn't seem like I can hit two keys at once and
 * move diagonal." Reported by Elliott on packaged 126, and never answered.
 *
 * Holds one key, then two keys, and measures the direction the character
 * actually travels relative to where the camera is looking. The thing to watch
 * is not just the angle: `both_keys_down` reports whether the input layer even
 * believes two movement keys are held at the same time, which is the half of
 * the complaint a keyboard or an input mode can break on its own.
 *
 *   -BattleDiagonalAudit
 */
void TickBattleDiagonalAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{float Clock=0;int Stage=0;float Forward=0,Right=0,Left=0;bool BothRight=false,BothLeft=false;};
 static FState S;
 if(S.Stage<0||!PC)return;
 if(PC->GetWorld()->GetTimeSeconds()<5)return;
 S.Clock+=Dt;
 auto Key=[&](FKey K,bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto End=[&](bool Pass,const TCHAR* Why){
  UE_LOG(LogTemp,Display,TEXT("BattleDiagonalAudit: {\"passed\":%s,\"reason\":\"%s\",\"forward_deg\":%.1f,\"right_deg\":%.1f,\"left_deg\":%.1f,\"two_keys_down\":%s}"),
   Pass?TEXT("true"):TEXT("false"),Why,S.Forward,S.Right,S.Left,(S.BothRight&&S.BothLeft)?TEXT("true"):TEXT("false"));
  Key(EKeys::W,false);Key(EKeys::A,false);Key(EKeys::D,false);
  S.Stage=-1;PC->ConsoleCommand(TEXT("quit"));
 };
 auto Bearing=[&]()->float{
  const APawn* Pawn=PC->GetPawn();if(!Pawn)return 0.f;
  const FVector V=FVector(Pawn->GetVelocity().X,Pawn->GetVelocity().Y,0);
  const FRotator Yaw(0,PC->GetControlRotation().Yaw,0);
  const FVector Fwd=FRotationMatrix(Yaw).GetUnitAxis(EAxis::X),Side=FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y);
  if(V.SizeSquared()<100.f)return 999.f;
  return FMath::RadiansToDegrees(FMath::Atan2(FVector::DotProduct(V,Side),FVector::DotProduct(V,Fwd)));
 };
 if(S.Stage==0){
  if(auto* Bike=Cast<ABattleBike>(PC->GetPawn()))if(!Bike->Dismount())return;
  if(!Cast<ABattleRider>(PC->GetPawn()))return;
  // Straighten the camera: the bearing is measured against where it looks.
  if(const APawn* Pawn=PC->GetPawn())PC->SetControlRotation(FRotator(0,Pawn->GetActorRotation().Yaw,0));
  Key(EKeys::W,true);S.Stage=1;S.Clock=0;return;
 }
 if(S.Clock<.7f)return;
 if(S.Stage==1){
  S.Forward=Bearing();Key(EKeys::W,false);Key(EKeys::D,true);Key(EKeys::W,true);
  S.Stage=2;S.Clock=0;return;
 }
 if(S.Stage==2)S.BothRight|=PC->IsInputKeyDown(EKeys::W)&&PC->IsInputKeyDown(EKeys::D);
 if(S.Stage==3)S.BothLeft|=PC->IsInputKeyDown(EKeys::W)&&PC->IsInputKeyDown(EKeys::A);
 if(S.Clock<.7f)return;
 if(S.Stage==2){
  S.Right=Bearing();Key(EKeys::D,false);Key(EKeys::A,true);
  S.Stage=3;S.Clock=0;return;
 }
 if(S.Stage==3){
  S.Left=Bearing();Key(EKeys::A,false);
  const bool HeldBoth=S.BothRight&&S.BothLeft;
  const bool Forward=(FMath::Abs(S.Forward)<12.f);
  const bool DiagRight=(S.Right>30.f&&S.Right<60.f);
  const bool DiagLeft=(S.Left>-60.f&&S.Left<-30.f);
  if(!HeldBoth){End(false,TEXT("input layer never saw two movement keys held together"));return;}
  if(!Forward){End(false,TEXT("forward-only travel was not straight ahead"));return;}
  if(!DiagRight){End(false,TEXT("W+D did not travel 45 degrees to the right"));return;}
  if(!DiagLeft){End(false,TEXT("W+A did not travel 45 degrees to the left"));return;}
  End(true,TEXT("forward, W+D and W+A all travel where they should"));
  return;
 }
#endif
}
