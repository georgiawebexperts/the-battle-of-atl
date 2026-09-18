#include "BattleBike.h"
#include "BattleRoadCar.h"
#include "BattleRoadCrossing.h"
#include "BattleKrogRiderRoute.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
void TickBattleKrogRiderAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TArray<TWeakObjectPtr<ABattleRoadCar>> Cars;TWeakObjectPtr<ABattleRoadCrossing> Gate;float Clock=0,Hold=0,Travel=0,MaxError=0;FVector Last;int Phase=0,Wipeouts=0,Ground=0,Paved=0;bool Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 auto Key=[&](FKey K,bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto* Bike=Cast<ABattleBike>(PC->GetPawn());
 auto Finish=[&](bool Passed,const TCHAR* Reason){
  // A failure used to say only "rider missing or timed out" or "crashed", which is not enough to
  // act on: two runs died parked at the end of the route (speed 0.0) and two died moving. Name
  // the rider's state and the nearest car on the way out so the next run says which one it was.
  if(!Passed&&Bike){
   const FVector P=Bike->GetActorLocation();float BestCar=-1;FString CarName=TEXT("none");
   for(TActorIterator<ABattleRoadCar> I(PC->GetWorld());I;++I){const float D=FVector::Dist2D(I->GetActorLocation(),P);if(BestCar<0||D<BestCar){BestCar=D;CarName=I->GetName();}}
   UE_LOG(LogTemp,Display,TEXT("KrogRiderAuditFail: {\"phase\":%d,\"x\":%.1f,\"y\":%.1f,\"z\":%.1f,\"speed\":%.1f,\"health\":%.1f,\"wipeouts\":%d,\"on_ground\":%d,\"nearest_car\":\"%s\",\"nearest_car_cm\":%.1f}"),S.Phase,P.X,P.Y,P.Z,Bike->Ride->Speed,Bike->RiderHealth,Bike->Ride->Wipeouts,Bike->Ride->IsMovingOnGround()?1:0,*CarName,BestCar);
  }
  PC->FlushPressedKeys();S.Done=true;UE_LOG(LogTemp,Display,TEXT("KrogRiderAudit: {\"passed\":%s,\"reason\":\"%s\",\"travel_cm\":%.2f,\"max_error_cm\":%.2f,\"held_seconds\":%.2f,\"ground_samples\":%d,\"paved_samples\":%d}"),Passed?TEXT("true"):TEXT("false"),Reason,S.Travel,S.MaxError,S.Hold,S.Ground,S.Paved);PC->ConsoleCommand(TEXT("quit"));};
 // 50 s was not enough: the ride itself finishes (travel 1478 cm against a 1444 cm route,
 // 83 cm max centreline error, every ground sample on RidePath pavement), but the closing
 // wait for the two cars to finish their own route runs past the clock, so the audit reported
 // "Rider missing or timed out" on a clean Krog leg. The Monroe occupancy sibling allows 90 s
 // for the same car-completion wait.
 if(!Bike||S.Clock>150){Finish(false,TEXT("Rider missing or timed out"));return;}
 if(S.Phase==0){
  for(TActorIterator<ABattleRoadCrossing> I(PC->GetWorld());I;++I)if(I->ActorHasTag(TEXT("KrogCrossingReview")))S.Gate=*I;
  for(TActorIterator<ABattleRoadCar> I(PC->GetWorld());I;++I)if(I->ActorHasTag(TEXT("KrogCarLaneReview")))S.Cars.Add(*I);
  if(!S.Gate.IsValid()||S.Cars.Num()!=2){Finish(false,TEXT("Crossing fixtures missing"));return;}
  const FVector Start=BattleKrogRiderRoute::Points[0];FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(KrogRiderStart),true,Bike);
  if(!PC->GetWorld()->LineTraceSingleByChannel(Hit,Start+FVector(0,0,150),Start-FVector(0,0,150),ECC_Visibility,Q)){Finish(false,TEXT("Bike floor missing"));return;}
  PC->FlushPressedKeys();Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->Ride->Gear=1;
  Bike->SetActorLocationAndRotation(Hit.ImpactPoint+FVector(0,0,98),(BattleKrogRiderRoute::Points[1]-Start).Rotation(),false,nullptr,ETeleportType::TeleportPhysics);
  Bike->Ride->SetMovementMode(MOVE_Walking);Bike->Ride->bForceNextFloorCheck=true;S.Wipeouts=Bike->Ride->Wipeouts;
  S.Gate->bAutoCycle=false;S.Gate->bVehicleGreen=true;S.Gate->bVehicleAmber=false;
  for(auto Car:S.Cars)if(!Car->StartRoute()){Finish(false,TEXT("Car start failed"));return;}
  S.Phase=1;return;
 }
 if(Bike->Ride->Wipeouts!=S.Wipeouts||Bike->RiderHealth<=0){Finish(false,TEXT("Unexpected rider crash or death"));return;}
 for(auto Car:S.Cars)if(!Car.IsValid()||!Car->bGrounded){Finish(false,TEXT("Car lost support"));return;}
 if(S.Phase==1){
  bool Held=true;
  for(auto Car:S.Cars){if(Car->DistanceTravelled>Car->Crossings[0].StopDistance+.5){Finish(false,TEXT("Car crossed occupied rider stop"));return;}Held&=Car->bWaitingForCrossing&&Car->Speed<1&&Car->Crossings[0].StopDistance-Car->DistanceTravelled<10;}
  S.Hold=Held?S.Hold+Dt:0;if(S.Hold>=2){S.Phase=2;S.Last=Bike->GetActorLocation();}return;
 }
 if(S.Phase==2){
  const FVector P=Bike->GetActorLocation();S.Travel+=FVector::Dist2D(P,S.Last);S.Last=P;
  if(Bike->Ride->IsMovingOnGround()){++S.Ground;const AActor* Floor=Bike->Ride->CurrentFloor.HitResult.GetActor();if(Floor&&Floor->ActorHasTag(TEXT("RidePath")))++S.Paved;}
  else {Finish(false,TEXT("Rider lost ground support"));return;}
  const auto& Points=BattleKrogRiderRoute::Points;constexpr int Count=UE_ARRAY_COUNT(BattleKrogRiderRoute::Points);float Best=TNumericLimits<float>::Max();int Segment=0;FVector Closest;
  for(int I=0;I<Count-1;++I){FVector A=Points[I],B=Points[I+1],Flat=P;A.Z=B.Z=Flat.Z=0;const FVector V=FMath::ClosestPointOnSegment(Flat,A,B);const float D=FVector::Dist2D(P,V);if(D<Best){Best=D;Segment=I;Closest=V;}}
  S.MaxError=FMath::Max(S.MaxError,Best);if(Best>180){Finish(false,TEXT("Rider left route"));return;}
  // Flushing the pressed keys does not stop the bike: four runs parked a rider that was really
  // still rolling, found 10 m past the last route point and a metre above it, where it either
  // piled into the scenery (health 100 -> 80, one wipeout) or sat in the crossing so the two cars
  // never cleared it and the audit died on the clock. Brake at the end of the route instead.
  if(FVector::Dist2D(P,Points[Count-1])<90){Key(EKeys::W,false);Key(EKeys::A,false);Key(EKeys::D,false);PC->FlushPressedKeys();Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;S.Phase=3;return;}
  FVector Target=Closest;float Look=150;
  for(int I=Segment+1;I<Count;++I){float D=FVector::Dist2D(Target,Points[I]);if(D>=Look){Target=FMath::Lerp(Target,Points[I],Look/D);break;}Target=Points[I];Look-=D;}
  float Angle=FMath::FindDeltaAngleDegrees(Bike->GetActorRotation().Yaw,(Target-P).Rotation().Yaw);Key(EKeys::W,true);Key(EKeys::A,Angle< -2);Key(EKeys::D,Angle>2);
 }else{
  bool Finished=true;for(auto Car:S.Cars)Finished&=Car->bRouteFinished&&Car->Speed==0;
  if(Finished)Finish(S.Travel>1100&&S.Ground>100&&S.Paved>=S.Ground*.99f,TEXT("Cars held for mounted rider; W/A/D ride cleared crossing and cars completed"));
 }
#endif
}
