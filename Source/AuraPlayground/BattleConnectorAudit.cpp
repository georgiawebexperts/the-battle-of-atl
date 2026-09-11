#include "BattleMacController.h"
#include "BattleBike.h"
#include "PiedmontPathSpline.h"
#include "Components/SplineComponent.h"
#include "EngineUtils.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Algo/Reverse.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"

// Opt-in cooked-game physics test. Only each leg's starting fixture is teleported;
// the entire crossing is traversed through ordinary keyboard input and movement.
void ABattleMacController::TickConnectorAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5)return;
 auto* Bike=Cast<ABattleBike>(GetPawn());if(!Bike)return;
 auto Finish=[&](bool Passed){
  UE_LOG(LogTemp,Display,TEXT("Connector state: key=%d pedal=%.1f speed=%.1f movement=%d tick=%d"),IsInputKeyDown(EKeys::W),Bike->Ride->Pedal,Bike->Ride->Speed,int32(Bike->Ride->MovementMode),Bike->IsActorTickEnabled());
  FlushPressedKeys();
  UE_LOG(LogTemp,Display,TEXT("BattleConnectorAudit: {\"passed\":%s,\"completedLegs\":%d,\"travelCm\":%.2f,\"maxCenterlineErrorCm\":%.2f,\"wipeouts\":%d}"),Passed?TEXT("true"):TEXT("false"),ConnectorLeg,ConnectorTravel,ConnectorMaxError,Bike->Ride->Wipeouts-ConnectorWipeouts);
  UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);
 };
 if(ConnectorPoints.IsEmpty()){
  for(int32 Part=0;Part<2;Part++)for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It)if(It->ActorHasTag(FName(*FString::Printf(TEXT("BattleConnector_%d"),Part)))){
   for(int32 I=0;I<It->Centerline->GetNumberOfSplinePoints();I++){
    const FVector P=It->Centerline->GetLocationAtSplinePoint(I,ESplineCoordinateSpace::World);
    if(ConnectorPoints.IsEmpty()||!ConnectorPoints.Last().Equals(P,1))ConnectorPoints.Add(P);
   }
  }
  if(ConnectorPoints.Num()<2){Finish(false);return;}
  ConnectorWipeouts=Bike->Ride->Wipeouts;
 }
 if(ConnectorElapsed==0){
  FlushPressedKeys();Bike->Ride->Gear=1;Bike->Ride->Speed=0;Bike->Ride->Velocity=FVector::ZeroVector;
  Bike->SetActorLocationAndRotation(ConnectorPoints[0]+FVector(0,0,98),(ConnectorPoints[1]-ConnectorPoints[0]).Rotation(),false,nullptr,ETeleportType::TeleportPhysics);
  Bike->Ride->bForceNextFloorCheck=true;ConnectorPrevious=Bike->GetActorLocation();
 }
 ConnectorElapsed+=Dt;
 const FVector Position=Bike->GetActorLocation();
 ConnectorTravel+=FVector::Dist2D(Position,ConnectorPrevious);ConnectorPrevious=Position;
 float Best=TNumericLimits<float>::Max();int32 Segment=0;FVector Closest;
 for(int32 I=0;I<ConnectorPoints.Num()-1;I++){
  FVector A=ConnectorPoints[I],B=ConnectorPoints[I+1],P=Position;A.Z=B.Z=P.Z=0;
  const FVector Q=FMath::ClosestPointOnSegment(P,A,B);const float Distance=FVector::Dist(P,Q);
  if(Distance<Best){Best=Distance;Segment=I;Closest=Q;}
 }
 ConnectorMaxError=FMath::Max(ConnectorMaxError,Best);
 if(ConnectorElapsed>30||Best>180||Bike->Ride->Wipeouts!=ConnectorWipeouts){Finish(false);return;}
 if(ConnectorElapsed>2&&FVector::Dist2D(Position,ConnectorPoints.Last())<100){
  FlushPressedKeys();ConnectorLeg++;
  if(ConnectorLeg==2){Finish(ConnectorTravel>4500);return;}
  Algo::Reverse(ConnectorPoints);ConnectorElapsed=0;return;
 }
 FVector Target=Closest;float Remaining=180;
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
