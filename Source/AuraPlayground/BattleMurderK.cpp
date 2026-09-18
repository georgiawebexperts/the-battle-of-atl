#include "BattleMurderK.h"
#include "BattleBike.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "PiedmontPedestrian.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
#include "BattleBenchFire.h"

namespace{
 // The plaza chant. Rotates like the Krog crash shouts so the riot reads even
 // when the rider only sees it for a second on the way past.
 const TCHAR* MurderKChants[]={TEXT("MURDER K, GET OUT!"),TEXT("WHOSE TRAIL? OUR TRAIL!"),TEXT("NO MORE COPS!"),
  TEXT("BURN IT DOWN!"),TEXT("WE WERE HERE FIRST!"),TEXT("SHUT IT DOWN!")};
 // The dance party and the riot own opposite edges of the apron; the middle of
 // the plaza stays rideable so the crowd is a hazard, not a wall.
 const FVector DanceCenter(-1750.f,-560.f,0);
 constexpr float DanceRadius=260.f;
 const FVector RiotCenter(1650.f,560.f,0);
 constexpr float RiotRadius=300.f;
}

ABattleMurderK::ABattleMurderK(){
 PrimaryActorTick.bCanEverTick=true;
 SceneRoot=CreateDefaultSubobject<USceneComponent>(TEXT("MurderKRoot"));RootComponent=SceneRoot;
 Tags.Add(TEXT("MurderKLandmark"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 auto Part=[&](const TCHAR* Name,FVector Loc,FVector Scale,TArray<TObjectPtr<UStaticMeshComponent>>& Group,bool Round=false,bool Collision=true){
  auto* C=CreateDefaultSubobject<UStaticMeshComponent>(Name);C->SetupAttachment(SceneRoot);C->SetStaticMesh(Round?Cylinder.Object:Cube.Object);C->SetRelativeLocation(Loc);C->SetRelativeScale3D(Scale);C->SetCollisionEnabled(Collision?ECollisionEnabled::QueryAndPhysics:ECollisionEnabled::NoCollision);C->SetCollisionProfileName(Collision?TEXT("BlockAll"):TEXT("NoCollision"));Group.Add(C);return C;
 };
 // 725 Ponce is east of the northbound BeltLine, opposite Ponce City Market.
 // Its grocery level sits below a tall raw-concrete office block with a glass wing.
 // The real trail frontage opens into a broad plaza, so the riding line stays wide.
 TrailApron=Part(TEXT("TrailApron"),FVector(0,0,8),FVector(42,14,.16),ConcreteParts);
 StoreMass=Part(TEXT("StoreMass"),FVector(300,-1550,350),FVector(30,10,7),BrickParts);
 OfficeTower=Part(TEXT("OfficeTower"),FVector(450,-1700,1650),FVector(28,8.5,25),ConcreteParts);
 Part(TEXT("RoofCrown"),FVector(450,-1700,2940),FVector(29,9,.65),DarkParts);
 GlassWing=Part(TEXT("GlassWing"),FVector(1320,-1180,1680),FVector(6.5,5.2,20),GlassParts);
 Part(TEXT("StorefrontApron"),FVector(250,-720,7),FVector(36,5,.14),ConcreteParts);
 Part(TEXT("Awning"),FVector(300,-1015,520),FVector(12,.9,.22),RedParts);
 Part(TEXT("StorefrontGlass"),FVector(300,-1025,275),FVector(12,.12,2.4),GlassParts,false,false);
 for(int X=-1150;X<=1850;X+=600)Part(*FString::Printf(TEXT("FacadeColumn%d"),X),FVector(X,-930,650),FVector(.38,.38,6.5),ConcreteParts);
 for(int X=-900;X<=1800;X+=300)for(int Z=900;Z<=2500;Z+=320)Part(*FString::Printf(TEXT("OfficeWindow%d_%d"),X,Z),FVector(X,-1268,Z),FVector(1.05,.10,1.05),GlassParts,false,false);
 for(int X:{-1300,1800}){
  Part(*FString::Printf(TEXT("StickerPole%d"),X),FVector(X,-760,170),FVector(.10,.10,3.4),DarkParts,true);
  for(int Z:{70,135,205})Part(*FString::Printf(TEXT("PoleSticker%d_%d"),X,Z),FVector(X,-760,Z),FVector(.13,.13,.09),RedParts,true,false);
 }
 Part(TEXT("BurnBarrelA"),FVector(-1050,-790,48),FVector(.45,.45,.75),DarkParts,true);
 Part(TEXT("BurnBarrelB"),FVector(1450,-820,48),FVector(.45,.45,.75),DarkParts,true);
 StoreSign=CreateDefaultSubobject<UTextRenderComponent>(TEXT("MurderKSign"));StoreSign->SetupAttachment(SceneRoot);StoreSign->SetRelativeLocation(FVector(300,-995,535));StoreSign->SetRelativeRotation(FRotator(0,90,0));StoreSign->SetHorizontalAlignment(EHTA_Center);StoreSign->SetText(FText::FromString(TEXT("MURDER K")));StoreSign->SetWorldSize(128);StoreSign->SetTextRenderColor(FColor(245,236,216));
 GraffitiSign=CreateDefaultSubobject<UTextRenderComponent>(TEXT("MurderGraffiti"));GraffitiSign->SetupAttachment(SceneRoot);GraffitiSign->SetRelativeLocation(FVector(500,-1260,2660));GraffitiSign->SetRelativeRotation(FRotator(-5,90,-7));GraffitiSign->SetHorizontalAlignment(EHTA_Center);GraffitiSign->SetText(FText::FromString(TEXT("MURDER")));GraffitiSign->SetWorldSize(105);GraffitiSign->SetTextRenderColor(FColor(180,28,35));
 // The DJ stack on the north edge of the apron. Deliberately collisionless: the
 // plaza is already full of bodies and the rig must never be a stuck spot.
 Speaker=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PartySpeaker"));Speaker->SetupAttachment(SceneRoot);Speaker->SetStaticMesh(Cube.Object);Speaker->SetRelativeLocation(DanceCenter+FVector(0,0,58));Speaker->SetRelativeScale3D(FVector(.36f,.26f,.58f));Speaker->SetCollisionEnabled(ECollisionEnabled::NoCollision);Speaker->SetCanEverAffectNavigation(false);
 SpeakerCone=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PartySpeakerCone"));SpeakerCone->SetupAttachment(SceneRoot);SpeakerCone->SetStaticMesh(Cylinder.Object);SpeakerCone->SetRelativeLocation(DanceCenter+FVector(-19,0,58));SpeakerCone->SetRelativeRotation(FRotator(90,0,0));SpeakerCone->SetRelativeScale3D(FVector(.14f,.14f,.03f));SpeakerCone->SetCollisionEnabled(ECollisionEnabled::NoCollision);SpeakerCone->SetCanEverAffectNavigation(false);
 PartyLabel=CreateDefaultSubobject<UTextRenderComponent>(TEXT("PartyLabel"));PartyLabel->SetupAttachment(SceneRoot);PartyLabel->SetRelativeLocation(DanceCenter+FVector(0,0,132));PartyLabel->SetHorizontalAlignment(EHTA_Center);PartyLabel->SetText(FText::FromString(TEXT("MURDER K DJ")));PartyLabel->SetWorldSize(30);PartyLabel->SetTextRenderColor(FColor(255,190,105));
 PartyMusic=CreateDefaultSubobject<UAudioComponent>(TEXT("PartyMusic"));PartyMusic->SetupAttachment(SceneRoot);PartyMusic->bAutoActivate=false;PartyMusic->bOverrideAttenuation=true;PartyMusic->AttenuationOverrides.bAttenuate=true;PartyMusic->AttenuationOverrides.bSpatialize=true;PartyMusic->AttenuationOverrides.AttenuationShapeExtents=FVector(140);PartyMusic->AttenuationOverrides.FalloffDistance=2200;
}
void ABattleMurderK::BeginPlay(){
 Super::BeginPlay();
 auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
 auto Paint=[&](TArray<TObjectPtr<UStaticMeshComponent>>& Parts,FLinearColor Color){
  auto* Mat=UMaterialInstanceDynamic::Create(Base,this);if(Mat)Mat->SetVectorParameterValue(TEXT("Color"),Color);for(auto& Part:Parts)if(Part)Part->SetMaterial(0,Mat);
 };
 Paint(BrickParts,FLinearColor(.20f,.10f,.065f));Paint(DarkParts,FLinearColor(.025f,.035f,.045f));Paint(RedParts,FLinearColor(.55f,.018f,.025f));Paint(GlassParts,FLinearColor(.035f,.13f,.18f));Paint(ConcreteParts,FLinearColor(.30f,.31f,.30f));
 if(auto* Mat=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Environment/FancyRoachMotel/M_Brick.M_Brick")))for(auto& Part:BrickParts)Part->SetMaterial(0,Mat);
 if(auto* Mat=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Environment/FancyRoachMotel/M_Glass.M_Glass")))for(auto& Part:GlassParts)Part->SetMaterial(0,Mat);
 if(auto* Mat=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Environment/KrogApproach/M_KrogApproachConcrete.M_KrogApproachConcrete")))for(auto& Part:ConcreteParts)Part->SetMaterial(0,Mat);
 for(const FVector Local:{FVector(-1050,-790,20),FVector(1450,-820,20)}){
  const FTransform T(FRotator::ZeroRotator,GetActorTransform().TransformPosition(Local));
  if(auto* Fire=GetWorld()->SpawnActorDeferred<ABattleBenchFire>(ABattleBenchFire::StaticClass(),T,this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn)){Fire->Duration=3600;Fire->FinishSpawning(T);}
 }
 // Other audits drive this same world and count populations. Leave the plaza
 // empty for them so their numbers and their camera framings are untouched.
#if !UE_BUILD_SHIPPING
 const bool OwnAudit=FParse::Param(FCommandLine::Get(),TEXT("BattleMurderKAudit"));
 if(FString(FCommandLine::Get()).Contains(TEXT("Audit"))&&!OwnAudit)bPlazaLive=false;
#endif
 if(!bPlazaLive){
  for(auto* Prop:{Speaker.Get(),SpeakerCone.Get()})if(Prop)Prop->SetVisibility(false);
  if(PartyLabel)PartyLabel->SetVisibility(false);
  return;
 }
 if(Speaker)Speaker->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/PiedmontRide/Materials/M_GunMetal.M_GunMetal")));
 if(SpeakerCone)SpeakerCone->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_DiscGlow.M_DiscGlow")));
 if(PartyMusic)if(auto* Theme=LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_BattleATLTheme2.S_BattleATLTheme2"))){PartyMusic->SetSound(Theme);PartyMusic->SetVolumeMultiplier(.22f);PartyMusic->Play();}
 // The chant is live from the moment the world starts so the plaza reads as
 // already in progress when the rider comes over the rise into it.
 ShoutIndex=0;ShoutText=MurderKChants[0];ShoutRemaining=3.8f;
 TopUp();
 UE_LOG(LogTemp,Display,TEXT("BattleMurderK: plaza dancers=%d rioters=%d bodies=%d"),Dancers,Rioters,BodiesDown);
}

// The plaza is the loudest place on the ride, so it is populated the Krog crash
// way: a top-up pass every few seconds keeps the party and the riot at strength
// even as individuals are knocked down or wander off.
FVector ABattleMurderK::GroundAt(const FVector& Local) const{
 const FVector World=GetActorTransform().TransformPosition(Local);
 FHitResult H;FCollisionQueryParams Q(SCENE_QUERY_STAT(MurderKGround),false,this);
 return GetWorld()->LineTraceSingleByChannel(H,World+FVector(0,0,900),World-FVector(0,0,1500),ECC_Visibility,Q)?H.ImpactPoint:World;
}
void ABattleMurderK::TopUp(){
 const int32 WantDancers=6,WantRioters=8,WantBodies=3;
 auto SpawnPed=[&](const FVector Local,float YawOffset,int32 Variant,bool bDancer,bool bSleeper,float Lift){
  const FVector Spot=GroundAt(Local)+FVector(0,0,Lift);
  const FRotator Facing(0,GetActorRotation().Yaw+YawOffset,0);
  const FTransform Transform(Facing,Spot);
  auto* P=GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),Transform,this,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
  if(!P)return (APiedmontPedestrian*)nullptr;
  // Set before FinishSpawning: BeginPlay reads these to pick the pose.
  P->bAmbientSleeper=bSleeper;P->AmbientWakeChance=bSleeper?0.f:.35f;
  if(bDancer){P->bParkDancer=true;P->DanceVariant=Variant;}
  P->CityAppearanceVariant=Variant;P->CityOutfitVariant=Variant;
  P->FinishSpawning(Transform);
  return P;
 };
 for(int32 I=Dancers;I<WantDancers;++I){
  const float A=2*PI*I/WantDancers+.35f;
  const FVector Local(DanceCenter.X+FMath::Cos(A)*DanceRadius,DanceCenter.Y+FMath::Sin(A)*DanceRadius,0);
  auto* P=SpawnPed(Local,(DanceCenter-Local).Rotation().Yaw,I%3,true,false,92.f);
  if(!P)continue;
  DancerList.Add(P);++Dancers;
 }
 for(int32 I=Rioters;I<WantRioters;++I){
  const float A=2*PI*I/WantRioters;
  const float Reach=(I%3)*.2f+.6f;
  const FVector Local(RiotCenter.X+FMath::Cos(A)*RiotRadius*Reach,RiotCenter.Y+FMath::Sin(A)*RiotRadius*Reach,0);
  auto* P=SpawnPed(Local,(RiotCenter-Local).Rotation().Yaw,3+I%4,false,false,92.f);
  if(!P)continue;
  P->HearGunfire(GetActorLocation());RiotList.Add(P);++Rioters;
 }
 const FVector BodySpots[]={FVector(-600,-600,0),FVector(300,620,0),FVector(1050,-600,0)};
 for(int32 I=BodiesDown;I<WantBodies;++I){
  auto* P=SpawnPed(BodySpots[I],I%2?90.f:-90.f,I,false,true,40.f);
  if(!P)continue;
  BodyList.Add(P);++BodiesDown;
 }
}

// Keeps the party dancing and the riot a knot instead of a scatter.
void ABattleMurderK::PollCrowd(float Dt){
 for(int32 I=DancerList.Num()-1;I>=0;--I)
  if(!DancerList[I].IsValid()){DancerList.RemoveAt(I);Dancers=FMath::Max(0,Dancers-1);}
 for(int32 I=RiotList.Num()-1;I>=0;--I){
  auto* P=RiotList[I].Get();
  if(!P||P->IsActorBeingDestroyed()){RiotList.RemoveAt(I);Rioters=FMath::Max(0,Rioters-1);continue;}
  P->HearGunfire(GetActorLocation());
  // Strays are put back on the ring so the plaza never empties out.
  if(FVector::Dist2D(P->GetActorLocation(),GetActorLocation())>4200.f){
   const float A=2*PI*I/FMath::Max(1,RiotList.Num());
   const FVector Local(RiotCenter.X+FMath::Cos(A)*RiotRadius,RiotCenter.Y+FMath::Sin(A)*RiotRadius,0);
   P->SetActorLocation(GroundAt(Local)+FVector(0,0,92.f),false,nullptr,ETeleportType::TeleportPhysics);
  }
 }
 for(int32 I=BodyList.Num()-1;I>=0;--I)
  if(!BodyList[I].IsValid()){BodyList.RemoveAt(I);BodiesDown=FMath::Max(0,BodiesDown-1);}
}

void ABattleMurderK::Tick(float Dt){
 Super::Tick(Dt);
 if(!bPlazaLive)return;
 auto* Viewer=UGameplayStatics::GetPlayerPawn(this,0);
 if(!Viewer)return;
 if(FVector::Dist2D(Viewer->GetActorLocation(),GetActorLocation())>14000.f)return;
 // The DJ sign reads from the ride line whichever way the rider approaches.
 if(PartyLabel)PartyLabel->SetWorldRotation(FRotator(0,(Viewer->GetActorLocation()-PartyLabel->GetComponentLocation()).Rotation().Yaw,0));
 TopUpClock-=Dt;
 if(TopUpClock<=0){TopUpClock=6.f;TopUp();}
 PollCrowd(Dt);
 ShoutClock-=Dt;
 if(ShoutClock<=0){
  ShoutClock=3.8f;ShoutIndex=(ShoutIndex+1)%(int32)UE_ARRAY_COUNT(MurderKChants);
  ShoutText=MurderKChants[ShoutIndex];ShoutRemaining=3.8f;
 }
 ShoutRemaining=FMath::Max(0.f,ShoutRemaining-Dt);
}

void ABattleMurderK::EndPlay(const EEndPlayReason::Type Reason){
 for(auto& P:DancerList)if(IsValid(P.Get()))P->Destroy();
 for(auto& P:RiotList)if(IsValid(P.Get()))P->Destroy();
 for(auto& P:BodyList)if(IsValid(P.Get()))P->Destroy();
 Super::EndPlay(Reason);
}
