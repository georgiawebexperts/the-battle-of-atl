#include "BattleTutorial.h"
#include "BattleMarketClosure.h"
#include "BattleTutorialData.h"
#include "BattleTutorialBlock.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleQuest.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Misc/CommandLine.h"
ABattleTutorial::ABattleTutorial(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("TutorialRoot"));Tags.Add(TEXT("RidePath"));Tags.Add(TEXT("BattleTutorial"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> White(TEXT("/Game/PiedmontRide/Materials/M_Concrete.M_Concrete")),Wood(TEXT("/Game/BattleForTheA/Furniture/M_BenchWood.M_BenchWood")),Dark(TEXT("/Game/BattleForTheA/Furniture/M_BenchFrame.M_BenchFrame")),Red(TEXT("/Game/BattleForTheA/Materials/M_ColaRed.M_ColaRed")),Asphalt(TEXT("/Game/PiedmontRide/Materials/M_Asphalt.M_Asphalt")),TextMat(TEXT("/Engine/EngineMaterials/UnlitText.UnlitText"));
 auto Group=[&](const TCHAR* N,UMaterialInterface* Mat){auto* M=CreateDefaultSubobject<UInstancedStaticMeshComponent>(N);M->SetupAttachment(RootComponent);M->SetStaticMesh(Cube.Object);M->SetMaterial(0,Mat);M->SetCollisionProfileName(TEXT("BlockAll"));return M;};
 auto* Concrete=Group(TEXT("HouseStone"),White.Object);auto* Timber=Group(TEXT("Porch"),Wood.Object);auto* Iron=Group(TEXT("IronAndRoof"),Dark.Object);auto* Door=Group(TEXT("Door"),Red.Object);auto* Road=Group(TEXT("PracticeStreet"),Asphalt.Object);
 auto Part=[&](UInstancedStaticMeshComponent* M,FVector P,FVector Size,FRotator R=FRotator::ZeroRotator){M->AddInstance(FTransform(R,P,Size/100));};
 for(int I=1;I<UE_ARRAY_COUNT(BattleTutorialData::Road);I++){const FVector A=BattleTutorialData::Road[I-1],B=BattleTutorialData::Road[I],D=B-A;Part(Road,(A+B)*.5f-FVector(0,0,12),FVector(D.Size()+3,450,24),D.Rotation());}
 auto Span=[&](FVector A,FVector B){const FVector D=B-A;Part(Road,(A+B)*.5f-FVector(0,0,12),FVector(D.Size()+3,450,24),D.Rotation());};
 for(int I=1;I<UE_ARRAY_COUNT(BattleTutorialBlock::Alternate);I++)Span(BattleTutorialBlock::Alternate[I-1],BattleTutorialBlock::Alternate[I]);
 for(int I=0;I<UE_ARRAY_COUNT(BattleTutorialBlock::StubEnds);I+=2)Span(BattleTutorialBlock::StubEnds[I],BattleTutorialBlock::StubEnds[I+1]);
 const FVector H=BattleTutorialData::Home;
 Part(Concrete,H+FVector(0,0,180),FVector(650,850,360));
 for(int Z=45;Z<345;Z+=22)Part(Concrete,H+FVector(0,-428,Z),FVector(650,10,5));
 for(int Side:{-1,1}){
  Part(Iron,H+FVector(Side*170,-434,205),FVector(115,12,140));
  for(int DX:{-1,1})Part(Concrete,H+FVector(Side*170+DX*64,-443,205),FVector(10,9,155));
  for(int DZ:{-1,1})Part(Concrete,H+FVector(Side*170,-443,205+DZ*76),FVector(138,9,10));
  Part(Concrete,H+FVector(Side*170,-446,205),FVector(6,10,138));
  Part(Iron,H+FVector(Side*174,0,445),FVector(390,950,22),FRotator(-Side*25,0,0));
  Part(Timber,H+FVector(Side*280,-520,160),FVector(18,18,290));
 }
 for(int Z=370;Z<526;Z+=12)for(int Side:{-1,1})Part(Concrete,H+FVector(0,Side*426,Z),FVector(FMath::Min(650.f,(527-Z)*2/FMath::Tan(FMath::DegreesToRadians(25.f))),12,12));
 Part(Door,H+FVector(0,-437,137),FVector(115,15,230));Part(Timber,H+FVector(0,-503,22),FVector(680,160,44));Part(Timber,H+FVector(0,-512,323),FVector(700,190,20));
 for(int Side:{-1,1})for(int X=160;X<=430;X+=35)Part(Concrete,H+FVector(Side*X,-600,70),FVector(14,14,140));
 for(int Side:{-1,1})for(int Z:{35,105})Part(Timber,H+FVector(Side*290,-600,Z),FVector(300,12,12));
 Part(Timber,H+FVector(180,-625,70),FVector(14,14,140));Part(Iron,H+FVector(180,-625,145),FVector(100,45,45));
 auto Label=[&](const TCHAR* N,const TCHAR* T,FVector P,float Size,FRotator R){auto* C=CreateDefaultSubobject<UTextRenderComponent>(N);C->SetupAttachment(RootComponent);C->SetRelativeLocation(P);C->SetRelativeRotation(R);C->SetWorldSize(Size);C->SetText(FText::FromString(T));C->SetTextMaterial(TextMat.Object);C->SetTextRenderColor(FColor(255,229,183));C->SetHorizontalAlignment(EHTA_Center);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);};
 Label(TEXT("Mailbox"),TEXT("ELLISON"),H+FVector(180,-650,140),16,FRotator(0,-90,0));
 for(int I=0;I<UE_ARRAY_COUNT(BattleTutorialBlock::BarrierCenters);I++){
  const FVector C=BattleTutorialBlock::BarrierCenters[I],D=BattleTutorialBlock::BarrierDirections[I],Right(-D.Y,D.X,0);const FRotator R=D.Rotation();
  // Closely spaced welded-mesh fence: the visible bars themselves block bikes and walkers.
  for(int Y=-1000;Y<=1000;Y+=45)Part(Iron,C+Right*Y+FVector(0,0,180),FVector(10,7,360),R);
  for(int Z=30;Z<=350;Z+=40)Part(Iron,C+FVector(0,0,Z),FVector(10,2050,6),R);
  Part(Door,C-D*16+FVector(0,0,180),FVector(20,740,145),R);
  Part(Concrete,C-D*29+FVector(0,0,103),FVector(8,740,14),R);
  for(int Side:{-1,1}){Part(Concrete,C+Right*Side*440+FVector(0,0,35),FVector(150,150,70),R);Part(Door,C+Right*Side*440+FVector(0,0,78),FVector(55,55,45),R);}
  Label(*FString::Printf(TEXT("Closure%d"),I),TEXT("UNDER CONSTRUCTION"),C-D*31+FVector(0,0,190),33,(-D).Rotation());
  Label(*FString::Printf(TEXT("Expansion%d"),I),TEXT("MORE MIDTOWN COMING SOON"),C-D*31+FVector(0,0,142),22,(-D).Rotation());
 }
 // Stone piers and open iron wings leave the central ride-through clear.
 const FVector G=BattleTutorialData::Gate;
 for(int Side:{-1,1}){
  for(int Z=20;Z<360;Z+=40)for(int X:{-1,1})for(int Y:{-1,1})Part(Concrete,G+FVector(X*36,Side*360+Y*36,Z),FVector(70,70,38));
  Part(Concrete,G+FVector(0,Side*360,365),FVector(170,170,30));Part(Concrete,G+FVector(0,Side*360,405),FVector(85,85,50));
  for(int I=0;I<8;I++)Part(Iron,G+FVector(100+I*35,Side*350,120),FVector(6,6,240));
  Part(Iron,G+FVector(220,Side*350,230),FVector(300,8,8));
 }
 Part(Iron,G+FVector(-80,-360,275),FVector(10,200,105));Label(TEXT("ParkName"),TEXT("PIEDMONT PARK"),G+FVector(-88,-360,280),19,FRotator(0,180,0));
 Label(TEXT("GateName"),TEXT("14TH STREET"),G+FVector(-88,-360,244),16,FRotator(0,180,0));
}
void ABattleTutorial::BeginPlay(){
 Super::BeginPlay();
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleMarketClosureReview")))GetWorld()->SpawnActor<ABattleMarketClosure>(FVector(-19107.993785,3187.443844,0),FRotator::ZeroRotator);
#endif
 auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* B=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(this,0));if(!M||!B)return;
#if !UE_BUILD_SHIPPING
 for(const TCHAR* Flag:{TEXT("BattleSkipTutorial"),TEXT("BattleFinishAudit"),TEXT("BattleHealthAudit"),TEXT("BattleHomeDriveAudit"),TEXT("BattleHUDReview")})if(FParse::Param(FCommandLine::Get(),Flag)){if(FString(Flag)==TEXT("BattleHUDReview")&&FParse::Param(FCommandLine::Get(),TEXT("BattleTutorialReview")))continue;M->bTutorialActive=false;return;}
#endif
 M->bTutorialActive=true;M->StartCountdown=0;
 const FVector P=BattleTutorialData::Road[0]+FVector(0,0,98);FRotator R=(BattleTutorialData::Road[1]-BattleTutorialData::Road[0]).Rotation();
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
 B->HornUses=5;B->RiderHealth=100;B->PistolAmmo=10;B->Inventory[0].Magazine=10;B->Inventory[0].Reserve=0;B->Ride->Wipeouts=0;B->NearMisses=B->EnemyKills=0;
 M->Trouble=0;M->PeopleHit=0;M->bPoliceAlert=false;
 if(Foot){Foot->Health=100;if(Foot->CurrentWeapon==0)Foot->Ammo=10;}
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
  const FVector D=BattleTutorialBlock::BarrierDirections[I],R(-D.Y,D.X,0),Delta=Current-BattleTutorialBlock::BarrierCenters[I];
  const float Along=FVector::DotProduct(Delta,D);
  if(Along>-650&&Along<120&&FMath::Abs(FVector::DotProduct(Delta,R))<250)M->ExpansionNoticeRemaining=1;
 }

 TryStart(PreviousPosition,Current);PreviousPosition=Current;
}
