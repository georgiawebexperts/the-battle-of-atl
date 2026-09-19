#include "BattleSpirit.h"
#include "BattleSpiritData.h"
#include "BattleBike.h"
#include "PiedmontBike.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/BillboardComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"
ABattleSpirit::ABattleSpirit(){
 PrimaryActorTick.bCanEverTick=true;
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("SpiritRoot"));
 SetCanBeDamaged(false);SetActorEnableCollision(false);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 auto Part=[&](const TCHAR* Name,UStaticMesh* Mesh,FVector Loc,FVector Scale,FRotator Rot=FRotator::ZeroRotator){auto* C=CreateDefaultSubobject<UStaticMeshComponent>(Name);C->SetupAttachment(RootComponent);C->SetStaticMesh(Mesh);C->SetRelativeLocation(Loc);C->SetRelativeScale3D(Scale);C->SetRelativeRotation(Rot);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCanEverAffectNavigation(false);C->SetCastShadow(false);FigureParts.Add(C);return C;};
 // A black bear, built so it reads as a bear from the trail. The encounter is on
 // the southbound approach to Murder K and Elliott rode straight past the old
 // figure and could only call it "the bear or whatever that is looks bad": nine
 // opaque primitive parts standing upright, roughly person sized, with no
 // silhouette to name. Design/BLACK-BEAR-SPIRIT.md asks for a black bear with a
 // dark translucent body, a fine silver-blue rim and a faint drifting trail, and
 // warns against an unreviewed primitive placeholder.
 //
 // Bear proportions, side on: a long low barrel, a shoulder hump that is the
 // shape that says "bear", a heavy head carried low with a snout and round ears,
 // four planted legs and a short tail. 1.9 m nose to tail and 1.1 m at the
 // withers, which is a black bear next to a 1.8 m rider - the first pass was
 // about half that and read as a pile of stones.
 Part(TEXT("BearBody"),Sphere.Object,FVector(0,0,74),FVector(1.05f,.50f,.46f));
 Part(TEXT("BearHump"),Sphere.Object,FVector(-34,0,100),FVector(.34f,.40f,.30f));
 Part(TEXT("BearRump"),Sphere.Object,FVector(58,0,76),FVector(.44f,.44f,.38f));
 Part(TEXT("BearChest"),Sphere.Object,FVector(-62,0,72),FVector(.36f,.42f,.36f));
 Part(TEXT("BearNeck"),Sphere.Object,FVector(-92,0,78),FVector(.26f,.28f,.26f));
 Part(TEXT("BearHead"),Sphere.Object,FVector(-118,0,76),FVector(.31f,.28f,.26f));
 Part(TEXT("BearSnout"),Cone.Object,FVector(-148,0,64),FVector(.13f,.13f,.21f),FRotator(-90,0,0));
 for(int Side:{-1,1})Part(*FString::Printf(TEXT("BearEar%d"),Side),Sphere.Object,FVector(-126,Side*19,96),FVector(.10f,.06f,.10f));
 for(int32 I=0;I<4;I++)Part(*FString::Printf(TEXT("BearLeg%d"),I),Cylinder.Object,FVector((I<2)?-52.f:54.f,(I%2)?23.f:-23.f,27),FVector(.17f,.17f,.55f));
 Part(TEXT("BearTail"),Sphere.Object,FVector(86,0,76),FVector(.10f,.10f,.10f));
 for(int Side:{-1,1})Part(*FString::Printf(TEXT("BearEye%d"),Side),Sphere.Object,FVector(-134,Side*11,82),FVector(.04f,.04f,.04f));
 auto Wisp=[&](const TCHAR* Name,FVector Loc,float Scale){auto* C=CreateDefaultSubobject<UStaticMeshComponent>(Name);C->SetupAttachment(RootComponent);C->SetStaticMesh(Sphere.Object);C->SetRelativeLocation(Loc);C->SetRelativeScale3D(FVector(Scale));C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCanEverAffectNavigation(false);C->SetCastShadow(false);Wisps.Add(C);};
 Wisp(TEXT("SpiritWispA"),FVector(-65,-85,145),.10f);Wisp(TEXT("SpiritWispB"),FVector(20,80,190),.075f);Wisp(TEXT("SpiritWispC"),FVector(105,-65,210),.055f);
 // Five more, for the drifting trail: the design note asks for a faint trail
 // behind the animal, and a trail needs more than three points to read as a
 // path rather than as three floating specks.
 for(int32 I=0;I<5;I++)Wisp(*FString::Printf(TEXT("SpiritTrail%d"),I),FVector(-140-60*I,0,60),.09f-.012f*I);
 // The painted spectral bear, which is the art the design note asked for and
 // which was already authored for this scene. It is unlit and two-sided, so it
 // reads the same from any angle the rider approaches from. Sized to a real
 // black bear next to a rider: about 2.7 m of card, 1.85 m tall, feet on the
 // ground. The primitive figure below is only the fallback if this art fails to
 // load, because an opaque primitives bear is what looked bad in the first
 // place.
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Plane(TEXT("/Engine/BasicShapes/Plane.Plane"));
 SpiritCard=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpectralBlackBear"));SpiritCard->SetupAttachment(RootComponent);SpiritCard->SetStaticMesh(Plane.Object);SpiritCard->SetRelativeLocation(FVector(0,0,92));SpiritCard->SetRelativeRotation(FRotator(90,0,0));SpiritCard->SetRelativeScale3D(FVector(2.70f,1.85f,1));SpiritCard->SetCollisionEnabled(ECollisionEnabled::NoCollision);SpiritCard->SetCanEverAffectNavigation(false);SpiritCard->SetCastShadow(false);SpiritCard->SetVisibility(false);
 static ConstructorHelpers::FObjectFinder<UTexture2D> SpiritTexture(TEXT("/Game/BattleForTheA/Spirit/T_SpectralBlackBear.T_SpectralBlackBear"));
 // M_BearCard, not M_SpectralBlackBear: the older material renders as the engine
 // default checker at run time even though its texture sample is wired and the
 // texture is present. This one is built from a freshly re-encoded PNG.
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> SpiritMaterial(TEXT("/Game/BattleForTheA/Spirit/M_SpectralBlackBear.M_SpectralBlackBear"));
 if(SpiritMaterial.Succeeded())SpiritCard->SetMaterial(0,SpiritMaterial.Object);
SpiritBillboard=CreateDefaultSubobject<UBillboardComponent>(TEXT("SpectralBlackBearBillboard"));SpiritBillboard->SetupAttachment(RootComponent);SpiritBillboard->SetSprite(SpiritTexture.Object);SpiritBillboard->SetRelativeLocation(FVector(0,0,110));SpiritBillboard->SetRelativeScale3D(FVector(1.15f));SpiritBillboard->SetCollisionEnabled(ECollisionEnabled::NoCollision);SpiritBillboard->SetCanEverAffectNavigation(false);SpiritBillboard->SetCastShadow(false);SpiritBillboard->SetVisibility(false);
 // The 3D body, and the reason it exists: the card above is a flat cut-out, so
 // from the bike it reads as a painted sign standing in the trail. A real
 // quadruped reads as a bear from every angle the rider can approach from.
 // Found on 2026-09-19 (OpenGameArt, CC-BY 4.0 - see
 // Design/BLACK-BEAR-SPIRIT.md for the licence and the credit it requires).
 // Hard references on purpose: an asset this project loads by string does not
 // get cooked, and a material that fails to load falls back to the engine's
 // checker, which is the failure that made the card look wrong in the first
 // place.
 static ConstructorHelpers::FObjectFinder<UStaticMesh> BearBodyMesh(TEXT("/Game/BattleForTheA/Spirit/SM_SpectralBear.SM_SpectralBear"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> BearBodySource(TEXT("/Game/BattleForTheA/Spirit/M_SpectralBearBody.M_SpectralBearBody"));
 if(BearBodyMesh.Succeeded()){
  BearBody=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BearBodyMesh"));
  BearBody->SetupAttachment(RootComponent);
  BearBody->SetStaticMesh(BearBodyMesh.Object);
  // Authored nose along +Y and standing on its own origin; the actor faces +X,
  // so the nose is yawed forward and the body scaled to 1.9 m nose to tail.
  const float BearScale=0.334f;
  const FRotator BearFacing(0,-90,0);
  // The file's pivot is not at the animal: its bounding box centre sits 279 cm
  // along local X with the feet 7 cm above the origin. Cancelling that from the
  // bounds, rather than from a number measured once, keeps the body on the actor
  // if the mesh is ever replaced.
  const FBoxSphereBounds BearBounds=BearBodyMesh.Object->GetBounds();
  const FVector BearFeetCentre(BearBounds.Origin.X,BearBounds.Origin.Y,BearBounds.Origin.Z-BearBounds.BoxExtent.Z);
  BearBody->SetRelativeLocation(-BearFacing.RotateVector(BearFeetCentre*BearScale));
  BearBody->SetRelativeRotation(BearFacing);
  BearBody->SetRelativeScale3D(FVector(BearScale));
  BearBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  BearBody->SetCanEverAffectNavigation(false);
  BearBody->SetCastShadow(false);
  if(BearBodySource.Succeeded())BearBody->SetMaterial(0,BearBodySource.Object);
  BearBody->SetVisibility(false);
  // Two small cold eyes on the body. A rim says "something is there"; eyes say
  // "an animal". They are attached to the body component rather than to the
  // actor, so they sit on the mesh's own axes - the first attempt placed them
  // in actor coordinates and put them inside the chest, which is why the last
  // review render had a bear with no face. Their position comes from the mesh's
  // own bounds: the file is authored nose along +Y, so the head is at the far
  // +Y end and a little above the middle of the body.
  const FVector HeadLocal(
   BearBounds.Origin.X,
   BearBounds.Origin.Y+BearBounds.BoxExtent.Y*0.60f,
   BearBounds.Origin.Z+BearBounds.BoxExtent.Z*0.26f);
  for(int Side:{-1,1}){
   auto* Eye=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("SpiritBodyEye%d"),Side));
   Eye->SetupAttachment(BearBody);
   Eye->SetStaticMesh(Sphere.Object);
   Eye->SetRelativeLocation(HeadLocal+FVector(Side*BearBounds.BoxExtent.X*0.15f,0,0));
   // The body is scaled to 0.334, so an eye authored at 5.5 cm of world size is
   // 5.5/0.334 in the body's own units.
   Eye->SetRelativeScale3D(FVector(.055f/BearScale));
   Eye->SetCollisionEnabled(ECollisionEnabled::NoCollision);
   Eye->SetCanEverAffectNavigation(false);
   Eye->SetCastShadow(false);
   Eye->SetVisibility(false);
   BodyEyes.Add(Eye);
  }
 }
 // Two lights, because the body cannot emit on its own: the material that would
 // have made it glow is the one this project's scripts cannot get drawn (see the
 // note on BearBodyMaterial below). A light inside the chest lights the animal
 // from within, and a second, wider one above it catches the back and reads as
 // moonlight along the shoulders - which is the rim the design note asks for,
 // made out of light instead of a shader.
MoonGlow=CreateDefaultSubobject<UPointLightComponent>(TEXT("SpiritMoonGlow"));MoonGlow->SetupAttachment(RootComponent);MoonGlow->SetRelativeLocation(FVector(25,0,115));MoonGlow->SetLightColor(FLinearColor(.10f,.42f,1.f));MoonGlow->SetAttenuationRadius(300);MoonGlow->SetIntensity(0);MoonGlow->SetCastShadows(false);
SpiritBackGlow=CreateDefaultSubobject<UPointLightComponent>(TEXT("SpiritBackGlow"));SpiritBackGlow->SetupAttachment(RootComponent);SpiritBackGlow->SetRelativeLocation(FVector(-70,0,215));SpiritBackGlow->SetLightColor(FLinearColor(.15f,.52f,1.f));SpiritBackGlow->SetAttenuationRadius(760);SpiritBackGlow->SetIntensity(0);SpiritBackGlow->SetCastShadows(false);
 // Card visible, primitive bear hidden. If the material or the mesh is missing
 // the card is skipped at runtime and the primitives carry the scene.
 for(auto& C:FigureParts)C->SetVisibility(false);for(auto& C:Wisps)C->SetVisibility(false);for(auto& C:BodyEyes)C->SetVisibility(false);
}
bool ABattleSpirit::IsLiveRun() const {
 const auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));
 return M&&!M->bTutorialActive&&!M->bRunEnded&&M->StartCountdown<=0&&M->TimeRemaining>0&&!UGameplayStatics::IsGamePaused(this);
}
ABattleBike* ABattleSpirit::MountedRider() const {
 auto* B=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(this,0));
 if(!B||!B->GetController()||B->bParked||B->RiderHealth<=0||B->RespawnRemaining>0||B->StunRemaining>0||B->Ride->Recovery>0)return nullptr;
 for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->ContainsBike(B->GetActorLocation()))return nullptr;
 return B;
}
bool ABattleSpirit::TryApproach(float Roll){
 if(State!=EBattleSpiritState::Untried||!IsLiveRun()||!FMath::IsFinite(Roll)||Roll<0||Roll>=1)return false;
 State=EBattleSpiritState::Absent;
 if(!MountedRider()||Roll>=.50f)return false;
 State=EBattleSpiritState::Appearing;Age=Fade=0;return true;
}
void ABattleSpirit::Cancel(){
 if(State==EBattleSpiritState::Appearing||State==EBattleSpiritState::Active){State=EBattleSpiritState::Fading;Fade=0;}
}
void ABattleSpirit::CancelForRider(const UObject* Context){
 if(!Context||!Context->GetWorld())return;
 for(TActorIterator<ABattleSpirit> It(Context->GetWorld());It;++It)It->Cancel();
}
bool ABattleSpirit::TryCatch(){
 if(State!=EBattleSpiritState::Active||!IsLiveRun())return false;
 auto* B=MountedRider();if(!B){Cancel();return false;}
 if(FVector::Dist2D(B->GetActorLocation(),GetActorLocation())>160||FMath::Abs(B->GetActorLocation().Z-GetActorLocation().Z)>160)return false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(SpiritCatch),false,this);Q.AddIgnoredActor(B);FHitResult H;
 if(GetWorld()->LineTraceSingleByChannel(H,B->GetActorLocation(),GetActorLocation(),ECC_Visibility,Q))return false;
 auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));
 B->RestoreRiderHealth(100);
 const float Added=FMath::Max(0.f,M->Difficulty.TimeLimitSeconds-M->TimeRemaining);
 if(Added>0)M->AdjustRunTime(Added,TEXT("TIME RESTORED"));
 else {M->LastTimeDelta=0;M->TimeNotice=TEXT("TIME RESTORED");M->TimeNoticeRemaining=2.5f;}
 Rewards++;Cancel();return true;
}
void ABattleSpirit::AdvanceEncounter(float Dt){
 if(!FMath::IsFinite(Dt)||Dt<=0||UGameplayStatics::IsGamePaused(this))return;
 if(State==EBattleSpiritState::Fading){Fade+=Dt;if(Fade>=1.5f)State=EBattleSpiritState::Resolved;return;}
 if(State!=EBattleSpiritState::Appearing&&State!=EBattleSpiritState::Active)return;
 if(!IsLiveRun()||!MountedRider()){Cancel();return;}
 Age+=Dt;
 if(Age>=12){Cancel();return;}
 if(Age>=1.5f)State=EBattleSpiritState::Active;
}
void ABattleSpirit::Tick(float Dt){
 Super::Tick(Dt);
 if(!bPresentationReady)return;
 auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);
 // The encounter used to fire on a coin flip, so half the time a rider who rode
 // right past it saw nothing at all. Elliott did not see the angel; now it
 // always appears when he gets near it.
 if(State==EBattleSpiritState::Untried&&Pawn&&FVector::Dist2D(Pawn->GetActorLocation(),ApproachPoint)<900&&FMath::Abs(Pawn->GetActorLocation().Z-ApproachPoint.Z)<200){
  if(ChaseRoute.Num()>1){SetActorLocation(ChaseRoute[0]);TryApproach(0.f);}
 }
 AdvanceEncounter(Dt);
 if(State==EBattleSpiritState::Active){
  RouteDistance+=110*Dt;float Left=RouteDistance;
  for(int32 I=1;I<ChaseRoute.Num();I++){const FVector D=ChaseRoute[I]-ChaseRoute[I-1];const float Length=D.Size();if(Left<=Length){SetActorLocation(ChaseRoute[I-1]+D.GetSafeNormal()*Left);SetActorRotation(D.Rotation());break;}Left-=Length;if(I==ChaseRoute.Num()-1)SetActorLocation(ChaseRoute.Last());}
  TryCatch();
 }
 const bool Visible=State==EBattleSpiritState::Appearing||State==EBattleSpiritState::Active||State==EBattleSpiritState::Fading;
 const float Reveal=State==EBattleSpiritState::Appearing?FMath::Clamp(Age/1.5f,0.f,1.f):(State==EBattleSpiritState::Fading?1.f-FMath::Clamp(Fade/1.5f,0.f,1.f):(State==EBattleSpiritState::Active?1.f:0.f));
 // The presentation is the 3D figure, wearing M_SpectralFigure: a dark unlit
 // body with a silver-blue rim that brightens at grazing angles.
 //
 // Elliott's "the bear or angel or whatever still doesnt look good" was the
 // painted card, and the render says why: the plane and the material were both
 // there (the log prints M_SpectralBlackBear) but the material sampled the
 // engine's default checker, so a 2.7 m pale panel floated beside the trail. The
 // billboard that draws the same art is editor-only, so it cannot carry the
 // scene in game either. A card also reads from exactly one angle; a body reads
 // from all of them. The card and the sprite stay as hidden references.
 SpiritCard->SetVisibility(false);
 SpiritBillboard->SetVisibility(false);
 const float FigureScale=Visible?FMath::Lerp(.82f,1.f,Reveal):1.f;
 // Body first, and the card and the primitives only as fallbacks: if the mesh
 // asset is missing the actor still has something to show rather than nothing.
 const bool bCardMode=FParse::Param(FCommandLine::Get(),TEXT("BattleSpiritCard"));
 const bool bBody=BearBody!=nullptr&&!bCardMode;
 if(BearBody){
  BearBody->SetVisibility(Visible&&bBody);
  // The reveal grows the body into place and the dissolve fades it out through
  // the material's SpiritFade scalar, so the encounter's own timing drives the
  // look rather than a second clock.
  if(Visible&&bBody){
   BearBody->SetRelativeScale3D(FVector(0.334f*FMath::Lerp(.86f,1.f,Reveal)));
   if(BearBodyMaterial)BearBodyMaterial->SetScalarParameterValue(TEXT("SpiritFade"),FMath::Clamp(Reveal,0.f,1.f));
  }
 }
 // -BattleSpiritCard shows the painted card instead, so the two presentations can
 // be compared in one build while the card's material is being sorted out.
 if(bCardMode){
  const float S=Visible?FMath::Lerp(.80f,1.f,Reveal):1.f;
  SpiritCard->SetRelativeScale3D(FVector(2.70f,1.85f,1)*S);
  SpiritCard->SetRelativeLocation(FVector(0,0,92.f*S));
  SpiritCard->SetVisibility(Visible);
 }
 for(auto& C:FigureParts){const bool bShow=Visible&&!bCardMode&&!bBody;C->SetVisibility(bShow);if(bShow)C->SetRelativeScale3D(FVector(FigureScale));}
 for(auto& C:BodyEyes){if(auto* E=C.Get())E->SetVisibility(Visible&&bBody);}
 // The trail is a record of where the body has been: sample the position while
 // the encounter is live, then place the last five samples behind the animal,
 // shrinking as they age. They are world-space, so they hang back where the
 // bear was rather than following it around.
 TrailClock+=Dt;
 if(Visible&&TrailClock>.22f){
  TrailClock=0;
  TrailPoints.Insert(GetActorLocation()-GetActorForwardVector()*40.f+FVector(0,0,70),0);
  while(TrailPoints.Num()>5)TrailPoints.RemoveAt(TrailPoints.Num()-1);
 }
 if(!Visible)TrailPoints.Reset();
 const int32 AmbientWisps=FMath::Max(0,Wisps.Num()-5);
 for(int32 I=0;I<Wisps.Num();I++){
  auto* C=Wisps[I].Get();if(!C)continue;
  const bool bTrail=I>=AmbientWisps;
  if(!bTrail){C->SetVisibility(Visible);if(Visible)C->AddLocalOffset(FVector(0,0,FMath::Sin(GetWorld()->GetTimeSeconds()*2.f+I)*Dt*12.f));continue;}
  const int32 Age=I-AmbientWisps;
  if(!Visible||Age>=TrailPoints.Num()){C->SetVisibility(false);continue;}
  C->SetVisibility(true);
  C->SetWorldLocation(TrailPoints[Age]);
  const float T=1.f-float(Age)/float(FMath::Max(1,TrailPoints.Num()));
  C->SetWorldScale3D(FVector((0.04f+0.07f*T)*Reveal));
 }
 // The cold light that used to sit here was 3200 at a 900 cm radius, which lit
 // the deck under the bear to a white blowout - in the review capture it read
 // as a headlight on the ground rather than as a spirit, and it washed out the
 // animal's own legs. The body carries its own emissive now, so this only has
 // to be the faint pool of cold light the design note asks for.
 const float Pulse=.85f+.15f*FMath::Sin(GetWorld()->GetTimeSeconds()*5.f);
 // The core light carries the animal and a small pool of cold light on the
 // ground under it; the back light is what makes the silhouette read from any
 // approach angle, which a view-dependent rim material never did.
 MoonGlow->SetVisibility(Visible);MoonGlow->SetIntensity(Visible?950.f*Reveal*Pulse:0.f);
 SpiritBackGlow->SetVisibility(Visible);SpiritBackGlow->SetIntensity(Visible?340.f*Reveal*Pulse:0.f);
}

void ABattleSpirit::BeginPlay(){
 Super::BeginPlay();ApproachPoint=BattleSpiritData::Approach;for(const FVector& P:BattleSpiritData::Chase)ChaseRoute.Add(P);
 // The authored route was captured against an older ground surface, and part of
 // it now sits inside the Murder K trail apron, so the encounter triggered from
 // inside the scenery. Snap the approach and the chase line onto the ground that
 // is actually there now, the same way every placed prop does.
 auto Ground=[&](const FVector& P,FVector& Out)->bool{
  FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(SpiritGround),true);
  if(!GetWorld()->LineTraceSingleByChannel(Hit,P+FVector(0,0,1500),P-FVector(0,0,1500),ECC_Visibility,Q))return false;
  Out=Hit.ImpactPoint;return true;
 };
 FVector Snapped;
 if(Ground(BattleSpiritData::Approach,Snapped)){
  if(FMath::Abs(Snapped.Z-ApproachPoint.Z)>2.f)UE_LOG(LogTemp,Display,TEXT("BattleSpiritGround: approach moved %.0f cm from %s to %s"),Snapped.Z-ApproachPoint.Z,*ApproachPoint.ToString(),*Snapped.ToString());
  ApproachPoint=Snapped;
 }
 ChaseRoute.Reset();
 int32 Moved=0;
 for(const FVector& P:BattleSpiritData::Chase){
  FVector Q2;
  if(Ground(P,Q2)){if(FMath::Abs(Q2.Z-P.Z)>2.f)++Moved;ChaseRoute.Add(Q2);}
  else ChaseRoute.Add(P);
 }
 UE_LOG(LogTemp,Display,TEXT("BattleSpiritGround: chase_points=%d regrounded=%d"),ChaseRoute.Num(),Moved);
 auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
 // Tinted at runtime from an engine material, not from M_SpectralFigure. Every
 // material this project's editor scripts create - M_SpectralFigure included,
 // with no texture sample anywhere in it - renders as the engine's default
 // checker in game, so the drawn look has to come from a material that already
 // works. A near-black blue body under a cold rim light reads as spectral, and
 // it is the tint the figure shipped with before the painted card was tried.
 auto* FigureMat=UMaterialInstanceDynamic::Create(Base,this);
 if(FigureMat)FigureMat->SetVectorParameterValue(TEXT("Color"),FLinearColor(.020f,.045f,.105f));
 for(auto& C:FigureParts)C->SetMaterial(0,FigureMat);
auto* WispMat=UMaterialInstanceDynamic::Create(Base,this);if(WispMat)WispMat->SetVectorParameterValue(TEXT("Color"),FLinearColor(.08f,.55f,1.f));
 for(auto& C:Wisps)C->SetMaterial(0,WispMat);
 // The eyes stay the one bright thing on the figure: small, no outline, and
 // emissive so they glow rather than sitting in the bear's own shadow as a dark
 // bead - which is what the first body-eyed pass produced. EmissiveMeshMaterial
 // is an engine material, and engine materials are the ones that draw here.
 auto* EyeMat=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/EngineMaterials/EmissiveMeshMaterial.EmissiveMeshMaterial"));
 for(auto& C:BodyEyes)if(auto* E=C.Get())E->SetMaterial(0,EyeMat?EyeMat:WispMat);
 UE_LOG(LogTemp,Display,TEXT("BattleSpiritVisual: eye_material=%s"),*GetNameSafe(EyeMat?EyeMat:WispMat));
 // The body is drawn with an ENGINE material, and that is now a measured
 // decision rather than a superstition.
 //
 // Three passes at a spectral look for this bear failed, and on 2026-09-19 the
 // reason was isolated with one experiment instead of three pictures: the
 // authored material was rebuilt as unlit emissive PURE RED - a value no engine
 // default, no sunlight and no tone mapper can produce by accident - and the
 // bear still rendered grey. The material is simply not drawn. Engine materials
 // are: the same mesh, component and slot drew a dark navy body from the
 // BasicShapeMaterial tint on build 144.
 //
 // So M_SpectralBearBody stays in the repo as the authored intent (a dark body
 // with a fine silver-blue rim, with the reveal wired through SpiritFade), and
 // Scripts/debug_spirit_material_red.py stays as the experiment that proves what
 // happens to it. The look in the meantime comes from material that draws: a
 // tinted engine body under a cold light, plus emissive eyes.
 if(BearBody){
  // The authored graph first, and this is a correction rather than a repeat of
  // the 2026-09-19 morning: the body DID draw as authored once. The review
  // capture at 09:25 shows a dark body with the cold rim along its upper edge,
  // taken while this code preferred M_SpectralBearBody. What followed - the
  // "it is never drawn" conclusion - rested on one experiment that deleted the
  // material while the mesh still referenced it and then rebuilt it at the same
  // path, which is exactly the pattern the vault already warns about, so that
  // result was the experiment's fault rather than the material's.
  //
  // What is still true: nothing here is set up to notice a material that does
  // not draw, the painted card's checker is an unset texture parameter sampling
  // the engine's default checker texture rather than a missing material, and the
  // engine fallback stays because it is the one thing known to draw if the
  // authored graph ever does not.
  auto* Authored=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Spirit/M_SpectralBearBody.M_SpectralBearBody"));
  auto* Source=Authored?Authored:LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
  if(!Source)Source=BearBody->GetMaterial(0);
  BearBodyMaterial=Source?UMaterialInstanceDynamic::Create(Source,this):nullptr;
  // Only the engine fallback has a colour parameter; the authored graph carries
  // its own values and is driven by SpiritFade.
  if(BearBodyMaterial&&!Authored)BearBodyMaterial->SetVectorParameterValue(TEXT("Color"),FLinearColor(.050f,.068f,.125f));
  if(BearBodyMaterial)BearBody->SetMaterial(0,BearBodyMaterial);
  UE_LOG(LogTemp,Display,TEXT("BattleSpiritVisual: bear_body=%s material=%s source=%s authored=%d eyes=%d scale=%.3f"),
   *GetNameSafe(BearBody->GetStaticMesh()),*GetNameSafe(BearBody->GetMaterial(0)),*GetNameSafe(Source),Authored?1:0,BodyEyes.Num(),BearBody->GetRelativeScale3D().X);
 }else{
  UE_LOG(LogTemp,Warning,TEXT("BattleSpiritVisual: no bear body mesh - falling back to the card"));
 }
 UE_LOG(LogTemp,Display,TEXT("BattleSpiritVisual: texture=%s material=%s"),*GetNameSafe(SpiritBillboard->Sprite),*GetNameSafe(SpiritCard->GetMaterial(0)));
}
