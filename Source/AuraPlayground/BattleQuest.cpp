#include "BattleQuest.h"
#include "BattleTutorialData.h"
#include "BattleTutorialBlock.h"
#include "BattleRouteAnchors.h"
#include "BattleCheckpoints.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "PiedmontPathSpline.h"
#include "PiedmontExplorer.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "Styling/CoreStyle.h"
#include "EngineUtils.h"
#include "GameFramework/HUD.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Sound/SoundBase.h"

ABattleQuest::ABattleQuest(){PrimaryActorTick.bCanEverTick=true;}
void ABattleQuest::BeginPlay(){
 Super::BeginPlay();
 auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);if(!Pawn)return;
 StartLocation=Pawn->GetActorLocation();InitialStartTransform=Pawn->GetActorTransform();
 // Sourced Eastside endpoint reached by the installed Monroe connector.
 ExitLocation=FVector(BattleRouteAnchors::ParkExitX,BattleRouteAnchors::ParkExitY,0);
 FHitResult Ground;FCollisionQueryParams Q(SCENE_QUERY_STAT(BattleExit),false,Pawn);
 if(GetWorld()->LineTraceSingleByChannel(Ground,ExitLocation+FVector(0,0,6000),ExitLocation-FVector(0,0,6000),ECC_Visibility,Q))ExitLocation.Z=Ground.ImpactPoint.Z;
 for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It){
  auto* S=It->Centerline.Get();const float Length=S->GetSplineLength();
  for(float D=0;D<Length;D+=700)Segments.Add({FVector2D(S->GetLocationAtDistanceAlongSpline(D,ESplineCoordinateSpace::World)),FVector2D(S->GetLocationAtDistanceAlongSpline(FMath::Min(D+700,Length),ESplineCoordinateSpace::World))});
 }
 // Ordered sourced mainline, kept separate from Artifact's park-exit exclusion.
 for(int32 Section=0;Section<2;Section++)for(int32 Part=0;Part<(Section==0?8:6);Part++)for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It)if(It->ActorHasTag(FName(*FString::Printf(TEXT("%s_%d"),Section==0?TEXT("BattleEastside"):TEXT("BattleKrog"),Part)))){
  auto* S=It->Centerline.Get();for(int32 I=0;I<S->GetNumberOfSplinePoints();I++){
   const FVector P=S->GetLocationAtSplinePoint(I,ESplineCoordinateSpace::World);
   if(Mainline.IsEmpty()||!Mainline.Last().Equals(P,1))Mainline.Add(P);
  }
 }
 for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("BattleHomeRoute"))){auto* S=It->Centerline.Get();for(int I=0;I<S->GetNumberOfSplinePoints();I++){const FVector P=S->GetLocationAtSplinePoint(I,ESplineCoordinateSpace::World);if(Mainline.IsEmpty()||!Mainline.Last().Equals(P,1))Mainline.Add(P);}}
 EastsideRoutePointCount=Mainline.Num();
 for(const auto& Anchor:BattleCheckpoints::Anchors){
  int32 BestIndex=INDEX_NONE;double Best=TNumericLimits<double>::Max();const FVector Point(Anchor.X,Anchor.Y,Anchor.Z);
  for(int32 I=0;I<Mainline.Num();I++){const double D=FVector::DistSquared(Mainline[I],Point);if(D<Best){Best=D;BestIndex=I;}}
  if(BestIndex!=INDEX_NONE&&Best<100){CheckpointLocations.Add(Mainline[BestIndex]);CheckpointIndices.Add(BestIndex);}
 }
 RouteEndIndex=CheckpointIndices.IsEmpty()?Mainline.Num()-1:CheckpointIndices[0];
 RouteTargetLocation=Mainline.IsValidIndex(RouteEndIndex)?Mainline[RouteEndIndex]:ExitLocation;
 SpawnCheckpointMarkers();
 RadarSegmentCount=Segments.Num();
 bReady=PlaceArtifact();
 UE_LOG(LogTemp,Display,TEXT("BattleQuest: ready=%d candidates=%d way=%s segments=%d location=%s"),bReady,CandidateCount,*ArtifactWay,RadarSegmentCount,*ArtifactLocation.ToString());
}
bool ABattleQuest::PlaceArtifact(){
 struct FCandidate{FVector Location;FString Way;};
 TArray<FCandidate> Candidates;
 const FVector2D Start(StartLocation),Exit(ExitLocation),Direction=Exit-Start;
 for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It){
  if(It->bBridge||!It->bArtifactEligible)continue;
  auto* S=It->Centerline.Get();const float Length=S->GetSplineLength();
  for(float D=FMath::Min(Length*.5f,500.f);D<Length;D+=1800){
   const FVector P=S->GetLocationAtDistanceAlongSpline(D,ESplineCoordinateSpace::World);const FVector2D P2(P);
   const float Along=FMath::Clamp(FVector2D::DotProduct(P2-Start,Direction)/FMath::Max(Direction.SizeSquared(),1.f),0.f,1.f);
   if(FVector2D::Distance(P2,Start)<5000||FVector2D::Distance(P2,Start+Direction*Along)<1200)continue;
   Candidates.Add({P,It->OsmWayId});
  }
 }
 CandidateCount=Candidates.Num();auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);
 FCollisionQueryParams Q(SCENE_QUERY_STAT(BattleArtifactGround),false,Pawn);
 while(!Candidates.IsEmpty()){
  const int I=FMath::RandRange(0,Candidates.Num()-1);const FCandidate Candidate=Candidates[I];Candidates.RemoveAtSwap(I);
  FHitResult Hit;
  if(!GetWorld()->LineTraceSingleByChannel(Hit,Candidate.Location+FVector(0,0,250),Candidate.Location-FVector(0,0,250),ECC_Visibility,Q)||!Hit.GetActor()||!Hit.GetActor()->ActorHasTag(TEXT("RidePath")))continue;
  bool Wet=false;for(TActorIterator<APiedmontWaterHazard> Water(GetWorld());Water;++Water)if(Water->ContainsBike(Hit.ImpactPoint+FVector(0,0,96))){Wet=true;break;}
  if(Wet||GetWorld()->OverlapBlockingTestByChannel(Hit.ImpactPoint+FVector(0,0,98),FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,96),Q))continue;
  auto* Path=UNavigationSystemV1::FindPathToLocationSynchronously(this,StartLocation,Hit.ImpactPoint,Pawn);
  if(!Path||!Path->IsValid()||Path->IsPartial()||Path->PathPoints.Num()<2)continue;
  auto* HomePath=UNavigationSystemV1::FindPathToLocationSynchronously(this,Hit.ImpactPoint,ExitLocation,Pawn);
  if(!HomePath||!HomePath->IsValid()||HomePath->IsPartial()||HomePath->PathPoints.Num()<2)continue;
  ArtifactLocation=Hit.ImpactPoint+FVector(0,0,85);ArtifactWay=Candidate.Way;
  auto* Phone=GetWorld()->SpawnActor<AStaticMeshActor>(ArtifactLocation,FRotator(0,0,12));
  if(!Phone)return false;
  auto* Body=Phone->GetStaticMeshComponent();Body->SetMobility(EComponentMobility::Movable);
  auto* Cube=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));
  Body->SetStaticMesh(Cube);Body->SetWorldScale3D(FVector(.22,.055,.38));Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);Body->SetCanEverAffectNavigation(false);
  Body->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BeltLineGlide/Materials/M_Rubber.M_Rubber")));
  auto* Screen=NewObject<UStaticMeshComponent>(Phone);Screen->SetupAttachment(Body);Screen->SetStaticMesh(Cube);Screen->SetRelativeLocation(FVector(0,-53,0));Screen->SetRelativeScale3D(FVector(.84,.08,.83));Screen->SetCollisionEnabled(ECollisionEnabled::NoCollision);Screen->SetCanEverAffectNavigation(false);Screen->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_ShotGlow.M_ShotGlow")));Screen->RegisterComponent();
  auto* Light=NewObject<UPointLightComponent>(Phone);Light->SetupAttachment(Body);Light->SetIntensity(180);Light->SetAttenuationRadius(220);Light->SetLightColor(FLinearColor(1,.65,.05));Light->SetCastShadows(false);Light->RegisterComponent();
  Artifact=Phone;Phone->Tags.Add(TEXT("BattleArtifact"));return true;
 }
 return false;
}
void ABattleQuest::RefreshRoute(){
 RoutePoints.Reset();auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);if(!Pawn)return;
 FVector Join=ExitLocation;int32 Next=0;float Best=TNumericLimits<float>::Max();
 if(Mainline.Num()>1)for(int32 I=0;I<RouteEndIndex;I++){
  const FVector2D A(Mainline[I]),B(Mainline[I+1]),P(Pawn->GetActorLocation());
  const FVector2D D=B-A;const float T=D.SizeSquared()>0?FMath::Clamp(FVector2D::DotProduct(P-A,D)/D.SizeSquared(),0.,1.):0;
  const FVector Q=FMath::Lerp(Mainline[I],Mainline[I+1],T);const float Distance=FVector::DistSquared2D(Pawn->GetActorLocation(),Q);
  if(Distance<Best){Best=Distance;Join=Q;Next=I+1;}
 }
 auto* Path=UNavigationSystemV1::FindPathToLocationSynchronously(this,Pawn->GetActorLocation(),Join,Pawn);
 if(Path&&Path->IsValid()&&!Path->IsPartial())RoutePoints=Path->PathPoints;
 else if(Best<FMath::Square(120.f)&&FMath::Abs(Pawn->GetActorLocation().Z-Join.Z)<150){
  // At an exact trail/checkpoint join Recast can return a zero-length path.
  // A nearby, unobstructed join needs no approach route, but still follows the
  // canonical trail. Do not turn an arbitrary failed navigation query into a shortcut.
  FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(BattleRouteJoin),false,Pawn);
  if(!GetWorld()->LineTraceSingleByChannel(Hit,Pawn->GetActorLocation(),Join+FVector(0,0,98),ECC_Visibility,Q))RoutePoints.Add(Join);
 }
 if(!RoutePoints.IsEmpty())for(int32 I=Next;I<=RouteEndIndex;I++)RoutePoints.Add(Mainline[I]);
}
void ABattleQuest::Tick(float Dt){
 Super::Tick(Dt);Clock+=Dt;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));
 auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);if(!Mode||!Pawn)return;
 if(!bReady){RouteDelay-=Dt;if(RouteDelay<=0){RouteDelay=2;bReady=PlaceArtifact();}return;}
 auto* Bike=Cast<ABattleBike>(Pawn);if(auto* Person=Cast<ABattleRider>(Pawn))Bike=Person->ParkedBike;
 if(Artifact&&!bCollected){Artifact->SetActorLocation(ArtifactLocation+FVector(0,0,FMath::Sin(Clock*2.5f)*8));Artifact->AddActorWorldRotation(FRotator(0,Dt*45,0));}
 if(Bike&&Bike->RiderHealth>0&&Bike->RespawnRemaining<=0&&!Mode->bTutorialActive&&Mode->StartCountdown<=0&&!Mode->bRunEnded&&!bCollected&&FVector::DistSquared(Pawn->GetActorLocation(),ArtifactLocation)<FMath::Square(150.f)){
  FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(BattleCollect),false,Pawn);Q.AddIgnoredActor(Artifact);
  if(!GetWorld()->LineTraceSingleByChannel(Hit,Pawn->GetActorLocation(),ArtifactLocation,ECC_Visibility,Q)){
   bCollected=true;Mode->bItemCollected=true;
   if(Artifact){Artifact->Destroy();Artifact=nullptr;}
   CheckpointNotice=TEXT("PHONE FOUND • Morgan is waiting at 98 Estoria");CheckpointNoticeTime=6;
   RefreshRoute();
   if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_Boost.S_Boost")))UGameplayStatics::PlaySound2D(this,Sound,.5f,1.5f);
   UE_LOG(LogTemp,Display,TEXT("BattleQuest: Artifact collected; route points=%d"),RoutePoints.Num());
  }
 }
 if(bCollected&&!Mode->bRunEnded){UpdateCheckpoints(Dt);RouteDelay-=Dt;if(RouteDelay<=0){RouteDelay=2;RefreshRoute();}}
 EnemyDelay-=Dt;if(EnemyDelay<=0){
  EnemyDelay=.5f;EnemyLocations.Reset();
  for(TActorIterator<APawn> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("PiedmontHostile"))||It->ActorHasTag(TEXT("BattleHostile"))){
   auto* Legacy=Cast<APiedmontExplorer>(*It);if(!It->IsHidden()&&(!Legacy||!Legacy->bDead))EnemyLocations.Add(It->GetActorLocation());
  }
 }
}
void ABattleQuest::EndPlay(const EEndPlayReason::Type Reason){if(IsValid(Artifact))Artifact->Destroy();for(auto Marker:CheckpointMarkers)if(IsValid(Marker))Marker->Destroy();Super::EndPlay(Reason);}
bool ABattleQuest::ArtifactVisibleOnRadar(FVector Viewer) const{return false;}
FVector2D ABattleQuest::CoarseDirection(FVector2D Delta){
 if(FMath::Abs(Delta.X)>FMath::Abs(Delta.Y))return FVector2D(Delta.X>=0?1:-1,0);
 return FVector2D(0,Delta.Y>=0?1:-1);
}
FString ABattleQuest::SearchDirection(FVector Viewer) const{
 if(!bReady)return TEXT("Searching for signal...");
 const FVector2D D=CoarseDirection(FVector2D(ArtifactLocation-Viewer));
 return D.X>0?TEXT("Search EAST"):D.X<0?TEXT("Search WEST"):D.Y<0?TEXT("Search NORTH"):TEXT("Search SOUTH");
}
void ABattleQuest::ResetAfterDeath(){
 SearchResets++;bCollected=false;bReady=false;RoutePoints.Reset();NextCheckpoint=0;CheckpointNotice.Empty();CheckpointNoticeTime=0;
 if(IsValid(Artifact))Artifact->Destroy();Artifact=nullptr;
 RouteEndIndex=CheckpointIndices.IsEmpty()?Mainline.Num()-1:CheckpointIndices[0];
 RouteTargetLocation=Mainline.IsValidIndex(RouteEndIndex)?Mainline[RouteEndIndex]:ExitLocation;
 if(auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this)))Mode->bItemCollected=false;
 for(TActorIterator<ABattleBike> It(GetWorld());It;++It){It->CheckpointTransform=InitialStartTransform;It->CheckpointName=TEXT("14th Street Gate");}
 bReady=PlaceArtifact();RouteDelay=2;
 UE_LOG(LogTemp,Display,TEXT("BattleQuest: death reset=%d ready=%d"),SearchResets,bReady);
}
bool ABattleQuest::ClipToCircle(FVector2D& A,FVector2D& B,float Radius){
 const FVector2D D=B-A;const double AA=D.SizeSquared();if(AA<.00001)return A.SizeSquared()<=Radius*Radius;
 const double BB=2*FVector2D::DotProduct(A,D),CC=A.SizeSquared()-Radius*Radius,Disc=BB*BB-4*AA*CC;
 if(Disc<0)return false;
 const double T0=FMath::Max(0.,(-BB-FMath::Sqrt(Disc))/(2*AA)),T1=FMath::Min(1.,(-BB+FMath::Sqrt(Disc))/(2*AA));
 if(T0>T1)return false;const FVector2D Origin=A;A=Origin+D*T0;B=Origin+D*T1;return true;
}
void ABattleQuest::DrawRadar(AHUD* HUD,UCanvas* Canvas) const{
 if(!HUD||!Canvas)return;auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);if(!Pawn)return;
 const auto* PracticeMode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));const bool Practice=PracticeMode&&PracticeMode->bTutorialActive;
 const float UIScale=FMath::Clamp(FMath::Min(Canvas->SizeX/1920.f,Canvas->SizeY/1080.f),.75f,1.5f);const float Radius=140*UIScale;const FVector2D Center(Canvas->SizeX-Radius-40*UIScale,Canvas->SizeY-Radius-45*UIScale),Player(Pawn->GetActorLocation());
 auto Project=[&](FVector2D World){return RadarOffset(World-Player,Radius/FMath::Max(1.f,RadarRange));};
 auto Line=[&](FVector2D A,FVector2D B,FLinearColor C,float W){HUD->DrawLine(Center.X+A.X,Center.Y+A.Y,Center.X+B.X,Center.Y+B.Y,C,W);};
 for(float Y=-Radius;Y<Radius;Y+=2){const float X=FMath::Sqrt(FMath::Max(0.f,Radius*Radius-Y*Y));HUD->DrawRect(FLinearColor(.008,.016,.025,.9),Center.X-X,Center.Y+Y,X*2,2);}
 auto Circle=[&](FVector2D C,float R,FLinearColor Color,float Width){for(int I=0;I<48;I++){const float A=I*2*PI/48,B=(I+1)*2*PI/48;Line(C+FVector2D(FMath::Cos(A),FMath::Sin(A))*R,C+FVector2D(FMath::Cos(B),FMath::Sin(B))*R,Color,Width);}};
 for(const auto& Segment:Segments){FVector2D A=Project(Segment.Key),B=Project(Segment.Value);if(ClipToCircle(A,B,Radius-3))Line(A,B,FLinearColor(.36,.48,.5),1);}
 if(Practice)for(int I=1;I<UE_ARRAY_COUNT(BattleTutorialData::Road);I++){FVector2D A=Project(FVector2D(BattleTutorialData::Road[I-1])),B=Project(FVector2D(BattleTutorialData::Road[I]));if(ClipToCircle(A,B,Radius-3))Line(A,B,FLinearColor(.2,1,.7),2);}
 if(Practice)for(int I=1;I<UE_ARRAY_COUNT(BattleTutorialBlock::Alternate);I++){FVector2D A=Project(FVector2D(BattleTutorialBlock::Alternate[I-1])),B=Project(FVector2D(BattleTutorialBlock::Alternate[I]));if(ClipToCircle(A,B,Radius-3))Line(A,B,FLinearColor(.2,1,.7),2);}
 if(bCollected)for(int I=1;I<RoutePoints.Num();I++){FVector2D A=Project(FVector2D(RoutePoints[I-1])),B=Project(FVector2D(RoutePoints[I]));if(ClipToCircle(A,B,Radius-3))Line(A,B,FLinearColor(1,.66,.05),2);}
 Circle(FVector2D::ZeroVector,Radius,FLinearColor(.3,.6,.67),2);
 const float Yaw=FMath::DegreesToRadians(Pawn->GetActorRotation().Yaw);const FVector2D Forward(FMath::Cos(Yaw),FMath::Sin(Yaw)),Right(-Forward.Y,Forward.X);
 Line(Forward*8,-Forward*5+Right*5,FLinearColor::White,2);Line(Forward*8,-Forward*5-Right*5,FLinearColor::White,2);Line(-Forward*5+Right*5,-Forward*5-Right*5,FLinearColor::White,2);
 if(!Pawn->IsA<ABattleBike>())for(TActorIterator<ABattleBike> It(GetWorld());It;++It)if(It->bParked){const FVector2D B=Project(FVector2D(It->GetActorLocation())).GetClampedToMaxSize(Radius-9);HUD->DrawText(TEXT("B"),FColor::Cyan,Center.X+B.X-4,Center.Y+B.Y-6,nullptr,.9f);}
 for(FVector P:EnemyLocations){FVector2D D=Project(FVector2D(P));if(D.Size()<Radius-4)Circle(D,2.5f,FLinearColor::Red,2);}
 if(bReady){
  const FVector Target=Practice?BattleTutorialData::Gate:bCollected?RouteTargetLocation:ArtifactLocation;const float Distance=FVector::Dist2D(Pawn->GetActorLocation(),Target);
  const float Pulse=.65f+.35f*FMath::Sin(Clock*2*PI);
  const FLinearColor Gold(1,.5f+.3f*Pulse,.05f);
  const FVector2D D=(bCollected||Practice)?Project(FVector2D(Target)):CoarseDirection(FVector2D(Target-Pawn->GetActorLocation()))*Radius;
  if((bCollected||Practice)&&Distance<=RadarRange)Circle(D.GetClampedToMaxSize(Radius-6),3+Pulse*2,Gold,2);
  else{const auto F=D.GetSafeNormal(),R=FVector2D(-F.Y,F.X);const auto Tip=F*(Radius-5);Line(Tip,Tip-F*10+R*6,Gold,3);Line(Tip,Tip-F*10-R*6,Gold,3);}
 }
 auto Compass=[&](const TCHAR* Label,FVector2D Offset){if(auto* BattleHUD=Cast<ABattleLabHUD>(HUD)){FCanvasTextItem Item(Center+Offset,FText::FromString(Label),FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(16*UIScale)),FLinearColor::White);Item.Font=BattleHUD->ReadableFont;Item.bCentreX=true;Canvas->DrawItem(Item);}};
 Compass(TEXT("ELLISON’S WATCH"),FVector2D(0,-Radius-52*UIScale));
 Compass(TEXT("N"),FVector2D(0,-Radius-28*UIScale));Compass(TEXT("S"),FVector2D(0,Radius+3*UIScale));Compass(TEXT("W"),FVector2D(-Radius-18*UIScale,-12*UIScale));Compass(TEXT("E"),FVector2D(Radius+18*UIScale,-12*UIScale));

}
