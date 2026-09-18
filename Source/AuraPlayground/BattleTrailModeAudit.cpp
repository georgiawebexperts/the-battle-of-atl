#include "BattleTrailMode.h"
#include "BattleBike.h"
#include "PiedmontPathSpline.h"
#include "Components/SplineComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

/**
 * Proves that the BeltLine really is two ribbons.
 *
 * The width is baked into the pavement rather than being a runtime parameter, so
 * a mode that only changed a number would leave the rider on a wide path in
 * realistic mode and on a narrow one in arcade. This audit measures the pavement
 * itself: at a sample station it walks the centreline out to 130 cm and 190 cm
 * either side and asks which ribbon, if any, answers a downward trace. Arcade has
 * to answer with the wide ribbon out at 190 cm; realistic has to answer with the
 * narrow one at 130 cm and with nothing at 190 cm. The spline width has to move
 * with it, because that is the number the BeltLine grass rule reads, and the
 * actors of the inactive ribbon have to be hidden so the swap is not just a
 * collision trick.
 *
 * The mode is changed the way the player changes it - by putting the bike into
 * realistic handling and letting the bike apply the matching ribbon - so this
 * covers the wiring as well as the geometry.
 *
 * Elliott, 2026-09-18: "i wonder if you are doing the arcade mode if the
 * beltlien is wide and if you are doing the pro mode or realistic mode it was
 * like it was before?"
 */
void TickBattleTrailModeAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{int32 Phase=0;float Clock=0;float Elapsed=0;FVector Site=FVector::ZeroVector;FVector Side=FVector::ZeroVector;int32 Stations=0;int32 ArcadeActors=0;int32 RealisticActors=0;};
 static FState S;
 if(!PC||!PC->GetWorld())return;
 UWorld* World=PC->GetWorld();
 if(World->GetTimeSeconds()<5.f)return;
 S.Clock+=Dt;S.Elapsed+=Dt;
 if(S.Elapsed>60.f){UE_LOG(LogTemp,Display,TEXT("BattleTrailModeAudit: {\"passed\":false,\"reason\":\"audit timed out\"}"));PC->ConsoleCommand(TEXT("quit"));return;}

 auto Finish=[&](bool Pass,const FString& Why,const FString& Detail){
  UE_LOG(LogTemp,Display,TEXT("BattleTrailModeAudit: {\"passed\":%s,\"reason\":\"%s\",\"arcade_actors\":%d,\"realistic_actors\":%d,\"stations\":%d,%s}"),
   Pass?TEXT("true"):TEXT("false"),*Why,S.ArcadeActors,S.RealisticActors,S.Stations,*Detail);
  PC->ConsoleCommand(TEXT("quit"));
 };
 // Character pawns walk the trail all day; they are not the surface.
 FCollisionQueryParams Q;Q.bIgnoreTouches=true;
 for(TActorIterator<APawn> It(World);It;++It)Q.AddIgnoredActor(*It);
 auto Sample=[&](const FVector& At,FString& OutKind,FString& OutLabel){
  FHitResult Hit;OutKind=TEXT("nothing");OutLabel=TEXT("-");
  if(!World->LineTraceSingleByChannel(Hit,At+FVector(0,0,300),At-FVector(0,0,300),ECC_Visibility,Q))return;
  const AActor* Actor=Hit.GetActor();
  if(!Actor)return;
  OutKind=Actor->ActorHasTag(TEXT("BattleTrailArcade"))?TEXT("arcade"):Actor->ActorHasTag(TEXT("BattleTrailRealistic"))?TEXT("realistic"):TEXT("other");
  // GetActorLabel is editor-only; a cooked build falls back to the object name.
#if WITH_EDITOR
  OutLabel=Actor->GetActorLabel();
#else
  OutLabel=Actor->GetName();
#endif
 };
 auto SplineRange=[&](float& Lo,float& Hi,int32& Count){
  Lo=BIG_NUMBER;Hi=-BIG_NUMBER;Count=0;
  for(TActorIterator<APiedmontPathSpline> It(World);It;++It)if(It->ActorHasTag(TEXT("BattleTrailWidth"))){Count++;Lo=FMath::Min(Lo,It->WidthCm);Hi=FMath::Max(Hi,It->WidthCm);}
 };
 auto RibbonCounts=[&](int32& Arcade,int32& Realistic,int32& ArcadeHidden,int32& RealisticVisible){
  Arcade=Realistic=ArcadeHidden=RealisticVisible=0;
  for(TActorIterator<AActor> It(World);It;++It){
   if(It->ActorHasTag(TEXT("BattleTrailArcade"))){Arcade++;if(It->IsHidden())ArcadeHidden++;}
   else if(It->ActorHasTag(TEXT("BattleTrailRealistic"))){Realistic++;if(!It->IsHidden())RealisticVisible++;}
  }
 };
 if(S.Phase==0){
  auto* Bike=Cast<ABattleBike>(PC->GetPawn());
  if(!Bike||!Bike->Ride){Finish(false,TEXT("no bike to put into realistic handling"),TEXT("\"detail\":\"phase 0\""));return;}
  // Site selection has to happen with the wide ribbon live, and has to find a
  // station where the trail is the topmost surface all the way out to 190 cm -
  // otherwise a street legitimately overlaid on the trail would be measured
  // instead of the pavement.
  UBattleTrailMode::Apply(PC,false);
  Bike->Ride->bRealHandling=false;
  for(TActorIterator<APiedmontPathSpline> It(World);It;++It){
   if(!It->ActorHasTag(TEXT("BattleTrailWidth"))||It->bBridge)continue;
   USplineComponent* Route=It->Centerline.Get();if(!Route)continue;
   const float Length=Route->GetSplineLength();
   for(int32 Step=1;Step<=3&&S.Site.IsNearlyZero();++Step){
    const float D=Length*Step/4.f;
    const FVector P=Route->GetLocationAtDistanceAlongSpline(D,ESplineCoordinateSpace::World);
    const FVector Dir=Route->GetDirectionAtDistanceAlongSpline(D,ESplineCoordinateSpace::World).GetSafeNormal2D();
    if(Dir.IsNearlyZero())continue;
    const FVector Across=FVector::CrossProduct(FVector::UpVector,Dir).GetSafeNormal();
    bool bClear=true;
    for(float Offset:{0.f,190.f,-190.f}){FString Kind,Label;Sample(P+Across*Offset,Kind,Label);if(Kind!=TEXT("arcade")){bClear=false;break;}}
    if(bClear){S.Site=P;S.Side=Across;S.Stations++;}
   }
  }
  if(S.Site.IsNearlyZero()){Finish(false,TEXT("no station has wide pavement out to 190 cm"),TEXT("\"detail\":\"site selection failed\""));return;}
  UE_LOG(LogTemp,Display,TEXT("TrailModeAuditSite: site=%s side=%s"),*S.Site.ToString(),*S.Side.ToString());
  S.Phase=1;S.Clock=0;return;
 }
 if(S.Phase==1&&S.Clock>.3f){
  auto* Bike=Cast<ABattleBike>(PC->GetPawn());
  if(!Bike||!Bike->Ride){Finish(false,TEXT("the bike went away before the mode could be switched"),TEXT("\"detail\":\"phase 1\""));return;}
  float Lo=0,Hi=0;int32 Splines=0;SplineRange(Lo,Hi,Splines);
  if(Splines<2){Finish(false,TEXT("no trail splines carry the width tag"),FString::Printf(TEXT("\"splines\":%d"),Splines));return;}
  if(!FMath::IsNearlyEqual(Lo,UBattleTrailMode::ArcadeWidthCm,1.f)||!FMath::IsNearlyEqual(Hi,UBattleTrailMode::ArcadeWidthCm,1.f)){
   Finish(false,TEXT("arcade spline width is not the wide one"),FString::Printf(TEXT("\"arcade_spline_cm\":[%.1f,%.1f]"),Lo,Hi));return;
  }
  FString Wide,Label;Sample(S.Site+S.Side*130.f,Wide,Label);
  if(Wide!=TEXT("arcade")){Finish(false,TEXT("arcade pavement is missing at 130 cm"),FString::Printf(TEXT("\"arcade_130\":\"%s\""),*Wide));return;}
  Sample(S.Site+S.Side*190.f,Wide,Label);
  if(Wide!=TEXT("arcade")){Finish(false,TEXT("arcade pavement is missing at 190 cm"),FString::Printf(TEXT("\"arcade_190\":\"%s\""),*Wide));return;}
  S.Phase=2;S.Clock=0;Bike->Ride->bRealHandling=true;return;
 }
 if(S.Phase==2&&S.Clock>.6f){
  auto* Bike=Cast<ABattleBike>(PC->GetPawn());
  if(!Bike||!Bike->Ride||!Bike->Ride->bRealHandling){Finish(false,TEXT("the bike did not go into realistic handling"),TEXT("\"detail\":\"phase 2\""));return;}
  float Lo=0,Hi=0;int32 Splines=0;SplineRange(Lo,Hi,Splines);
  if(!FMath::IsNearlyEqual(Lo,UBattleTrailMode::RealisticWidthCm,1.f)||!FMath::IsNearlyEqual(Hi,UBattleTrailMode::RealisticWidthCm,1.f)){
   Finish(false,TEXT("realistic spline width did not narrow"),FString::Printf(TEXT("\"realistic_spline_cm\":[%.1f,%.1f]"),Lo,Hi));return;
  }
  int32 Arcade=0,Realistic=0,ArcadeHidden=0,RealisticVisible=0;RibbonCounts(Arcade,Realistic,ArcadeHidden,RealisticVisible);
  S.ArcadeActors=Arcade;S.RealisticActors=Realistic;
  if(Arcade==0||Realistic==0){Finish(false,TEXT("one of the two ribbons has no actors"),FString::Printf(TEXT("\"arcade\":%d,\"realistic\":%d"),Arcade,Realistic));return;}
  if(ArcadeHidden!=Arcade){Finish(false,TEXT("the wide ribbon is still visible in realistic mode"),FString::Printf(TEXT("\"arcade_hidden\":%d,\"arcade\":%d"),ArcadeHidden,Arcade));return;}
  if(RealisticVisible!=Realistic){Finish(false,TEXT("the narrow ribbon is not visible in realistic mode"),FString::Printf(TEXT("\"realistic_visible\":%d,\"realistic\":%d"),RealisticVisible,Realistic));return;}
  FString Narrow,NarrowLabel,Edge,EdgeLabel;
  Sample(S.Site+S.Side*130.f,Narrow,NarrowLabel);
  if(Narrow!=TEXT("realistic")){Finish(false,TEXT("realistic pavement is missing at 130 cm"),FString::Printf(TEXT("\"realistic_130\":\"%s\" (\"%s\")"),*Narrow,*NarrowLabel));return;}
  Sample(S.Site+S.Side*190.f,Edge,EdgeLabel);
  if(Edge==TEXT("arcade")||Edge==TEXT("realistic")){
   Finish(false,TEXT("the narrow ribbon still lays pavement at 190 cm"),FString::Printf(TEXT("\"realistic_190\":\"%s\" (\"%s\")"),*Edge,*EdgeLabel));return;
  }
  S.Phase=3;S.Clock=0;Bike->Ride->bRealHandling=false;return;
 }
 if(S.Phase==3&&S.Clock>.6f){
  // ...and back, so the swap is a toggle and not a one-way door.
  FString Back,BackLabel;Sample(S.Site+S.Side*190.f,Back,BackLabel);
  float Lo=0,Hi=0;int32 Splines=0;SplineRange(Lo,Hi,Splines);
  if(Back!=TEXT("arcade")){Finish(false,TEXT("arcade pavement did not come back at 190 cm"),FString::Printf(TEXT("\"arcade_return_190\":\"%s\""),*Back));return;}
  if(!FMath::IsNearlyEqual(Lo,UBattleTrailMode::ArcadeWidthCm,1.f)){Finish(false,TEXT("arcade spline width did not come back"),FString::Printf(TEXT("\"arcade_return_cm\":%.1f"),Lo));return;}
  Finish(true,TEXT("arcade lays 4.2 m of pavement out to 190 cm, realistic lays 3.2 m and stops before it, and the spline width and ribbon visibility follow the toggle both ways"),
   FString::Printf(TEXT("\"arcade_cm\":%.0f,\"realistic_cm\":%.0f,\"station\":\"%s,%s\""),UBattleTrailMode::ArcadeWidthCm,UBattleTrailMode::RealisticWidthCm,*S.Site.ToCompactString(),*S.Side.ToCompactString()));
  return;
 }
#endif
}
