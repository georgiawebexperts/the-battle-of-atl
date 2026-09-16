#include "BattleTutorial.h"
#include "BattleGateSupport.h"
#include "BattleMarketClosure.h"
#include "BattleTutorialData.h"
#include "BattleTutorialBlock.h"
#include "BattlePrideRoadCleanup.h"
#include "BattleGateRoadCleanup.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleQuest.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
ABattleTutorial::ABattleTutorial(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("TutorialRoot"));Tags.Add(TEXT("RidePath"));Tags.Add(TEXT("BattleTutorial"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> White(TEXT("/Game/PiedmontRide/Materials/M_Concrete.M_Concrete")),Wood(TEXT("/Game/BattleForTheA/Furniture/M_BenchWood.M_BenchWood")),Dark(TEXT("/Game/BattleForTheA/Furniture/M_BenchFrame.M_BenchFrame")),Red(TEXT("/Game/BattleForTheA/Materials/M_ColaRed.M_ColaRed")),Asphalt(TEXT("/Game/PiedmontRide/Materials/M_Asphalt.M_Asphalt")),TextMat(TEXT("/Engine/EngineMaterials/UnlitText.UnlitText"));
 auto Group=[&](const TCHAR* N,UMaterialInterface* Mat){auto* M=CreateDefaultSubobject<UInstancedStaticMeshComponent>(N);M->SetupAttachment(RootComponent);M->SetStaticMesh(Cube.Object);M->SetMaterial(0,Mat);M->SetCollisionProfileName(TEXT("BlockAll"));return M;};
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> SidingMat(TEXT("/Game/BattleForTheA/Environment/TutorialHouse/M_Siding.M_Siding")),TrimMat(TEXT("/Game/BattleForTheA/Environment/TutorialHouse/M_Trim.M_Trim")),HouseDoorMat(TEXT("/Game/BattleForTheA/Environment/TutorialHouse/M_Door.M_Door")),GlassMat(TEXT("/Game/BattleForTheA/Environment/TutorialHouse/M_Glass.M_Glass"));
 auto* Siding=Group(TEXT("BungalowSiding"),SidingMat.Object);auto* Trim=Group(TEXT("BungalowTrim"),TrimMat.Object);auto* HouseDoor=Group(TEXT("BungalowDoor"),HouseDoorMat.Object);auto* Glass=Group(TEXT("BungalowGlass"),GlassMat.Object);
 auto* Concrete=Group(TEXT("HouseStone"),White.Object);auto* Timber=Group(TEXT("Porch"),Wood.Object);auto* Iron=Group(TEXT("IronAndRoof"),Dark.Object);auto* Door=Group(TEXT("Door"),Red.Object);auto* Road=Group(TEXT("PracticeStreet"),Asphalt.Object);PracticeStreet=Road;
 auto Part=[&](UInstancedStaticMeshComponent* M,FVector P,FVector Size,FRotator R=FRotator::ZeroRotator){M->AddInstance(FTransform(R,P,Size/100));};
 for(int I=1;I<UE_ARRAY_COUNT(BattleTutorialData::Road);I++){const FVector A=BattleTutorialData::Road[I-1],B=BattleTutorialData::Road[I],D=B-A;Part(Road,(A+B)*.5f-FVector(0,0,12),FVector(D.Size()+3,450,24),D.Rotation());}
 auto Span=[&](FVector A,FVector B){const FVector D=B-A;Part(Road,(A+B)*.5f-FVector(0,0,12),FVector(D.Size()+3,450,24),D.Rotation());};
 for(int I=1;I<UE_ARRAY_COUNT(BattleTutorialBlock::Alternate);I++)Span(BattleTutorialBlock::Alternate[I-1],BattleTutorialBlock::Alternate[I]);
 for(int I=0;I<UE_ARRAY_COUNT(BattleTutorialBlock::StubEnds);I+=2)Span(BattleTutorialBlock::StubEnds[I],BattleTutorialBlock::StubEnds[I+1]);
 // Continuous terrain-fitted frontage covers the old block seams near the house.
 for(const TCHAR* Name:{TEXT("FrontageRoad"),TEXT("FrontageSouthWalk"),TEXT("FrontageNorthWalk")}){
  const FString Path=FString::Printf(TEXT("/Game/BattleForTheA/Environment/StartingStreet/SM_%s.SM_%s"),Name,Name);
  ConstructorHelpers::FObjectFinder<UStaticMesh> Asset(*Path);
  auto* Surface=CreateDefaultSubobject<UStaticMeshComponent>(Name);Surface->SetupAttachment(RootComponent);Surface->SetStaticMesh(Asset.Object);Surface->SetCollisionProfileName(TEXT("BlockAll"));
 }
 const FVector H=BattleTutorialData::Home+FVector(0,80,80);
 // Native terrain peaks 68 cm above the original base at the porch.
 // A solid foundation spans the lower rear grade without exposing a floating floor.
 Part(Concrete,H+FVector(0,0,-75),FVector(650,850,150));
 Part(Concrete,H+FVector(0,-515,-75),FVector(680,180,150));
 // The fictional bungalow has its own finishes, separate from street barriers and gate stone.
 Part(Siding,H+FVector(0,0,180),FVector(650,850,360));
 for(int Z=30;Z<355;Z+=18){
  Part(Siding,H+FVector(0,-429,Z),FVector(650,12,14),FRotator(0,0,-7));
  for(int Side:{-1,1})Part(Siding,H+FVector(Side*329,0,Z),FVector(12,850,14));
 }
 for(int Side:{-1,1}){
  Part(Glass,H+FVector(Side*170,-438,205),FVector(115,12,140));
  for(int DX:{-1,1})Part(Trim,H+FVector(Side*170+DX*64,-449,205),FVector(10,12,155));
  for(int DZ:{-1,1})Part(Trim,H+FVector(Side*170,-449,205+DZ*76),FVector(138,12,10));
  Part(Trim,H+FVector(Side*170,-451,205),FVector(6,12,138));
  Part(Trim,H+FVector(Side*170,-451,210),FVector(120,12,6));
  Part(Trim,H+FVector(Side*170,-457,124),FVector(145,26,12));
  Part(Iron,H+FVector(Side*174,0,445),FVector(390,950,22),FRotator(-Side*25,0,0));
  Part(Trim,H+FVector(Side*174,-482,445),FVector(390,16,25),FRotator(-Side*25,0,0));
  Part(Trim,H+FVector(Side*320,-437,185),FVector(18,18,340));
  Part(Trim,H+FVector(Side*280,-555,171),FVector(28,28,288));
  Part(Concrete,H+FVector(Side*280,-555,55),FVector(48,48,66));
  Part(Trim,H+FVector(Side*280,-555,312),FVector(44,44,18));
 }
 for(int Z=370;Z<526;Z+=12)for(int Side:{-1,1})Part(Siding,H+FVector(0,Side*426,Z),FVector(FMath::Min(650.f,(527-Z)*2/FMath::Tan(FMath::DegreesToRadians(25.f))),12,12));
 Part(HouseDoor,H+FVector(0,-445,137),FVector(115,15,230));
 for(int Side:{-1,1})Part(Trim,H+FVector(Side*64,-453,138),FVector(12,16,240));
 Part(Trim,H+FVector(0,-453,260),FVector(140,16,15));
 for(int Z:{85,155})for(int Side:{-1,1})Part(HouseDoor,H+FVector(Side*27,-456,Z),FVector(42,8,50));
 Part(Glass,H+FVector(0,-456,218),FVector(83,8,34));
 Part(Trim,H+FVector(43,-466,137),FVector(5,10,18));
 Part(Timber,H+FVector(0,-515,22),FVector(680,180,44));
 Part(Iron,H+FVector(0,-530,326),FVector(720,240,20));
 Part(Trim,H+FVector(0,-653,317),FVector(720,14,25));
 for(int I=0;I<4;I++)Part(Concrete,H+FVector(0,-625-I*30,-36-I*9),FVector(145,40,128-I*18));
 Part(Concrete,H+FVector(0,-744,-30),FVector(145,30,8));
 for(int Side:{-1,1})for(int X=160;X<=300;X+=35)Part(Trim,H+FVector(Side*X,-600,70),FVector(14,14,140));
 for(int Side:{-1,1})for(int Z:{35,105})Part(Timber,H+FVector(Side*225,-600,Z),FVector(170,12,12));
 Part(Timber,H+FVector(180,-625,70),FVector(14,14,140));Part(Iron,H+FVector(180,-625,145),FVector(100,45,45));
 auto Label=[&](const TCHAR* N,const TCHAR* T,FVector P,float Size,FRotator R){auto* C=CreateDefaultSubobject<UTextRenderComponent>(N);C->SetupAttachment(RootComponent);C->SetRelativeLocation(P);C->SetRelativeRotation(R);C->SetWorldSize(Size);C->SetText(FText::FromString(T));C->SetTextMaterial(TextMat.Object);C->SetTextRenderColor(FColor(255,229,183));C->SetHorizontalAlignment(EHTA_Center);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);return C;};
 Label(TEXT("Mailbox"),TEXT("ELLISON"),H+FVector(180,-650,140),16,FRotator(0,-90,0));
 for(int I=0;I<UE_ARRAY_COUNT(BattleTutorialBlock::BarrierCenters);I++){
  auto* Boundary=CreateDefaultSubobject<USceneComponent>(*FString::Printf(TEXT("BoundaryRoot%d"),I));Boundary->SetupAttachment(RootComponent);Boundary->SetRelativeLocation(BattleTutorialBlock::BarrierCenters[I]);BoundaryRoots.Add(Boundary);
  auto* BoundaryIron=Group(*FString::Printf(TEXT("BoundaryIron%d"),I),Dark.Object);BoundaryIron->SetupAttachment(Boundary);
  auto* BoundaryDoor=Group(*FString::Printf(TEXT("BoundaryRed%d"),I),Red.Object);BoundaryDoor->SetupAttachment(Boundary);
  auto* BoundaryConcrete=Group(*FString::Printf(TEXT("BoundaryConcrete%d"),I),White.Object);BoundaryConcrete->SetupAttachment(Boundary);
  auto* BoundaryCollision=Group(*FString::Printf(TEXT("BoundaryCollision%d"),I),Dark.Object);BoundaryCollision->SetupAttachment(Boundary);BoundaryCollision->SetVisibility(false,true);BoundaryCollision->SetHiddenInGame(true,true);
  const FVector C=FVector::ZeroVector,D=BattleTutorialBlock::BarrierDirections[I],Right(-D.Y,D.X,0);const FRotator R=D.Rotation();
  // Human-scale temporary fencing still blocks the road without reading as a
  // prison wall or hiding the park beyond it.
  constexpr float ClosureFenceTop=190.f;
  Part(BoundaryCollision,C+FVector(0,0,180),FVector(20,2050,360),R);
  for(int Y=-1000;Y<=1000;Y+=45)Part(BoundaryIron,C+Right*Y+FVector(0,0,ClosureFenceTop*.5f),FVector(10,7,ClosureFenceTop),R);
  for(float Z:{25.f,75.f,125.f,ClosureFenceTop-15.f})Part(BoundaryIron,C+FVector(0,0,Z),FVector(10,2050,6),R);
  Part(BoundaryDoor,C-D*16+FVector(0,0,122),FVector(20,740,78),R);
  Part(BoundaryConcrete,C-D*29+FVector(0,0,103),FVector(8,740,14),R);
  for(int Side:{-1,1}){Part(BoundaryConcrete,C+Right*Side*440+FVector(0,0,35),FVector(150,150,70),R);Part(BoundaryDoor,C+Right*Side*440+FVector(0,0,78),FVector(55,55,45),R);}
  Label(*FString::Printf(TEXT("Closure%d"),I),TEXT("UNDER CONSTRUCTION"),C-D*31+FVector(0,0,142),25,(-D).Rotation())->SetupAttachment(Boundary);
  Label(*FString::Printf(TEXT("Expansion%d"),I),TEXT("MORE MIDTOWN COMING SOON"),C-D*31+FVector(0,0,111),17,(-D).Rotation())->SetupAttachment(Boundary);
 }
 // Stone piers and open iron wings leave the central ride-through clear.
 // Keep the side fencing near a five-foot park scale so the entrance preserves
 // long views into the lawn. The masonry remains tall enough to read as the
 // 14th Street landmark arch supports.
 const FVector G=BattleTutorialData::Gate;
 constexpr float GateFenceTop=150.f;
 for(int Side:{-1,1}){
  for(int Z=20;Z<360;Z+=40)for(int X:{-1,1})for(int Y:{-1,1})Part(Concrete,G+FVector(X*36,Side*360+Y*36,Z),FVector(70,70,38));
  Part(Concrete,G+FVector(0,Side*360,365),FVector(170,170,30));Part(Concrete,G+FVector(0,Side*360,405),FVector(85,85,50));
  for(int I=0;I<8;I++){
   const float Base=BattleGateSupport::BaseZ[Side<0?0:1][I]-G.Z-4.f,Top=GateFenceTop;
   Part(Iron,G+FVector(100+I*35,Side*350,(Base+Top)*.5f),FVector(6,6,Top-Base));
   if(I==0||I==7){
    Part(Iron,G+FVector(100+I*35,Side*350,(Base+Top)*.5f),FVector(10,10,Top-Base));
    Part(Iron,G+FVector(100+I*35,Side*350,Base+5),FVector(18,18,6));
   }
  }
  for(float Z:{30.f,GateFenceTop-10.f})Part(Iron,G+FVector(220,Side*350,Z),FVector(300,8,8));
 }
 Part(Iron,G+FVector(-80,-360,185),FVector(10,200,90));Label(TEXT("ParkName"),TEXT("PIEDMONT PARK"),G+FVector(-88,-360,198),17,FRotator(0,180,0));
 Label(TEXT("GateName"),TEXT("14TH STREET"),G+FVector(-88,-360,166),14,FRotator(0,180,0));
}
void ABattleTutorial::BeginPlay(){
 Super::BeginPlay();
 // The mapped asphalt replaces these complete legacy cube footprints. Keep the
 // old surface on maps without that exact, untransformed replacement mesh.
 bool bMappedPavement=false,bGatePavement=false;
 for(TActorIterator<AActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("BattlePrideStreet"))||It->ActorHasTag(TEXT("BattleGateRoad"))){
  TInlineComponentArray<UStaticMeshComponent*> Surfaces;It->GetComponents(Surfaces);
  for(const UStaticMeshComponent* Surface:Surfaces)if(Surface->GetStaticMesh()
   &&Surface->GetComponentTransform().Equals(FTransform::Identity,.01f)
   &&Surface->GetCollisionEnabled()!=ECollisionEnabled::NoCollision){
   const FString Path=Surface->GetStaticMesh()->GetPathName();
   bMappedPavement|=Path==TEXT("/Game/BattleForTheA/Environment/PrideIntersection/SM_PiedmontRoad.SM_PiedmontRoad");
   bGatePavement|=Path==TEXT("/Game/BattleForTheA/Environment/GateRoad/SM_GateRoad.SM_GateRoad");
  }
 }
 if((bMappedPavement||bGatePavement)&&PracticeStreet&&PracticeStreet->GetInstanceCount()==BattlePrideRoadCleanup::OriginalCount){
  // Both masks refer to the original instance list. Remove once so the first
  // cleanup cannot shift the indices used by the second replacement.
  TArray<int32> Covered;
  if(bMappedPavement)for(int32 Index:BattlePrideRoadCleanup::Indices)Covered.AddUnique(Index);
  if(bGatePavement)for(int32 Index:BattleGateRoadCleanup::Indices)Covered.AddUnique(Index);
  Covered.Sort();
  const bool bRemoved=PracticeStreet->RemoveInstances(Covered);
  UE_LOG(LogTemp,Display,TEXT("PrideRoadCleanup: removed=%d remaining=%d success=%d"),Covered.Num(),PracticeStreet->GetInstanceCount(),bRemoved);
 }
 // A map-authored extension moves the complete visible/colliding closure together.
 // Maps without this anchor retain the original practice boundaries.
 for(TActorIterator<AActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("PrideStreetBoundary"))&&BoundaryRoots.IsValidIndex(4)){
  const float DeltaYaw=It->GetActorRotation().Yaw-BattleTutorialBlock::BarrierDirections[4].Rotation().Yaw;
  BoundaryRoots[4]->SetWorldLocationAndRotation(It->GetActorLocation(),FRotator(0,DeltaYaw,0));
  UE_LOG(LogTemp,Display,TEXT("PrideBoundary: relocated closure to %s"),*It->GetActorLocation().ToString());break;
 }
 auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* B=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(this,0));if(!M||!B)return;
#if !UE_BUILD_SHIPPING
 for(const TCHAR* Flag:{TEXT("BattleSkipTutorial"),TEXT("BattleFinishAudit"),TEXT("BattleHealthAudit"),TEXT("BattleHomeDriveAudit"),TEXT("BattleHUDReview")})if(FParse::Param(FCommandLine::Get(),Flag)){if(FString(Flag)==TEXT("BattleHUDReview")&&FParse::Param(FCommandLine::Get(),TEXT("BattleTutorialReview")))continue;M->bTutorialActive=false;return;}
#endif
 M->bTutorialActive=true;M->StartCountdown=0;
 GetWorld()->SpawnActor<ABattleMarketClosure>(FVector(-19107.993785,3187.443844,0),FRotator::ZeroRotator);
 FVector P=BattleTutorialData::Road[0]+FVector(0,0,98);FRotator R=(BattleTutorialData::Road[1]-BattleTutorialData::Road[0]).Rotation();
 // Place the bike on the authored surface before the protected opening begins.
 FHitResult StartGround;FCollisionQueryParams StartQuery(SCENE_QUERY_STAT(TutorialStartSurface),false,B);
 if(GetWorld()->LineTraceSingleByChannel(StartGround,P+FVector(0,0,1000),P-FVector(0,0,1000),ECC_Visibility,StartQuery))P.Z=StartGround.ImpactPoint.Z+98.f;
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleTutorialAlternate")))R=(BattleTutorialBlock::Alternate[1]-BattleTutorialBlock::Alternate[0]).Rotation();
#endif
 B->SetActorLocationAndRotation(P,R,false,nullptr,ETeleportType::TeleportPhysics);B->Ride->StopMovementImmediately();B->Ride->bForceNextFloorCheck=true;B->CheckpointTransform=B->GetActorTransform();B->Ride->LastSafeLocation=P;PreviousPosition=P;
 if(auto* C=B->GetController())C->SetControlRotation(R);
 UE_LOG(LogTemp,Display,TEXT("BattleTutorial: active=1 start=%s gate=%s"),*P.ToString(),*BattleTutorialData::Gate.ToString());
}
bool ABattleTutorial::TryStart(FVector Previous,FVector Current){
 auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!M||!M->bTutorialActive||UGameplayStatics::IsGamePaused(this))return false;
 const FVector G=BattleTutorialData::Gate;
 if(Previous.X>=G.X||Current.X<G.X||FMath::Abs(Current.Y-G.Y)>270||FMath::Abs(Current.Z-G.Z)>180)return false;
 auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);auto* B=Cast<ABattleBike>(Pawn);auto* Foot=Cast<ABattleRider>(Pawn);if(Foot)B=Foot->ParkedBike.Get();if(!B||B->RiderHealth<=0||B->Ride->Recovery>0||B->StunRemaining>0)return false;
 M->bTutorialActive=false;M->StartCountdown=3;M->TimeRemaining=M->Difficulty.TimeLimitSeconds;M->RunElapsed=M->RunTopSpeed=0;
 B->HornUses=5;B->RiderHealth=100;B->PistolAmmo=17;B->Inventory[0].Magazine=17;B->Inventory[0].Reserve=0;B->Ride->Wipeouts=0;B->NearMisses=B->EnemyKills=0;
 M->Trouble=0;M->PeopleHit=0;M->bPoliceAlert=false;
 if(Foot){Foot->Health=100;if(Foot->CurrentWeapon==0)Foot->Ammo=17;}
 if(M->Quest)B->CheckpointTransform=M->Quest->InitialStartTransform;
 UE_LOG(LogTemp,Display,TEXT("BattleTutorial: gate crossed; countdown=3 timer=%.0f"),M->TimeRemaining);return true;
}
void ABattleTutorial::Tick(float Dt){
 Super::Tick(Dt);auto* P=UGameplayStatics::GetPlayerPawn(this,0);auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!P||!M)return;
 const FVector Current=P->GetActorLocation();
 if(M->bTutorialActive){
  const float D=FVector::Dist2D(Current,PreviousPosition);if(D<200)M->PracticeDistance+=D;
  if(auto* C=UGameplayStatics::GetPlayerController(this,0)){M->bPracticeBraked|=C->IsInputKeyDown(EKeys::SpaceBar);M->bPracticeHorn|=C->IsInputKeyDown(EKeys::H);M->bPracticeSteered|=C->IsInputKeyDown(EKeys::A)||C->IsInputKeyDown(EKeys::D)||C->IsInputKeyDown(EKeys::Left)||C->IsInputKeyDown(EKeys::Right);}
  M->bPracticeDismounted|=P->IsA<ABattleRider>();
 }
 for(int I=0;I<UE_ARRAY_COUNT(BattleTutorialBlock::BarrierCenters);I++){
  const USceneComponent* Boundary=BoundaryRoots[I];const FVector D=Boundary->GetComponentQuat().RotateVector(BattleTutorialBlock::BarrierDirections[I]),R(-D.Y,D.X,0),Delta=Current-Boundary->GetComponentLocation();
  const float Along=FVector::DotProduct(Delta,D);
  if(Along>-650&&Along<120&&FMath::Abs(FVector::DotProduct(Delta,R))<250)M->ExpansionNoticeRemaining=1;
 }

 TryStart(PreviousPosition,Current);PreviousPosition=Current;
}
