#include "BattleSkyline.h"
#include "BattleCheckpoints.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

/**
 * Measures whether the downtown skyline has a clear line of sight from the
 * places a player actually rides. The terrain profile is gentle enough that it
 * never blocks the tower tops; park trees and the park's own buildings are what
 * hide the cluster, so the check casts a ray along the skyline bearing from each
 * checkpoint and names the first blocker.
 */
void TickBattleSkylineAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{int Phase=0;float Waited=0;};
 static FState S;
 if(!PC||!PC->GetWorld()||PC->GetWorld()->GetTimeSeconds()<5)return;
 ABattleSkyline* Line=nullptr;
 for(TActorIterator<ABattleSkyline> It(PC->GetWorld());It;++It)if(!Line)Line=*It;
 if(!Line){
  S.Waited+=Dt;
  if(S.Waited>40.f){UE_LOG(LogTemp,Display,TEXT("BattleSkylineAudit: {\"passed\":false,\"reason\":\"no skyline spawned\"}"));PC->ConsoleCommand(TEXT("quit"));}
  return;
 }
 if(S.Phase>0)return;
 S.Phase=1;
 struct FSpot{FVector XY;FString Name;};
 TArray<FSpot> Spots;
 const int32 Count=(int32)UE_ARRAY_COUNT(BattleCheckpoints::Anchors);
 for(int32 I=0;I<Count;++I){
  const auto& A=BattleCheckpoints::Anchors[I];
  Spots.Add({FVector(A.X,A.Y,0),FString(A.Name)});
 }
 Spots.Add({FVector(-17126.f,-5089.f,0),TEXT("gate")});
 Spots.Add({FVector(-19000.f,-2000.f,0),TEXT("lake-east")});
 Spots.Add({FVector(-14000.f,3000.f,0),TEXT("lake-north")});
 Spots.Add({FVector(-6000.f,1000.f,0),TEXT("lake-centre")});
 Spots.Add({FVector(-21000.f,4000.f,0),TEXT("oval")});
 // Sample tower tops, not the cluster origin: the origin sits on the ground
 // behind a ridge, so aiming there measures the hill, not the skyline.
 struct FTower{FVector Local;float HeightCm;};
 const FTower Towers[]={
  {FVector(-600.f,300.f,0),7400.f},{FVector(1500.f,-900.f,0),4200.f},{FVector(0.f,0.f,0),5600.f},
  {FVector(-2400.f,1400.f,0),3100.f},{FVector(2200.f,1200.f,0),4000.f}};
 int32 ClearOnBearing=0;float BestClear=0.f;int32 BestYaw=0;FString BestSpot=TEXT("none");
 for(const FSpot& Spot:Spots){
  FHitResult Ground;FCollisionQueryParams GQ;GQ.bIgnoreTouches=true;
  if(!PC->GetWorld()->LineTraceSingleByChannel(Ground,Spot.XY+FVector(0,0,6000),Spot.XY-FVector(0,0,20000),ECC_Visibility,GQ))continue;
  const FVector Eye=Ground.ImpactPoint+FVector(0,0,165);
  const FVector ToCentre=Line->Centre-Eye;
  const float Distance=FVector::Dist2D(Eye,Line->Centre);
  int32 Clear=0;FString Blocker=TEXT("none");float NearCm=0.f;
  for(const FTower& Tower:Towers){
   const FVector Target=Line->Centre+Tower.Local+FVector(0,0,Tower.HeightCm);
   FHitResult Hit;FCollisionQueryParams Q;Q.bIgnoreTouches=true;
   const bool bBlocked=PC->GetWorld()->LineTraceSingleByChannel(Hit,Eye,Target,ECC_Visibility,Q);
   const float ToTarget=FVector::Dist(Eye,Target);
   if(!bBlocked){++Clear;continue;}
   const float HitCm=FVector::Dist(Eye,Hit.ImpactPoint);
   if(HitCm<ToTarget*.98f){
    if(NearCm==0.f||HitCm<NearCm){NearCm=HitCm;Blocker=Hit.GetActor()?Hit.GetActor()->GetName():TEXT("none");}
   }else ++Clear;
  }
  if(Clear>=3)++ClearOnBearing;
  UE_LOG(LogTemp,Display,TEXT("BattleSkylineAudit: spot=%s eye_z=%.0f bearing_yaw=%.0f distance_m=%.0f towers_clear=%d/5 first_blocker_m=%.0f blocker=%s"),
   *Spot.Name,Eye.Z,ToCentre.Rotation().Yaw,Distance/100.f,Clear,NearCm/100.f,*Blocker);
  for(int32 Yaw=0;Yaw<360;Yaw+=15){
   const FVector Probe=FRotator(0,(float)Yaw,0).Vector()+FVector(0,0,FMath::Tan(FMath::DegreesToRadians(5.f)));
   FHitResult Sweep;FCollisionQueryParams SQ;SQ.bIgnoreTouches=true;
   const bool bHit=PC->GetWorld()->LineTraceSingleByChannel(Sweep,Eye,Eye+Probe.GetSafeNormal()*70000.f,ECC_Visibility,SQ);
   const float SweepClear=bHit?FVector::Dist(Eye,Sweep.ImpactPoint):70000.f;
   if(SweepClear>BestClear){BestClear=SweepClear;BestSpot=Spot.Name;BestYaw=Yaw;}
  }
 }
 UE_LOG(LogTemp,Display,TEXT("BattleSkylineAudit: {\"passed\":%s,\"towers\":%d,\"tallest_m\":%.0f,\"clear_vantages\":%d,\"best_spot\":\"%s\",\"best_yaw\":%d,\"best_clear_m\":%.0f}"),
  ClearOnBearing>0?TEXT("true"):TEXT("false"),Line->Towers,Line->TallestM,ClearOnBearing,*BestSpot,BestYaw,BestClear/100.f);
 PC->ConsoleCommand(TEXT("quit"));
#endif
}
