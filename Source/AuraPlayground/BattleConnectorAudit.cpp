#include "BattleMacController.h"
#include "UnrealClient.h"
#include "HAL/FileManager.h"
#include "BattleBike.h"
#include "BattleRoadCar.h"
#include "BattleHomeData.h"
#include "BattleSpiritData.h"
#include "PiedmontPathSpline.h"
#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "EngineUtils.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Algo/Reverse.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "PiedmontDarkZone.h"

bool PrepareScooterRideAudit(APlayerController* PC,float Dt);
bool ReportScooterRideAudit();
// Opt-in cooked-game physics test. Only each leg's starting fixture is teleported;
// the entire crossing is traversed through ordinary keyboard input and movement.
void ABattleMacController::TickConnectorAudit(float Dt){
#if !UE_BUILD_SHIPPING
 FString HillPath;FParse::Value(FCommandLine::Get(),TEXT("BattleParkHillPath="),HillPath);const bool Hill=!HillPath.IsEmpty();
 const bool Spirit=FParse::Param(FCommandLine::Get(),TEXT("BattleSpiritRouteAudit"));
 const bool Home=Spirit||FParse::Param(FCommandLine::Get(),TEXT("BattleHomeDriveAudit"));
 const bool Krog=FParse::Param(FCommandLine::Get(),TEXT("BattleKrogAudit"));
 const bool Eastside=Hill||Home||Krog||FParse::Param(FCommandLine::Get(),TEXT("BattleEastsideAudit"));
 if(GetWorld()->GetTimeSeconds()<5)return;
 if(!PrepareScooterRideAudit(this,Dt))return;
 auto* Bike=Cast<ABattleBike>(GetPawn());if(!Bike){UE_LOG(LogTemp,Error,TEXT("Connector fixture requires mounted bike; pawn=%s class=%s"),*GetNameSafe(GetPawn()),GetPawn()?*GetPawn()->GetClass()->GetName():TEXT("none"));UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);return;}
 struct FTrafficObservation{TWeakObjectPtr<UWorld> World;int CaptureMask=0;float TraceAge=0,LegTravel=0,PlannedLength=0,SinceSample=0,Nearest=TNumericLimits<float>::Max();int MaxLive=0,MaxMoving=0,NearbySamples=0,Samples=0,MaxPeople=0,MaxWalking=0,PeopleNearby=0;float NearestPerson=TNumericLimits<float>::Max(),MinZ=TNumericLimits<float>::Max(),MaxZ=-TNumericLimits<float>::Max(),MaxGrade=0;};static FTrafficObservation Traffic;
 if(Traffic.World!=GetWorld()){Traffic=FTrafficObservation();Traffic.World=GetWorld();}
 if(Bike->Ride->IsMovingOnGround()&&Bike->Ride->Speed>100){Traffic.MinZ=FMath::Min(Traffic.MinZ,float(Bike->GetActorLocation().Z));Traffic.MaxZ=FMath::Max(Traffic.MaxZ,float(Bike->GetActorLocation().Z));const FVector N=Bike->Ride->CurrentFloor.HitResult.ImpactNormal;Traffic.MaxGrade=FMath::Max(Traffic.MaxGrade,FMath::Abs(float(FVector::DotProduct(Bike->GetActorForwardVector(),N)/FMath::Max(.01,N.Z))));}
 Traffic.SinceSample+=Dt;
 if(Traffic.SinceSample>=.25f){
  Traffic.SinceSample=0;++Traffic.Samples;int Live=0,Moving=0;bool Nearby=false;
  for(TActorIterator<ABattleRoadCar> Car(GetWorld());Car;++Car)if(!Car->IsActorBeingDestroyed()){
   ++Live;if(Car->Speed>1)++Moving;const float Distance=FVector::Dist2D(Car->GetActorLocation(),Bike->GetActorLocation());Traffic.Nearest=FMath::Min(Traffic.Nearest,Distance);Nearby|=Distance<1000;
  }
  int People=0,Walking=0;bool PersonNearby=false;
  for(TActorIterator<APiedmontPedestrian> Person(GetWorld());Person;++Person)if(!Person->IsActorBeingDestroyed()){
   ++People;if(Person->GetVelocity().Size2D()>1)++Walking;const float Distance=FVector::Dist2D(Person->GetActorLocation(),Bike->GetActorLocation());Traffic.NearestPerson=FMath::Min(Traffic.NearestPerson,Distance);PersonNearby|=Distance<1000;
  }
  Traffic.MaxPeople=FMath::Max(Traffic.MaxPeople,People);Traffic.MaxWalking=FMath::Max(Traffic.MaxWalking,Walking);if(PersonNearby)++Traffic.PeopleNearby;
  Traffic.MaxLive=FMath::Max(Traffic.MaxLive,Live);Traffic.MaxMoving=FMath::Max(Traffic.MaxMoving,Moving);if(Nearby)++Traffic.NearbySamples;
 }
 auto Finish=[&](bool Passed){
  Passed=ReportScooterRideAudit()&&Passed;
  UE_LOG(LogTemp,Display,TEXT("HandlingRouteAudit: {\"realistic\":%s,\"elevation_span_cm\":%.2f,\"max_grade\":%.4f}"),Bike->Ride->bRealHandling?TEXT("true"):TEXT("false"),Traffic.MaxZ-Traffic.MinZ,Traffic.MaxGrade);
  UE_LOG(LogTemp,Display,TEXT("CrowdRideAudit: {\"max_live_people\":%d,\"max_walking_people\":%d,\"nearby_samples\":%d,\"nearest_person_cm\":%.2f}"),Traffic.MaxPeople,Traffic.MaxWalking,Traffic.PeopleNearby,Traffic.MaxPeople?Traffic.NearestPerson:-1.f);
  UE_LOG(LogTemp,Display,TEXT("TrafficRideAudit: {\"samples\":%d,\"max_live_cars\":%d,\"max_moving_cars\":%d,\"nearby_samples\":%d,\"nearest_car_cm\":%.2f}"),Traffic.Samples,Traffic.MaxLive,Traffic.MaxMoving,Traffic.NearbySamples,Traffic.MaxLive?Traffic.Nearest:-1.f);
  UE_LOG(LogTemp,Display,TEXT("Connector state: key=%d pedal=%.1f speed=%.1f movement=%d tick=%d"),IsInputKeyDown(EKeys::W),Bike->Ride->Pedal,Bike->Ride->Speed,int32(Bike->Ride->MovementMode),Bike->IsActorTickEnabled());
  UE_LOG(LogTemp,Display,TEXT("TrailGroundAudit: samples=%d paved=%d"),TrailGroundSamples,TrailPavedSamples);
  if(Eastside)Passed=Passed&&TrailGroundSamples>(Hill&&Traffic.PlannedLength<1000?10:100)&&TrailPavedSamples>=TrailGroundSamples*.99f;
  if(Krog){UE_LOG(LogTemp,Display,TEXT("TunnelLightAudit: lit=%d unlit=%d"),TunnelLitSamples,TunnelUnlitSamples);Passed=Passed&&TunnelLitSamples>10&&TunnelUnlitSamples==0;}
  FlushPressedKeys();
  UE_LOG(LogTemp,Display,TEXT("BattleConnectorAudit: {\"passed\":%s,\"completedLegs\":%d,\"travelCm\":%.2f,\"maxCenterlineErrorCm\":%.2f,\"wipeouts\":%d}"),Passed?TEXT("true"):TEXT("false"),ConnectorLeg,ConnectorTravel,ConnectorMaxError,Bike->Ride->Wipeouts-ConnectorWipeouts);
  UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);
 };
 if(ConnectorPoints.IsEmpty()){
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleApproachCandidate"))){
   int Count=0;for(TActorIterator<AActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("KrogApproachReview")))++Count;
   UE_LOG(LogTemp,Display,TEXT("ApproachCandidateAudit: actors=%d world=%s"),Count,*GetWorld()->GetName());
   if(Count!=1){Finish(false);return;}
  }

  float MinimumLength=0;FParse::Value(FCommandLine::Get(),TEXT("BattleRouteMinimumLength="),MinimumLength);
  if(Hill)for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It)if(It->OsmWayId==HillPath&&It->Centerline->GetSplineLength()>=MinimumLength){UE_LOG(LogTemp,Display,TEXT("LakeRouteSelection: id=%s length_cm=%.2f points=%d"),*HillPath,It->Centerline->GetSplineLength(),It->Centerline->GetNumberOfSplinePoints());for(int I=0;I<It->Centerline->GetNumberOfSplinePoints();I++)ConnectorPoints.Add(It->Centerline->GetLocationAtSplinePoint(I,ESplineCoordinateSpace::World));break;}
  if(Spirit){for(const FVector& P:BattleSpiritData::Ride)ConnectorPoints.Add(P);}
  else if(Home)for(const FVector& P:BattleHomeData::Route)ConnectorPoints.Add(P);
  for(int32 Part=0;!Hill&&!Home&&Part<(Krog?6:(Eastside?8:2));Part++)for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It)if(It->ActorHasTag(FName(*FString::Printf(TEXT("%s_%d"),Krog?TEXT("BattleKrog"):(Eastside?TEXT("BattleEastside"):TEXT("BattleConnector")),Part)))){
   for(int32 I=0;I<It->Centerline->GetNumberOfSplinePoints();I++){
    const FVector P=It->Centerline->GetLocationAtSplinePoint(I,ESplineCoordinateSpace::World);
    if(ConnectorPoints.IsEmpty()||!ConnectorPoints.Last().Equals(P,1))ConnectorPoints.Add(P);
   }
  }
  if(ConnectorPoints.Num()<2){Finish(false);return;}
  ConnectorWipeouts=Bike->Ride->Wipeouts;
  if(Eastside&&!FParse::Param(FCommandLine::Get(),TEXT("BattleKeepCrowds"))){for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();}
 }
 if(ConnectorElapsed==0){
  Traffic.CaptureMask=0;Traffic.LegTravel=0;Traffic.PlannedLength=0;for(int I=1;I<ConnectorPoints.Num();I++)Traffic.PlannedLength+=FVector::Dist2D(ConnectorPoints[I-1],ConnectorPoints[I]);
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleRealHandlingRoute")))Bike->Ride->bRealHandling=true;
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
 Traffic.LegTravel+=FVector::Dist2D(Position,ConnectorPrevious);ConnectorTravel+=FVector::Dist2D(Position,ConnectorPrevious);ConnectorPrevious=Position;
 float Best=TNumericLimits<float>::Max();int32 Segment=0;FVector Closest;
 for(int32 I=0;I<ConnectorPoints.Num()-1;I++){
  FVector A=ConnectorPoints[I],B=ConnectorPoints[I+1],P=Position;A.Z=B.Z=P.Z=0;
  const FVector Q=FMath::ClosestPointOnSegment(P,A,B);const float Distance=FVector::Dist(P,Q);
  if(Distance<Best){Best=Distance;Segment=I;Closest=Q;}
 }
 ConnectorMaxError=FMath::Max(ConnectorMaxError,Best);
 FString CaptureDir;if(FParse::Value(FCommandLine::Get(),TEXT("BattleRouteCaptureDir="),CaptureDir)){
  const int Stage=Traffic.LegTravel>Traffic.PlannedLength*.5f?1:0;const int Bit=1<<Stage;
  if(ConnectorElapsed>2&&!(Traffic.CaptureMask&Bit)){Traffic.CaptureMask|=Bit;IFileManager::Get().MakeDirectory(*CaptureDir,true);FScreenshotRequest::RequestScreenshot(CaptureDir/FString::Printf(TEXT("leg%d-view%d.png"),ConnectorLeg,Stage),true,false);UE_LOG(LogTemp,Display,TEXT("LakeRouteCapture: leg=%d stage=%d position=%s"),ConnectorLeg,Stage,*Position.ToString());}
 }

 if(ConnectorElapsed>(Eastside?180:30)||Best>180||Bike->Ride->Wipeouts!=ConnectorWipeouts){UE_LOG(LogTemp,Display,TEXT("Connector failure: position=%s endpoint=%s segment=%d distance=%.1f"),*Position.ToString(),*ConnectorPoints.Last().ToString(),Segment,FVector::Dist2D(Position,ConnectorPoints.Last()));Finish(false);return;}
 // A short connector can finish before two seconds; a closed loop must be ridden before its shared endpoint counts.
 const bool Arrived=Hill ? Traffic.LegTravel>=Traffic.PlannedLength*.85f&&FVector::Dist2D(Position,ConnectorPoints.Last())<FMath::Clamp(Traffic.PlannedLength*.05f,12.f,100.f) : ConnectorElapsed>2&&FVector::Dist2D(Position,ConnectorPoints.Last())<100;
 if(Arrived){
  FlushPressedKeys();ConnectorLeg++;
  if(ConnectorLeg==2){float Expected=0;for(int I=1;I<ConnectorPoints.Num();I++)Expected+=FVector::Dist2D(ConnectorPoints[I-1],ConnectorPoints[I]);Finish(ConnectorTravel>((Home||Hill)?Expected*1.75f:Krog?47000:(Eastside?200000:4500)));return;}
  Algo::Reverse(ConnectorPoints);ConnectorElapsed=0;return;
 }
 FVector Target=Closest;float Remaining=Bike->Ride->bRealHandling?180:Home?180:Eastside?350:180;
 float LookaheadOverride=0;if(FParse::Value(FCommandLine::Get(),TEXT("BattleRouteLookahead="),LookaheadOverride))Remaining=FMath::Clamp(LookaheadOverride,60.f,500.f);
 for(int32 I=Segment+1;I<ConnectorPoints.Num();I++){
  const FVector Next=ConnectorPoints[I];float Distance=FVector::Dist2D(Target,Next);
  if(Distance>=Remaining){Target=FMath::Lerp(Target,Next,Remaining/Distance);break;}
  Target=Next;Remaining-=Distance;
 }
 const float Error=FMath::FindDeltaAngleDegrees(Bike->GetActorRotation().Yaw,(Target-Position).Rotation().Yaw);
 auto Key=[&](FKey K,bool Down){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 FString YieldReason;
 const bool BrakeForBends=Bike->Ride->bRealHandling||FParse::Param(FCommandLine::Get(),TEXT("BattleRouteBrakeForBends"));
 bool Yield=BrakeForBends&&FMath::Abs(Error)>10&&Bike->Ride->Speed>450;
 if(BrakeForBends){
  // Brake before a mapped bend, using remaining distance and a conservative corner radius.
  const FVector Forward=Bike->GetActorForwardVector().GetSafeNormal2D();
  const float StopLook=Bike->Ride->Speed*Bike->Ride->Speed/1100.f+500.f;
  float RouteRemaining=FVector::Dist2D(Closest,ConnectorPoints[Segment+1]);
  for(int I=Segment+1;I+1<ConnectorPoints.Num();I++)RouteRemaining+=FVector::Dist2D(ConnectorPoints[I],ConnectorPoints[I+1]);
  for(TActorIterator<ABattleRoadCar> Car(GetWorld());Car;++Car){const FVector Offset=Car->GetActorLocation()-Position;const float Ahead=FVector::DotProduct(Offset,Forward);const float Side=FMath::Abs(Offset.X*Forward.Y-Offset.Y*Forward.X);if(Ahead>0&&Ahead<StopLook&&Side<350&&FMath::Abs(Offset.Z)<180){
   // A stopped car beyond the destination must not prevent finishing a clear route.
   // Sweep the real bike capsule through the remaining endpoint plus a safety margin;
   // keep ordinary yielding if any part of this car obstructs that movement.
   if(Car->Speed<1&&RouteRemaining<StopLook&&Ahead>RouteRemaining&&Car->Collision->GetCollisionEnabled()!=ECollisionEnabled::NoCollision){
    FHitResult EndHit;FVector End=ConnectorPoints.Last();End.Z=Position.Z;End+=(End-Position).GetSafeNormal2D()*75.f;
    if(!Car->Collision->SweepComponent(EndHit,Position,End,Bike->GetActorQuat(),FCollisionShape::MakeCapsule(Bike->Capsule->GetScaledCapsuleRadius(),Bike->Capsule->GetScaledCapsuleHalfHeight()))){
     static bool LoggedEndClear=false;if(!LoggedEndClear){UE_LOG(LogTemp,Display,TEXT("RouteEndpointClear: remaining=%.1f car_ahead=%.1f capsule_sweep_clear=1"),RouteRemaining,Ahead);LoggedEndClear=true;}continue;
    }
   }
   if(Car->Speed<1&&Car->bObstacleAhead&&Car->LastObstacle==Bike->GetName()&&Car->Collision->GetCollisionEnabled()!=ECollisionEnabled::NoCollision){
    FHitResult ClearanceHit;
    const bool Blocked=Car->Collision->SweepComponent(ClearanceHit,Position,Position+Forward*StopLook,Bike->GetActorQuat(),FCollisionShape::MakeCapsule(Bike->Capsule->GetScaledCapsuleRadius(),Bike->Capsule->GetScaledCapsuleHalfHeight()));
    if(!Blocked){static bool Logged=false;if(!Logged){UE_LOG(LogTemp,Display,TEXT("RouteYieldPriority: stopped car yields to bike; actual capsule path clear at %s"),*Position.ToString());Logged=true;}continue;}
   }
   Yield=true;YieldReason=FString::Printf(TEXT("car=%s ahead=%.1f side=%.1f speed=%.1f crossing=%d obstacle=%s"),*Car->GetName(),Ahead,Side,Car->Speed,Car->bWaitingForCrossing,*Car->LastObstacle);}}
  float DistanceToBend=FVector::Dist2D(Closest,ConnectorPoints[Segment+1]);
  for(int I=Segment+1;I+1<ConnectorPoints.Num()&&DistanceToBend<2000;I++){
   const FVector In=(ConnectorPoints[I]-ConnectorPoints[I-1]).GetSafeNormal2D(),Out=(ConnectorPoints[I+1]-ConnectorPoints[I]).GetSafeNormal2D();
   const float Angle=FMath::Acos(FMath::Clamp(float(FVector::DotProduct(In,Out)),-1.f,1.f));
   if(Angle>.15f){const float Radius=120.f/FMath::Max(.1f,2.f*FMath::Sin(Angle*.5f));const float CornerSpeed=FMath::Sqrt(400.f*Radius);const float SafeSpeed=FMath::Sqrt(CornerSpeed*CornerSpeed+2.f*650.f*FMath::Max(0.f,DistanceToBend-180.f));if(Bike->Ride->Speed>SafeSpeed)Yield=true;}
   DistanceToBend+=FVector::Dist2D(ConnectorPoints[I],ConnectorPoints[I+1]);
  }
 }
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleKeepCrowds"))){
  const FVector Forward=Bike->GetActorForwardVector().GetSafeNormal2D();
  const float LookAhead=FMath::Max(420.f,Bike->Ride->Speed*Bike->Ride->Speed/1000.f+Bike->Ride->Speed*.5f+200.f);
  for(TActorIterator<APiedmontPedestrian> Person(GetWorld());Person;++Person){
   const FVector Offset=Person->GetActorLocation()-Position;const float Ahead=FVector::DotProduct(Offset,Forward);
   const float SignedSide=Offset.X*Forward.Y-Offset.Y*Forward.X;
   const float Side=FMath::Abs(SignedSide);
   const FVector Velocity=Person->GetVelocity();
   const float Arrival=FMath::Clamp(Ahead/FMath::Max(100.f,Bike->Ride->Speed),0.f,2.f);
   const float FutureSide=SignedSide+(Velocity.X*Forward.Y-Velocity.Y*Forward.X)*Arrival;
   const float ClosestSide=SignedSide*FutureSide<=0?0.f:FMath::Min(Side,FMath::Abs(FutureSide));
   // Standing pedestrians collide through their capsule. Hidden visual components
   // can have oversized bounds, so actor-wide rendering bounds are not a collision corridor.
   const bool LowPose=Person->bIncidentPosing||Person->bDead||Person->KnockdownPhase>0;
   const float Clearance=LowPose?200.f:FMath::Max(100.f,Bike->Capsule->GetScaledCapsuleRadius()+Person->GetCapsuleComponent()->GetScaledCapsuleRadius()+40.f);
   if(Ahead>0&&Ahead<LookAhead&&ClosestSide<Clearance&&FMath::Abs(Offset.Z)<160){Yield=true;YieldReason=FString::Printf(TEXT("person=%s ahead=%.1f side=%.1f speed=%.1f pause=%.1f posed=%d dead=%d destination=%d"),*Person->GetName(),Ahead,Side,Person->GetVelocity().Size2D(),Person->PauseRemaining,Person->bIncidentPosing,Person->bDead,Person->bHasDestination);break;}
  }
 }
 static float YieldAge=0;
 if(Yield&&Bike->Ride->Speed<10){YieldAge+=Dt;if(YieldAge>=5){UE_LOG(LogTemp,Display,TEXT("RouteYieldDiagnostic: leg=%d elapsed=%.1f position=%s reason=%s"),ConnectorLeg,ConnectorElapsed,*Position.ToString(),*YieldReason);
  for(TActorIterator<ABattleRoadCar> Car(GetWorld());Car;++Car)if(FVector::Dist2D(Car->GetActorLocation(),Position)<2500)UE_LOG(LogTemp,Display,TEXT("RouteQueueDiagnostic: car=%s position=%s speed=%.1f finished=%d crossing=%d grounded=%d obstacle=%s"),*Car->GetName(),*Car->GetActorLocation().ToString(),Car->Speed,Car->bRouteFinished,Car->bWaitingForCrossing,Car->bGrounded,*Car->LastObstacle);
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleRouteYieldProbe"))&&YieldReason.StartsWith(TEXT("car="))){Finish(false);return;}
  YieldAge=0;}}else YieldAge=0;
 Traffic.TraceAge+=Dt;if(FParse::Param(FCommandLine::Get(),TEXT("BattleRouteTrace"))&&Traffic.TraceAge>.25f){Traffic.TraceAge=0;UE_LOG(LogTemp,Display,TEXT("LakeDriveTrace: dt=%.4f leg=%d segment=%d error=%.2f center=%.2f speed=%.2f steer=%.2f yawrate=%.2f brake=%d"),Dt,ConnectorLeg,Segment,Error,Best,Bike->Ride->Speed,Bike->Ride->SmoothedSteer,Bike->Ride->TurnRateDegrees,Yield);}
 Key(EKeys::SpaceBar,Yield);Key(EKeys::W,!Yield);Key(EKeys::A,Error < -2);Key(EKeys::D,Error > 2);
#endif
}
