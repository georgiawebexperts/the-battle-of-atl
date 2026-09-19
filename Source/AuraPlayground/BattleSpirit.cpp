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
 // four planted legs and a short tail. About 2 m nose to tail and 80 cm at the
 // shoulder, which is a black bear next to a 1.8 m rider.
 Part(TEXT("BearBody"),Sphere.Object,FVector(0,0,52),FVector(.95,.44,.42));
 Part(TEXT("BearHump"),Sphere.Object,FVector(-30,0,72),FVector(.30,.34,.24));
 Part(TEXT("BearRump"),Sphere.Object,FVector(44,0,54),FVector(.38,.40,.34));
 Part(TEXT("BearChest"),Sphere.Object,FVector(-52,0,52),FVector(.32,.36,.32));
 Part(TEXT("BearNeck"),Sphere.Object,FVector(-76,0,54),FVector(.22,.24,.22));
 Part(TEXT("BearHead"),Sphere.Object,FVector(-98,0,52),FVector(.26,.24,.22));
 Part(TEXT("BearSnout"),Cone.Object,FVector(-120,0,44),FVector(.10,.10,.14),FRotator(-90,0,0));
 for(int Side:{-1,1})Part(*FString::Printf(TEXT("BearEar%d"),Side),Sphere.Object,FVector(-104,Side*15,68),FVector(.08,.05,.08));
 for(int32 I=0;I<4;I++)Part(*FString::Printf(TEXT("BearLeg%d"),I),Cylinder.Object,FVector((I<2)?-40.f:40.f,(I%2)?20.f:-20.f,24),FVector(.15,.15,.48));
 Part(TEXT("BearTail"),Sphere.Object,FVector(62,0,54),FVector(.09,.09,.09));
 for(int Side:{-1,1})Part(*FString::Printf(TEXT("BearEye%d"),Side),Sphere.Object,FVector(-112,Side*10,58),FVector(.035,.035,.035));
 auto Wisp=[&](const TCHAR* Name,FVector Loc,float Scale){auto* C=CreateDefaultSubobject<UStaticMeshComponent>(Name);C->SetupAttachment(RootComponent);C->SetStaticMesh(Sphere.Object);C->SetRelativeLocation(Loc);C->SetRelativeScale3D(FVector(Scale));C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCanEverAffectNavigation(false);C->SetCastShadow(false);Wisps.Add(C);};
 Wisp(TEXT("SpiritWispA"),FVector(-65,-85,145),.10f);Wisp(TEXT("SpiritWispB"),FVector(20,80,190),.075f);Wisp(TEXT("SpiritWispC"),FVector(105,-65,210),.055f);
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
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> SpiritMaterial(TEXT("/Game/BattleForTheA/Spirit/M_SpectralBlackBear.M_SpectralBlackBear"));
 if(SpiritMaterial.Succeeded())SpiritCard->SetMaterial(0,SpiritMaterial.Object);
 SpiritBillboard=CreateDefaultSubobject<UBillboardComponent>(TEXT("SpectralBlackBearBillboard"));SpiritBillboard->SetupAttachment(RootComponent);SpiritBillboard->SetSprite(SpiritTexture.Object);SpiritBillboard->SetRelativeLocation(FVector(0,0,110));SpiritBillboard->SetRelativeScale3D(FVector(1.15f));SpiritBillboard->SetCollisionEnabled(ECollisionEnabled::NoCollision);SpiritBillboard->SetCanEverAffectNavigation(false);SpiritBillboard->SetCastShadow(false);SpiritBillboard->SetVisibility(false);
 MoonGlow=CreateDefaultSubobject<UPointLightComponent>(TEXT("SpiritMoonGlow"));MoonGlow->SetupAttachment(RootComponent);MoonGlow->SetRelativeLocation(FVector(25,0,115));MoonGlow->SetLightColor(FLinearColor(.12f,.55f,1.f));MoonGlow->SetAttenuationRadius(900);MoonGlow->SetIntensity(0);MoonGlow->SetCastShadows(false);
 // Card visible, primitive bear hidden. If the material or the mesh is missing
 // the card is skipped at runtime and the primitives carry the scene.
 for(auto& C:FigureParts)C->SetVisibility(false);for(auto& C:Wisps)C->SetVisibility(false);
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
 // The painted bear is the presentation; the primitive figure is the fallback.
 const bool bCard=SpiritCard&&SpiritCard->GetStaticMesh()&&SpiritCard->GetMaterial(0)!=nullptr;
 for(auto& C:FigureParts)C->SetVisibility(Visible&&!bCard);
 SpiritBillboard->SetVisibility(false);SpiritCard->SetVisibility(Visible&&bCard);
 if(bCard){
  // The art is a side view, so it is turned to face the rider rather than to
  // face the direction of travel. That way the bear reads as a bear from the
  // trail instead of as a dark shape running away.
  const float S=Visible?FMath::Lerp(.80f,1.f,Reveal):1.f;
  SpiritCard->SetRelativeScale3D(FVector(2.70f,1.85f,1)*S);
  SpiritCard->SetRelativeLocation(FVector(0,0,92.f*S));
  if(Visible)if(const APawn* Rider=UGameplayStatics::GetPlayerPawn(this,0)){
   const FVector To=Rider->GetActorLocation()-GetActorLocation();
   // Pitch 90 lays the card upright with its normal along -X at yaw 0, so the
   // yaw that points that normal at the rider is the rider's bearing plus 180.
   if(!To.IsNearlyZero())SpiritCard->SetWorldRotation(FRotator(90,To.Rotation().Yaw+180,0));
  }
 }
 for(int32 I=0;I<Wisps.Num();I++){auto* C=Wisps[I].Get();C->SetVisibility(Visible);if(Visible)C->AddLocalOffset(FVector(0,0,FMath::Sin(GetWorld()->GetTimeSeconds()*2.f+I)*Dt*12.f));}
 MoonGlow->SetVisibility(Visible);MoonGlow->SetIntensity(Visible?3200.f*Reveal*(.85f+.15f*FMath::Sin(GetWorld()->GetTimeSeconds()*5.f)):0.f);
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
 auto* FigureMat=UMaterialInstanceDynamic::Create(Base,this);if(FigureMat)FigureMat->SetVectorParameterValue(TEXT("Color"),FLinearColor(.006f,.012f,.025f));
 auto* WispMat=UMaterialInstanceDynamic::Create(Base,this);if(WispMat)WispMat->SetVectorParameterValue(TEXT("Color"),FLinearColor(.08f,.55f,1.f));
 for(auto& C:FigureParts)C->SetMaterial(0,FigureMat);for(auto& C:Wisps)C->SetMaterial(0,WispMat);
 UE_LOG(LogTemp,Display,TEXT("BattleSpiritVisual: texture=%s material=%s"),*GetNameSafe(SpiritBillboard->Sprite),*GetNameSafe(SpiritCard->GetMaterial(0)));
}
