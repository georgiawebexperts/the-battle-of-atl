#include "BattleHome.h"
#include "BattleHomeData.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleQuest.h"
#include "PiedmontPathSpline.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
ABattleHome::ABattleHome(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("HomeRoot"));Tags.Add(TEXT("RidePath"));Tags.Add(TEXT("BattleHome"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube")),Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Asphalt(TEXT("/Game/PiedmontRide/Materials/M_Asphalt.M_Asphalt")),White(TEXT("/Game/PiedmontRide/Materials/M_Concrete.M_Concrete")),Wood(TEXT("/Game/BattleForTheA/Furniture/M_BenchWood.M_BenchWood")),Dark(TEXT("/Game/BattleForTheA/Furniture/M_BenchFrame.M_BenchFrame")),Red(TEXT("/Game/BattleForTheA/Materials/M_ColaRed.M_ColaRed")),TextMat(TEXT("/Engine/EngineMaterials/UnlitText.UnlitText"));
 auto Group=[&](const TCHAR* N,UMaterialInterface* M,bool Canopy=false){auto* C=CreateDefaultSubobject<UInstancedStaticMeshComponent>(N);C->SetupAttachment(RootComponent);C->SetStaticMesh(Canopy?Cone.Object:Cube.Object);C->SetMaterial(0,M);C->SetCollisionProfileName(TEXT("BlockAll"));return C;};
 Road=Group(TEXT("WylieRoad"),Asphalt.Object);auto* Concrete=Group(TEXT("WalkwayAndSiding"),White.Object);auto* Timber=Group(TEXT("PorchTimber"),Wood.Object);auto* Iron=Group(TEXT("RoofWindowsFence"),Dark.Object);auto* Door=Group(TEXT("RedFrontDoor"),Red.Object);auto* Canopies=Group(TEXT("RedUmbrellaCanopies"),Red.Object,true);
 auto Span=[&](UInstancedStaticMeshComponent* M,FVector A,FVector B,float Width){const FVector D=B-A;M->AddInstance(FTransform(D.Rotation(),(A+B)*.5f-FVector(0,0,12),FVector((D.Size()+3)/100,Width/100,24.f/100)));};
 for(int I=1;I<UE_ARRAY_COUNT(BattleHomeData::Road);I++)Span(Road,BattleHomeData::Road[I-1],BattleHomeData::Road[I],450);
 for(int I=1;I<UE_ARRAY_COUNT(BattleHomeData::Approach);I++)Span(Concrete,BattleHomeData::Approach[I-1],BattleHomeData::Approach[I],260);
 const FRotator Facing(0,BattleHomeData::South.Rotation().Yaw-90,0);
 const FTransform House(Facing,BattleHomeData::Home);
 auto Part=[&](UInstancedStaticMeshComponent* M,FVector P,FVector Size,FRotator R=FRotator::ZeroRotator){M->AddInstance(FTransform(R,P,Size/100)*House);};
 // Photo-referenced low stucco gable, glazed door, dark rails and red umbrellas.
 Part(Concrete,FVector(0,0,160),FVector(610,800,320));
 for(int Side:{-1,1}){
  Part(Iron,FVector(Side*165,0,405),FVector(370,870,18),FRotator(-Side*30,0,0));
  for(int Z=325;Z<492;Z+=10)Part(Concrete,FVector(0,Side*401,Z),FVector(FMath::Max(10.f,(497-Z)*2/FMath::Tan(FMath::DegreesToRadians(30.f))),12,10));
 }
 Part(Iron,FVector(0,-409,123),FVector(110,15,230));
 for(int X:{-61,61})Part(Timber,FVector(X,-420,123),FVector(12,12,245));
 Part(Timber,FVector(0,-420,245),FVector(134,12,12));
 Part(Concrete,FVector(0,-460,8),FVector(190,120,16));
 // Courtyard sits to the west/right after leaving the south mouth of Krog.
 const float PatioZ=BattleHomeData::Gate.Z-BattleHomeData::Home.Z;
 Part(Concrete,FVector(-750,180,PatioZ-12),FVector(780,1100,24));
 for(int Y=-340;Y<=690;Y+=80)Part(Iron,FVector(-1130,Y,PatioZ+55),FVector(8,8,110));
 Part(Iron,FVector(-1130,175,PatioZ+112),FVector(8,1100,8));
 for(int X=-1050;X<-360;X+=80)Part(Iron,FVector(X,720,PatioZ+55),FVector(8,8,110));
 Part(Iron,FVector(-750,720,PatioZ+112),FVector(780,8,8));
 for(int Y:{-180,470}){
  Part(Timber,FVector(-550,Y,PatioZ+78),FVector(230,130,12));
  for(int Side:{-1,1}){Part(Timber,FVector(-550,Y+Side*110,PatioZ+42),FVector(240,38,10));for(int End:{-1,1})Part(Iron,FVector(-550+End*80,Y+Side*45,PatioZ+35),FVector(10,10,70));}
  Part(Iron,FVector(-550,Y,PatioZ+145),FVector(7,7,290));
  Part(Canopies,FVector(-550,Y,PatioZ+270),FVector(340,340,50));
 }
 auto Label=[&](const TCHAR* N,const TCHAR* T,FVector P,float Size){auto* C=CreateDefaultSubobject<UTextRenderComponent>(N);C->SetupAttachment(RootComponent);C->SetRelativeLocation(House.TransformPosition(P));C->SetRelativeRotation(FRotator(0,Facing.Yaw-90,0));C->SetWorldSize(Size);C->SetText(FText::FromString(T));C->SetTextMaterial(TextMat.Object);C->SetTextRenderColor(FColor(255,229,183));C->SetHorizontalAlignment(EHTA_Center);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);};
 Part(Iron,FVector(0,-421,365),FVector(275,12,94));Label(TEXT("VenueSign"),TEXT("98 ESTORIA"),FVector(0,-431,348),37);
 Part(Iron,FVector(-650,560,PatioZ+150),FVector(380,12,90));Label(TEXT("MorganWelcome"),TEXT("MORGAN IS HERE"),FVector(-650,551,PatioZ+140),27);
 Label(TEXT("PatioWelcome"),TEXT("CABBAGETOWN"),FVector(0,-431,307),18);

}
void ABattleHome::BeginPlay(){Super::BeginPlay();auto* Path=GetWorld()->SpawnActor<APiedmontPathSpline>();if(Path){Path->bArtifactEligible=false;Path->OsmWayId=TEXT("9242978 / 98 Estoria patio approach");Path->Tags.Add(TEXT("BattleHomeRoute"));TArray<FVector> Points;for(const FVector& P:BattleHomeData::Route)Points.Add(P);Path->SetCenterline(Points);}UE_LOG(LogTemp,Display,TEXT("BattleHome: route=%d gate=%s"),UE_ARRAY_COUNT(BattleHomeData::Route),*BattleHomeData::Gate.ToString());}
void ABattleHome::Tick(float Dt){
 Super::Tick(Dt);auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);if(!Mode||!Mode->Quest||!Pawn)return;
 if(!Mode->Quest->bCollected){bTunnelEntered=bTunnelExited=false;return;}if(Mode->Quest->NextCheckpoint<2)return;
 // These are route gates, not precision targets. The playable tunnel is wide
 // enough that a rider can pass either marker away from its surveyed centre.
 auto Near=[&](FVector P){return FVector::Dist2D(Pawn->GetActorLocation(),P)<650&&FMath::Abs(Pawn->GetActorLocation().Z-P.Z)<350;};
 if(Near(BattleHomeData::TunnelEntry))bTunnelEntered=true;if(bTunnelEntered&&Near(BattleHomeData::TunnelExit))bTunnelExited=true;TryFinish();
}
bool ABattleHome::TryFinish(){
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);if(!Mode||!Pawn||!Mode->Quest||!Mode->Quest->bCollected||Mode->Quest->NextCheckpoint<2||!bTunnelExited)return false;
 auto* Bike=Cast<ABattleBike>(Pawn);if(auto* P=Cast<ABattleRider>(Pawn))Bike=P->ParkedBike.Get();if(!Bike||Bike->RiderHealth<=0||Bike->RespawnRemaining>0||Bike->StunRemaining>0||Bike->Ride->Recovery>0)return false;
 // Reaching the patio is the finish. Tables, railings, other riders and the
 // venue itself may block line of sight to the surveyed centre point.
 if(FVector::Dist2D(Pawn->GetActorLocation(),BattleHomeData::Gate)>650||FMath::Abs(Pawn->GetActorLocation().Z-BattleHomeData::Gate.Z)>300)return false;
 return Mode->CompleteRun(Bike);
}
