#include "BattleScooterTraffic.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleScooterProp.h"
#include "PiedmontPathSpline.h"
#include "Camera/CameraActor.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
#include "UObject/ConstructorHelpers.h"

ABattleScooterTraffic::ABattleScooterTraffic(){
 PrimaryActorTick.bCanEverTick=true;
 static ConstructorHelpers::FClassFinder<AActor> Scooter(TEXT("/Game/BeltLineGlide/BP_ScooterRider"));
 ScooterClass=Scooter.Class;
}
// The path spline carries no ground truth: the BeltLine pavement ribbon is
// separate geometry, and the bridge and park decks sit at their own heights.
// Scooters were placed at spline Z + 92 with no trace at all, so anywhere the
// spline ran above the visible surface they hovered and slid along in mid-air -
// which is what Elliott has been reporting as "a flying log". Every other
// placed prop in this project traces down to its surface (BattleSpareBikes,
// the park furniture, the Trees ATL sign); this is the only one that did not.
// Snap only when a walkable-looking surface sits close to the spline, so an
// overpass deck above the rider is never mistaken for the ground below.
static FVector ScooterGroundedLocation(UWorld* World,const AActor* Owner,const AActor* Ignore,const FVector& Spline){
 FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(ScooterGround),false,Owner);
 // A scooter already parked at this spot is the one thing the trace must not
 // hit: it used to snap to the scooter's own body instead of the pavement and
 // then ride up on top of itself a little further every tick.
 if(Ignore)Q.AddIgnoredActor(Ignore);
 if(World->LineTraceSingleByChannel(Hit,Spline+FVector(0,0,120),Spline-FVector(0,0,260),ECC_Visibility,Q)
  &&Hit.ImpactNormal.Z>.7f&&FMath::Abs(Hit.ImpactPoint.Z-Spline.Z)<150.f)
  return FVector(Spline.X,Spline.Y,Hit.ImpactPoint.Z+92.f);
 return Spline+FVector(0,0,92.f);
}
void ABattleScooterTraffic::BeginPlay(){
 Super::BeginPlay();auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!Mode||!ScooterClass)return;
 TArray<APiedmontPathSpline*> Paths;for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It)if(It->bArtifactEligible&&!It->bBridge&&It->Centerline&&It->Centerline->GetSplineLength()>1800)Paths.Add(*It);
 Paths.Sort([](const APiedmontPathSpline& A,const APiedmontPathSpline& B){return A.OsmWayId<B.OsmWayId;});
 // Scooters are part of the furniture of the whole ride, not a rarity: Elliott
 // asked for them by name after riding the junctions. The difficulty table sets
 // the requested count; this raises the floor so they are always around.
 FRandomStream Random(9602);const int32 Count=FMath::Min(FMath::Max(Mode->Difficulty.Scooters,8),Paths.Num()*2);
 for(int32 I=0;I<Count;I++){
  auto* Path=Paths[I%Paths.Num()];const float Length=Path->Centerline->GetSplineLength();const float Distance=Random.FRandRange(250.f,Length-250.f);
  const bool Reverse=Random.FRand()<Mode->Difficulty.ScooterWrongWayFraction;FVector Location=ScooterGroundedLocation(GetWorld(),this,nullptr,Path->Centerline->GetLocationAtDistanceAlongSpline(Distance,ESplineCoordinateSpace::World));
  FVector Direction=Path->Centerline->GetDirectionAtDistanceAlongSpline(Distance,ESplineCoordinateSpace::World)*(Reverse?-1.f:1.f);
  FActorSpawnParameters Params;Params.Owner=this;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
  if(auto* Rider=GetWorld()->SpawnActor<AActor>(ScooterClass,Location,Direction.Rotation(),Params)){
   Rider->SetActorTickEnabled(false);Rider->Tags.Add(TEXT("BattleScooterRider"));
   // BP_ScooterRider's only visible part is an untextured engine cylinder, so
   // every moving scooter on the trail was a grey tube. Hide the placeholder -
   // after the scooter is built, so the new parts are not hidden with it - and
   // put the authored scooter on the same actor, which still owns the motion.
   TArray<UStaticMeshComponent*> Placeholder;
   Rider->GetComponents(Placeholder);
   for(auto* Part:Placeholder)Part->SetVisibility(false,true);
   // The prop's anchor is the deck plane, and a scooter standing on its wheels
   // wants that anchor 18 cm above the pavement. The actor origin sits 92 cm up,
   // so the anchor goes at Location - 74. It used to go at Location - 92, which
   // put the deck level with the surface and buried both wheels: a plank with a
   // stick through it, sliding along the trail, which is the thing Elliott has
   // been calling a log since the trail opened.
   BuildBattleScooter(Rider,Rider->GetRootComponent(),Location-FVector(0,0,74.f),FRotator(0,Direction.Rotation().Yaw,0),SpawnedScooters,false);
   const float Speeds[]={Mode->Difficulty.ScooterSlowSpeed,Mode->Difficulty.ScooterMediumSpeed,Mode->Difficulty.ScooterFastSpeed};
   Scooters.Add({Rider,Path,Distance,Speeds[I%3],Reverse});SpawnedScooters++;
  }
 }
 UE_LOG(LogTemp,Display,TEXT("BattleScooters: visible moving riders=%d requested=%d"),SpawnedScooters,Mode->Difficulty.Scooters);
}
void ABattleScooterTraffic::Tick(float Dt){
 Super::Tick(Dt);TickScooterReview(Dt);ContactCooldown=FMath::Max(0.f,ContactCooldown-Dt);auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);auto* Bike=Cast<ABattleBike>(Pawn);if(auto* Foot=Cast<ABattleRider>(Pawn))Bike=Foot->ParkedBike;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!Mode||Mode->bRunEnded)return;
 for(auto& Moving:Scooters){auto* Rider=Moving.Actor.Get();auto* Path=Moving.Path.Get();if(!Rider||!Path||!Path->Centerline)continue;const float Length=Path->Centerline->GetSplineLength();Moving.Distance+=Moving.Speed*Dt*(Moving.Reverse?-1.f:1.f);while(Moving.Distance>Length)Moving.Distance-=Length;while(Moving.Distance<0)Moving.Distance+=Length;
  const FVector Location=ScooterGroundedLocation(GetWorld(),this,Rider,Path->Centerline->GetLocationAtDistanceAlongSpline(Moving.Distance,ESplineCoordinateSpace::World));FVector Direction=Path->Centerline->GetDirectionAtDistanceAlongSpline(Moving.Distance,ESplineCoordinateSpace::World)*(Moving.Reverse?-1.f:1.f);Rider->SetActorLocationAndRotation(Location,Direction.Rotation(),false,nullptr,ETeleportType::TeleportPhysics);
  if(Pawn&&Bike&&ContactCooldown<=0&&FVector::DistSquared(Pawn->GetActorLocation(),Location)<FMath::Square(125.f)){PlayerContacts++;ContactCooldown=2;Bike->Ride->Wipeout(TEXT("Scooter rider clipped the bike"));}
 }
}

// Development-only visual QA for the scooter prop. Two angles, because the
// defect this exists to catch - wheels buried so the scooter reads as a plank,
// or a pose that stands it on its nose - is invisible from the rider's seat.
void ABattleScooterTraffic::TickScooterReview(float Dt){
#if !UE_BUILD_SHIPPING
 FString Dir;FParse::Value(FCommandLine::Get(),TEXT("BattleScooterTrafficReviewDir="),Dir);if(Dir.IsEmpty()||Scooters.IsEmpty())return;
 auto* Target=Scooters[0].Actor.Get();if(!Target)return;
 ReviewClock+=Dt;
 const FVector P=Target->GetActorLocation();
 const FVector Eye=P+FVector(300,-230,140),Look=P-FVector(0,0,45);
 if(!ReviewCamera)ReviewCamera=GetWorld()->SpawnActor<ACameraActor>(Eye,(Look-Eye).Rotation());
 if(ReviewCamera){
  ReviewCamera->SetActorLocationAndRotation(Eye,(Look-Eye).Rotation());
  if(auto* PC=UGameplayStatics::GetPlayerController(this,0))PC->SetViewTarget(ReviewCamera);
 }
 if(ReviewStage==0&&ReviewClock>.8f){FScreenshotRequest::RequestScreenshot(Dir/TEXT("scooter-traffic-side.png"),false,false);++ReviewStage;ReviewClock=0;return;}
 if(ReviewStage==1&&ReviewClock>.4f){
  const FVector Behind=P+FVector(-110,0,115),At=P+FVector(0,0,35);
  if(ReviewCamera)ReviewCamera->SetActorLocationAndRotation(Behind,(At-Behind).Rotation());
  ++ReviewStage;ReviewClock=0;return;
 }
 if(ReviewStage==2&&ReviewClock>.6f){FScreenshotRequest::RequestScreenshot(Dir/TEXT("scooter-traffic-back.png"),false,false);++ReviewStage;ReviewClock=0;return;}
 if(ReviewStage==3&&ReviewClock>.4f)
  if(auto* PC=UGameplayStatics::GetPlayerController(this,0))UKismetSystemLibrary::QuitGame(this,PC,EQuitPreference::Quit,false);
#endif
}
