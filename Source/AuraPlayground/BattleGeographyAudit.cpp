#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleQuest.h"
#include "BattleRouteAnchors.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Kismet/KismetSystemLibrary.h"

// Exercise the saved geographic frame and swimmer/bank separation in a
// cooked game. This fixture is never active during normal play or Shipping.
void ABattleMacController::TickGeographyAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5)return;
 auto* Foot=Cast<ABattleRider>(GetPawn());auto* Bike=Cast<ABattleBike>(GetPawn());if(Foot)Bike=Foot->ParkedBike.Get();if(!Bike)return;
 static FVector ParkedAt;
 APiedmontWaterHazard* Water=nullptr;
 TActorIterator<APiedmontWaterHazard> WaterIt(GetWorld());if(WaterIt)Water=*WaterIt;
 auto Finish=[&](bool Passed,const TCHAR* Reason){
  UE_LOG(LogTemp,Display,TEXT("BattleGeographyAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\",\"recoverySeconds\":%.3f,\"wipeouts\":%d}"),Passed?TEXT("true"):TEXT("false"),GeographyPhase,Reason,GeographyClock,Bike->Ride->Wipeouts-GeographyInitialWipeouts);
  UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);
 };
 if(!Water){Finish(false,TEXT("Missing lake hazard"));return;}
 if(GeographyPhase==0){
  // No `break` inside the loop: clang rejects that as
  // -Wunreachable-code-loop-increment and this project builds with -Werror, so
  // the loop form only compiled while unity grouping kept this file in a blob
  // that happened not to be recompiled. The iterator is asked once instead.
  FVector Start=Bike->GetActorLocation();{TActorIterator<APlayerStart> It(GetWorld());if(It)Start=It->GetActorLocation();}
  const bool Anchors=GetWorld()->GetWorldSettings()->ActorHasTag(TEXT("BattleGeography_ESU_v1"))&&FMath::Abs(Start.X+16926.2)<1&&FMath::Abs(Start.Y+5098)<1&&BattleRouteAnchors::ParkExitY>10729;
  const FVector2D North=ABattleQuest::RadarOffset(FVector2D(0,-100),1),East=ABattleQuest::RadarOffset(FVector2D(100,0),1);
  const bool Compass=North.Y<0&&North.X==0&&East.X>0&&East.Y==0&&FVector::DotProduct(FRotationMatrix(FRotator::ZeroRotator).GetUnitAxis(EAxis::Y),FVector(0,-1,0))<-.99;
  FVector Island=FVector::ZeroVector;
  for(FVector P:Water->IslandPolygon)Island+=P;
  if(!Water->IslandPolygon.IsEmpty())Island/=Water->IslandPolygon.Num();
  const bool IslandDry=!Water->IslandPolygon.IsEmpty()&&!Water->ContainsBike(Water->GetActorTransform().TransformPosition(Island));
  if(!Anchors||!Compass||!IslandDry){Finish(false,TEXT("Anchor, compass or island transform"));return;}
  FBox Bounds(ForceInit);for(FVector P:Water->Polygon)Bounds+=P;
  bool Found=false;FVector Drop;
  for(int Y=1;Y<10&&!Found;Y++)for(int X=1;X<10&&!Found;X++){
   FVector P=Water->GetActorTransform().TransformPosition(FVector(FMath::Lerp(Bounds.Min.X,Bounds.Max.X,X/10.f),FMath::Lerp(Bounds.Min.Y,Bounds.Max.Y,Y/10.f),0))+FVector(0,0,98);
   if(!Water->ContainsBike(P))continue;
   FHitResult Ground;FCollisionQueryParams Q(SCENE_QUERY_STAT(GeographyWater),true,Bike);
   if(GetWorld()->LineTraceSingleByChannel(Ground,P+FVector(0,0,2000),P-FVector(0,0,2000),ECC_Visibility,Q)&&Ground.GetActor()&&!Ground.GetActor()->ActorHasTag(TEXT("RideBridge"))&&Ground.ImpactPoint.Z<Water->GetActorLocation().Z+10){Drop=P;Found=true;}
  }
  if(!Found){Finish(false,TEXT("No open lake fixture"));return;}
  FlushPressedKeys();GeographyInitialWipeouts=Bike->Ride->Wipeouts;
  Bike->SetActorLocation(Drop,false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->Ride->bForceNextFloorCheck=true;
  GeographyPhase=1;GeographyClock=0;return;
 }
 GeographyClock+=Dt;
 if(GeographyPhase==1){
  if(Foot&&Foot->bSwimming&&Bike->bParked){ParkedAt=Bike->GetActorLocation();GeographyPhase=2;GeographyClock=0;}
  else if(GeographyClock>3)Finish(false,TEXT("Lake entry did not separate swimmer and bike"));
  return;
 }
 if(GeographyClock>2){
  FVector BankProbe=Bike->GetActorLocation();BankProbe.Z=Water->GetActorLocation().Z+1;
  const bool Dry=!Water->ContainsBike(BankProbe),Fixed=FVector::Distance(ParkedAt,Bike->GetActorLocation())<1;
  Finish(Dry&&Fixed&&Foot&&Foot->bSwimming&&Bike->bParked&&Bike->Ride->Wipeouts==GeographyInitialWipeouts+1,TEXT("Saved frame, island and persistent swimmer/bank separation checked"));
 }

#endif
}
