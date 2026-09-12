#include "BattleMarketClosure.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleTutorialData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
#include "HAL/PlatformMisc.h"
void TickMarketImpactAudit(ABattleMarketClosure* Fence,float Dt){
#if !UE_BUILD_SHIPPING
 auto* World=Fence->GetWorld();if(World->GetTimeSeconds()<5)return;
 static float Clock=0,Closest=MAX_flt,Peak=0,JumpReleaseAt=MAX_flt;static bool Started=false,Hopped=false,Air=false;static FVector Start;
 auto* PC=UGameplayStatics::GetPlayerController(World,0);if(!PC)return;
 auto* Bike=Cast<ABattleBike>(PC->GetPawn());if(auto* Foot=Cast<ABattleRider>(PC->GetPawn()))Bike=Foot->ParkedBike;if(!Bike)return;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(World));if(!Mode)return;
 float Offset=0;FParse::Value(FCommandLine::Get(),TEXT("BattleMarketImpactOffset="),Offset);
 const bool Pier=FParse::Param(FCommandLine::Get(),TEXT("BattleMarketPier"));
 const FVector Target=Pier?BattleTutorialData::Gate+FVector(0,360,0):Fence->GetActorLocation();
 const bool Junction=FParse::Param(FCommandLine::Get(),TEXT("BattleMarketJunction"));
 const FRotator Heading(0,Junction?24.12f:0,0);
 const bool OnFoot=FParse::Param(FCommandLine::Get(),TEXT("BattleMarketImpactFoot"));
 const bool Jump=FParse::Param(FCommandLine::Get(),TEXT("BattleMarketImpactJump"));
 auto Key=[&](FKey K,bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Finish=[&](bool Pass,const TCHAR* Why){Key(EKeys::W,false);Key(EKeys::LeftShift,false);Key(EKeys::SpaceBar,false);UE_LOG(LogTemp,Display,TEXT("MarketImpactAudit: {\"passed\":%s,\"jump\":%s,\"on_foot\":%s,\"reason\":\"%s\",\"offset_cm\":%.2f,\"closest_cm\":%.2f,\"peak_cm\":%.2f,\"airborne\":%s}"),Pass?TEXT("true"):TEXT("false"),Jump?TEXT("true"):TEXT("false"),OnFoot?TEXT("true"):TEXT("false"),Why,Offset,Closest,Peak,Air?TEXT("true"):TEXT("false"));FPlatformMisc::RequestExit(false);};
 if(!Started){
  if((Junction||Pier)&&OnFoot&&!Bike->Dismount()){Finish(false,TEXT("Initial practice dismount failed"));return;}
  Start=Pier?Target-FVector(1000,0,0):Junction?Fence->GetActorLocation()+FVector(0,640,0)-Heading.Vector()*1000:Fence->GetActorLocation()-FVector(1400,-Offset,0);FHitResult Floor;FCollisionQueryParams Q;Q.AddIgnoredActor(Bike);
  if(!World->LineTraceSingleByChannel(Floor,Start+FVector(0,0,600),Start-FVector(0,0,600),ECC_Visibility,Q)){Finish(false,TEXT("No approach ground"));return;}
  Start.Z=Floor.ImpactPoint.Z+98;Bike->SetActorLocationAndRotation(Start,Heading,false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->Ride->Gear=4;Bike->Ride->bForceNextFloorCheck=true;Bike->DamageGrace=100;PC->SetControlRotation(Heading);if(OnFoot){if(!Cast<ABattleRider>(PC->GetPawn())&&!Bike->Dismount()){Finish(false,TEXT("Dismount failed"));return;}if(Junction||Pier){PC->GetPawn()->SetActorLocationAndRotation(Start,Heading,false,nullptr,ETeleportType::TeleportPhysics);PC->SetControlRotation(Heading);}Start=PC->GetPawn()->GetActorLocation();Key(EKeys::LeftShift,true);}Started=true;Key(EKeys::W,true);
 }
 Clock+=Dt;const FVector P=PC->GetPawn()->GetActorLocation();const float Gap=Target.X-P.X;Closest=FMath::Min(Closest,Gap);Peak=FMath::Max(Peak,float(P.Z-Start.Z));if(auto* Foot=Cast<ABattleRider>(PC->GetPawn()))Air|=Foot->GetCharacterMovement()->IsFalling();else Air|=Bike->Ride->IsFalling();
 if(Gap<0){Finish(false,TEXT("Player crossed market fence"));return;}
 if(!Mode->bTutorialActive||Mode->RunElapsed!=0){Finish(false,TEXT("Practice clock started during blocked entry"));return;}
 if(Jump&&!Hopped&&Gap<450){Key(OnFoot?EKeys::SpaceBar:EKeys::J,true);if(OnFoot)JumpReleaseAt=Clock+.2f;else Key(EKeys::J,false);Hopped=true;}
 if(Clock>=JumpReleaseAt){Key(EKeys::SpaceBar,false);JumpReleaseAt=MAX_flt;}
 if(Clock>6){Finish(Closest<180&&(!Jump||(Hopped&&Air)),TEXT("Real W approach remained outside gate; foot mode sprints; jump requires airtime"));}
#endif
}
