#include "BattleKrogCrash.h"
#include "BattleHomeData.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

/**
 * Proves the Krog crash is a permanent, violent roadblock on the tunnel
 * approach: it sits before the mouth, blocks the crown of the road, still
 * leaves a rideable shoulder, and keeps fires, bodies, brawls and screaming
 * bystanders alive.
 */
void TickBattleKrogCrashAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 static int Phase=0;static float Waited=0;
 if(!PC||!PC->GetWorld()||PC->GetWorld()->GetTimeSeconds()<5)return;
 ABattleKrogCrash* Crash=nullptr;
 for(TActorIterator<ABattleKrogCrash> It(PC->GetWorld());It;++It)if(!Crash)Crash=*It;
 if(!Crash){Waited+=Dt;if(Waited>40.f){UE_LOG(LogTemp,Display,TEXT("BattleKrogCrashAudit: {\"passed\":false,\"reason\":\"no crash spawned\"}"));PC->ConsoleCommand(TEXT("quit"));}return;}
 if(Phase>0)return;Phase=1;
 const FVector Dir=(BattleHomeData::TunnelEntry-Crash->WreckSpot).GetSafeNormal2D();
 FHitResult Hit;FCollisionQueryParams Q;
 const bool bBlocks=PC->GetWorld()->LineTraceSingleByChannel(Hit,Crash->WreckSpot-Dir*1400.f+FVector(0,0,70),Crash->WreckSpot+Dir*1400.f+FVector(0,0,70),ECC_Visibility,Q);
 const bool bBefore=Crash->DistanceToTunnelCm>0.f&&Crash->DistanceToTunnelCm<4500.f;
 const bool bGap=Crash->GapIsRideable();
 const bool bCast=Crash->Fires>=2&&Crash->Bodies>=3&&Crash->Bystanders>=6&&Crash->Brawlers>=2;
 const bool bShout=Crash->ShoutRemaining>0.f&&!Crash->ShoutText.IsEmpty();
 const bool Pass=bBlocks&&bBefore&&bGap&&bCast&&bShout;
 UE_LOG(LogTemp,Display,TEXT("BattleKrogCrashAudit: {\"passed\":%s,\"blocks_road\":%s,\"before_tunnel\":%s,\"distance_to_tunnel_cm\":%.0f,\"gap_open\":%s,\"fires\":%d,\"bodies\":%d,\"bystanders\":%d,\"brawlers\":%d,\"shout\":\"%s\"}"),
  Pass?TEXT("true"):TEXT("false"),bBlocks?TEXT("true"):TEXT("false"),bBefore?TEXT("true"):TEXT("false"),Crash->DistanceToTunnelCm,
  bGap?TEXT("true"):TEXT("false"),Crash->Fires,Crash->Bodies,Crash->Bystanders,Crash->Brawlers,*Crash->ShoutText);
 PC->ConsoleCommand(TEXT("quit"));
#endif
}
