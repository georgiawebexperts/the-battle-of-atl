#include "BattleMarketClosure.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
#include "HAL/PlatformMisc.h"
void TickMarketImpactAudit(ABattleMarketClosure* Fence,float Dt){
#if !UE_BUILD_SHIPPING
 auto* World=Fence->GetWorld();if(World->GetTimeSeconds()<5)return;
 static float Clock=0,Closest=MAX_flt,Peak=0;static bool Started=false,Hopped=false,Air=false;static FVector Start;
 auto* PC=UGameplayStatics::GetPlayerController(World,0);if(!PC)return;
 auto* Bike=Cast<ABattleBike>(PC->GetPawn());if(auto* Foot=Cast<ABattleRider>(PC->GetPawn()))Bike=Foot->ParkedBike;if(!Bike)return;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(World));if(!Mode)return;
 const bool Jump=FParse::Param(FCommandLine::Get(),TEXT("BattleMarketImpactJump"));
 auto Key=[&](FKey K,bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Finish=[&](bool Pass,const TCHAR* Why){Key(EKeys::W,false);UE_LOG(LogTemp,Display,TEXT("MarketImpactAudit: {\"passed\":%s,\"jump\":%s,\"reason\":\"%s\",\"closest_cm\":%.2f,\"peak_cm\":%.2f,\"airborne\":%s}"),Pass?TEXT("true"):TEXT("false"),Jump?TEXT("true"):TEXT("false"),Why,Closest,Peak,Air?TEXT("true"):TEXT("false"));FPlatformMisc::RequestExit(false);};
 if(!Started){
  Start=Fence->GetActorLocation()-FVector(1400,0,0);FHitResult Floor;FCollisionQueryParams Q;Q.AddIgnoredActor(Bike);
  if(!World->LineTraceSingleByChannel(Floor,Start+FVector(0,0,600),Start-FVector(0,0,600),ECC_Visibility,Q)){Finish(false,TEXT("No approach ground"));return;}
  Start.Z=Floor.ImpactPoint.Z+98;Bike->SetActorLocationAndRotation(Start,FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->Ride->Gear=4;Bike->Ride->bForceNextFloorCheck=true;Bike->DamageGrace=100;PC->SetControlRotation(FRotator::ZeroRotator);Started=true;Key(EKeys::W,true);
 }
 Clock+=Dt;const FVector P=Bike->GetActorLocation();const float Gap=Fence->GetActorLocation().X-P.X;Closest=FMath::Min(Closest,Gap);Peak=FMath::Max(Peak,float(P.Z-Start.Z));Air|=Bike->Ride->IsFalling();
 if(Gap<0){Finish(false,TEXT("Bike crossed market fence"));return;}
 if(!Mode->bTutorialActive||Mode->RunElapsed!=0){Finish(false,TEXT("Practice clock started during blocked entry"));return;}
 if(Jump&&!Hopped&&Gap<450){Key(EKeys::J,true);Key(EKeys::J,false);Hopped=true;}
 if(Clock>6){Finish(Closest<180&&(!Jump||(Hopped&&Air)),TEXT("Real W approach remained outside gate; jump mode requires observed airtime"));}
#endif
}
