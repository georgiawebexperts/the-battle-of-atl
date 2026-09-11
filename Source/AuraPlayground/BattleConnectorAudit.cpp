#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleHomeData.h"
#include "PiedmontPathSpline.h"
#include "Components/SplineComponent.h"
#include "EngineUtils.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Algo/Reverse.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "PiedmontDarkZone.h"

// Opt-in cooked-game physics test. Only each leg's starting fixture is teleported;
// the entire crossing is traversed through ordinary keyboard input and movement.
void ABattleMacController::TickConnectorAudit(float Dt){
#if !UE_BUILD_SHIPPING
 const bool Home=FParse::Param(FCommandLine::Get(),TEXT("BattleHomeDriveAudit"));
 const bool Krog=FParse::Param(FCommandLine::Get(),TEXT("BattleKrogAudit"));
 const bool Eastside=Home||Krog||FParse::Param(FCommandLine::Get(),TEXT("BattleEastsideAudit"));
 if(GetWorld()->GetTimeSeconds()<5)return;
 auto* Bike=Cast<ABattleBike>(GetPawn());if(!Bike)return;
 auto Finish=[&](bool Passed){
  UE_LOG(LogTemp,Display,TEXT("Connector state: key=%d pedal=%.1f speed=%.1f movement=%d tick=%d"),IsInputKeyDown(EKeys::W),Bike->Ride->Pedal,Bike->Ride->Speed,int32(Bike->Ride->MovementMode),Bike->IsActorTickEnabled());
  UE_LOG(LogTemp,Display,TEXT("TrailGroundAudit: samples=%d paved=%d"),TrailGroundSamples,TrailPavedSamples);
  if(Eastside)Passed=Passed&&TrailGroundSamples>100&&TrailPavedSamples>=TrailGroundSamples*.99f;
  if(Krog){UE_LOG(LogTemp,Display,TEXT("TunnelLightAudit: lit=%d unlit=%d"),TunnelLitSamples,TunnelUnlitSamples);Passed=Passed&&TunnelLitSamples>10&&TunnelUnlitSamples==0;}
  FlushPressedKeys();
  UE_LOG(LogTemp,Display,TEXT("BattleConnectorAudit: {\"passed\":%s,\"completedLegs\":%d,\"travelCm\":%.2f,\"maxCenterlineErrorCm\":%.2f,\"wipeouts\":%d}"),Passed?TEXT("true"):TEXT("false"),ConnectorLeg,ConnectorTravel,ConnectorMaxError,Bike->Ride->Wipeouts-ConnectorWipeouts);
  UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);
 };
 if(ConnectorPoints.IsEmpty()){
  if(Home)for(const FVector& P:BattleHomeData::Route)ConnectorPoints.Add(P);
  for(int32 Part=0;!Home&&Part<(Krog?6:(Eastside?8:2));Part++)for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It)if(It->ActorHasTag(FName(*FString::Printf(TEXT("%s_%d"),Krog?TEXT("BattleKrog"):(Eastside?TEXT("BattleEastside"):TEXT("BattleConnector")),Part)))){
   for(int32 I=0;I<It->Centerline->GetNumberOfSplinePoints();I++){
    const FVector P=It->Centerline->GetLocationAtSplinePoint(I,ESplineCoordinateSpace::World);
    if(ConnectorPoints.IsEmpty()||!ConnectorPoints.Last().Equals(P,1))ConnectorPoints.Add(P);
   }
  }
  if(ConnectorPoints.Num()<2){Finish(false);return;}
  ConnectorWipeouts=Bike->Ride->Wipeouts;
  if(Eastside){for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();}
 }
 if(ConnectorElapsed==0){
  FlushPressedKeys();Bike->Ride->Gear=Home?2:Eastside?3:1;Bike->Ride->Speed=0;Bike->Ride->Velocity=FVector::ZeroVector;
  Bike->SetActorLocationAndRotation(ConnectorPoints[0]+FVector(0,0,98),(ConnectorPoints[1]-ConnectorPoints[0]).Rotation(),false,nullptr,ETeleportType::TeleportPhysics);
  Bike->Ride->bForceNextFloorCheck=true;ConnectorPrevious=Bike->GetActorLocation();
 }
 if(Eastside&&FMath::FloorToInt(ConnectorElapsed/30)!=FMath::FloorToInt((ConnectorElapsed+Dt)/30))UE_LOG(LogTemp,Display,TEXT("EastsideDrive: leg=%d elapsed=%.0f travelCm=%.0f"),ConnectorLeg,ConnectorElapsed,ConnectorTravel);
 ConnectorElapsed+=Dt;
 if(Bike->Ride->IsMovingOnGround()&&Bike->Ride->Speed>100){TrailGroundSamples++;const AActor* Floor=Bike->Ride->CurrentFloor.HitResult.GetActor();if(Floor&&(Floor->ActorHasTag(TEXT("RidePath"))||Floor->ActorHasTag(TEXT("RideDirt"))))TrailPavedSamples++;}
 if(Krog){
  bool Dark=false;for(TActorIterator<APiedmontDarkZone> It(GetWorld());It;++It)if(It->Contains(Bike->GetActorLocation())){Dark=true;break;}
  TunnelDarkTime=Dark?TunnelDarkTime+Dt:0;
  if(TunnelDarkTime>.4f){if(Bike->bLightsOn)TunnelLitSamples++;else TunnelUnlitSamples++;}
 }
 const FVector Position=Bike->GetActorLocation();
 ConnectorTravel+=FVector::Dist2D(Position,ConnectorPrevious);ConnectorPrevious=Position;
 float Best=TNumericLimits<float>::Max();int32 Segment=0;FVector Closest;
 for(int32 I=0;I<ConnectorPoints.Num()-1;I++){
  FVector A=ConnectorPoints[I],B=ConnectorPoints[I+1],P=Position;A.Z=B.Z=P.Z=0;
  const FVector Q=FMath::ClosestPointOnSegment(P,A,B);const float Distance=FVector::Dist(P,Q);
  if(Distance<Best){Best=Distance;Segment=I;Closest=Q;}
 }
 ConnectorMaxError=FMath::Max(ConnectorMaxError,Best);
 if(ConnectorElapsed>(Eastside?180:30)||Best>180||Bike->Ride->Wipeouts!=ConnectorWipeouts){UE_LOG(LogTemp,Display,TEXT("Connector failure: position=%s endpoint=%s segment=%d distance=%.1f"),*Position.ToString(),*ConnectorPoints.Last().ToString(),Segment,FVector::Dist2D(Position,ConnectorPoints.Last()));Finish(false);return;}
 if(ConnectorElapsed>2&&FVector::Dist2D(Position,ConnectorPoints.Last())<100){
  FlushPressedKeys();ConnectorLeg++;
  if(ConnectorLeg==2){float Expected=0;for(int I=1;I<ConnectorPoints.Num();I++)Expected+=FVector::Dist2D(ConnectorPoints[I-1],ConnectorPoints[I]);Finish(ConnectorTravel>(Home?Expected*1.75f:Krog?47000:(Eastside?200000:4500)));return;}
  Algo::Reverse(ConnectorPoints);ConnectorElapsed=0;return;
 }
 FVector Target=Closest;float Remaining=Home?180:Eastside?350:180;
 for(int32 I=Segment+1;I<ConnectorPoints.Num();I++){
  const FVector Next=ConnectorPoints[I];float Distance=FVector::Dist2D(Target,Next);
  if(Distance>=Remaining){Target=FMath::Lerp(Target,Next,Remaining/Distance);break;}
  Target=Next;Remaining-=Distance;
 }
 const float Error=FMath::FindDeltaAngleDegrees(Bike->GetActorRotation().Yaw,(Target-Position).Rotation().Yaw);
 auto Key=[&](FKey K,bool Down){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 Key(EKeys::W,true);Key(EKeys::A,Error < -2);Key(EKeys::D,Error > 2);
#endif
}
