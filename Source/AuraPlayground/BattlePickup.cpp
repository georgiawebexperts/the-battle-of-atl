#include "BattlePickup.h"
#include "BattleSpareBikes.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleCheckpoints.h"
#include "BattleQuest.h"
#include "PiedmontPathSpline.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/CommandLine.h"

ABattleColaPickup::ABattleColaPickup(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("PickupRoot"));
 Can=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ColaCan"));Can->SetupAttachment(RootComponent);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));Can->SetStaticMesh(Cylinder.Object);Can->SetRelativeScale3D(FVector(.22,.22,.4));Can->SetCollisionEnabled(ECollisionEnabled::NoCollision);Can->SetCanEverAffectNavigation(false);
 Glow=CreateDefaultSubobject<UPointLightComponent>(TEXT("PickupGlow"));Glow->SetupAttachment(RootComponent);Glow->SetIntensity(120);Glow->SetAttenuationRadius(180);Glow->SetLightColor(FLinearColor(1,.12,.04));Glow->SetCastShadows(false);
 for(int32 I=0;I<2;I++){
  auto* Label=CreateDefaultSubobject<UTextRenderComponent>(*FString::Printf(TEXT("ColaLabel%d"),I));Label->SetupAttachment(RootComponent);Label->SetRelativeLocation(FVector(I?-12:12,0,3));Label->SetRelativeRotation(FRotator(0,I?180:0,0));Label->SetWorldSize(18);Label->SetText(FText::FromString(TEXT("Soda Pop")));Label->SetHorizontalAlignment(EHTA_Center);Label->SetTextRenderColor(FColor::White);Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);Label->SetCanEverAffectNavigation(false);
 }
 Tags.Add(TEXT("BattleHealthPickup"));
}
void ABattleColaPickup::BeginPlay(){
 Super::BeginPlay();RestLocation=GetActorLocation();
 Can->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,(bTimeBonus||bSpeedBonus)?TEXT("/Game/BattleForTheA/Materials/M_DiscGlow.M_DiscGlow"):TEXT("/Game/BattleForTheA/Materials/M_ColaRed.M_ColaRed")));
 if(bTimeBonus||bSpeedBonus){
  Tags.Remove(TEXT("BattleHealthPickup"));Tags.Add(bSpeedBonus?TEXT("BattleSpeedPickup"):TEXT("BattleTimePickup"));
  Can->SetRelativeScale3D(FVector(.55,.55,.12));Can->SetRelativeRotation(FRotator(90,0,0));Glow->SetLightColor(FLinearColor(.2,1,.65));
  TArray<UTextRenderComponent*> Labels;GetComponents(Labels);for(auto* Label:Labels){Label->SetText(FText::FromString(bSpeedBonus?TEXT(">> 5s"):TEXT("+30s")));Label->SetTextRenderColor(FColor(8,25,15));Label->SetWorldSize(26);Label->SetRelativeLocation(Label->GetRelativeLocation()+FVector(0,0,3));}
 }
}
void ABattleColaPickup::Tick(float Dt){
 Super::Tick(Dt);if(bConsumed)return;Clock+=Dt;SetActorLocation(RestLocation+FVector(0,0,FMath::Sin(Clock*2.5f)*5));AddActorWorldRotation(FRotator(0,45*Dt,0));
 // Labels grow with speed so ground text stays readable at pace.
 APawn* Rider=UGameplayStatics::GetPlayerPawn(this,0);
 float LabelScale=1.f;
 if(Rider){auto* Bike=Cast<ABattleBike>(Rider);if(!Bike)if(auto* Person=Cast<ABattleRider>(Rider))Bike=Person->ParkedBike;
  if(Bike&&Bike->Ride)LabelScale=1.f+FMath::Clamp(Bike->Ride->Speed/1800.f,0.f,1.4f);}
 TArray<UTextRenderComponent*> Labels;GetComponents(Labels);
 for(auto* Label:Labels)if(!bTimeBonus&&!bSpeedBonus)Label->SetWorldSize(18.f*LabelScale);
 TryCollect(Rider);
}
bool ABattleColaPickup::TryCollect(APawn* Pawn){
 if(bConsumed||!Pawn||!Pawn->IsPlayerControlled()||FVector::DistSquared(Pawn->GetActorLocation(),GetActorLocation())>FMath::Square(150.f))return false;
 auto* Bike=Cast<ABattleBike>(Pawn);if(auto* Person=Cast<ABattleRider>(Pawn))Bike=Person->ParkedBike;if(!Bike)return false;
 FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(ColaPickupSight),false,Pawn);Q.AddIgnoredActor(this);
 if(GetWorld()->LineTraceSingleByChannel(Hit,Pawn->GetActorLocation(),GetActorLocation(),ECC_Visibility,Q))return false;
 if(bSpeedBonus){
  auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));
  if(Pawn!=Bike||Bike->bParked||Bike->bCrashActive||Bike->RiderHealth<=0||Bike->StunRemaining>0||Bike->Ride->Recovery>0||!Mode||Mode->bRunEnded||Mode->StartCountdown>0)return false;
  // Speed crates now bank a throttle boost instead of firing it immediately.
  Bike->BoostCharges=FMath::Min(3,Bike->BoostCharges+1);
  bConsumed=true;if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_Boost.S_Boost")))UGameplayStatics::PlaySound2D(this,Sound);Destroy();return true;
 }
 if(bTimeBonus){
  auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));if(Bike->RiderHealth<=0||!Mode||!Mode->AdjustRunTime(30,TEXT("TIME BONUS")))return false;
  bConsumed=true;if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_Boost.S_Boost")))UGameplayStatics::PlaySoundAtLocation(this,Sound,GetActorLocation(),.5f);Destroy();return true;
 }
 const float Applied=Bike->RestoreRiderHealth(HealAmount);if(Applied<=0)return false;
 bConsumed=true;Bike->HealthPickups++;Bike->LastHealAmount=Applied;Bike->PickupNoticeRemaining=2;
 if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_ColaOpen.S_ColaOpen")))UGameplayStatics::PlaySoundAtLocation(this,Sound,GetActorLocation(),.7f);
 Destroy();return true;
}
ABattlePickupDirector::ABattlePickupDirector(){PrimaryActorTick.bCanEverTick=false;}
namespace {
bool FindAmmoBinSite(AActor* Owner,FVector Surface,FVector& Base){
 APiedmontPathSpline* Nearest=nullptr;FVector Center;float Best=MAX_flt;
 for(TActorIterator<APiedmontPathSpline> It(Owner->GetWorld());It;++It){
  if(It->bBridge)continue;const FVector P=It->Centerline->FindLocationClosestToWorldLocation(Surface,ESplineCoordinateSpace::World);const float D=FVector::DistSquared2D(P,Surface);
  if(D<Best){Best=D;Nearest=*It;Center=P;}
 }
 if(!Nearest||Best>FMath::Square(450.f))return false;
 const float Key=Nearest->Centerline->FindInputKeyClosestToWorldLocation(Center);
 const FVector Direction=Nearest->Centerline->GetDirectionAtSplineInputKey(Key,ESplineCoordinateSpace::World).GetSafeNormal2D(),Right(-Direction.Y,Direction.X,0);
 FCollisionQueryParams Q(SCENE_QUERY_STAT(AmmoBinSite),false,UGameplayStatics::GetPlayerPawn(Owner,0));
 for(float Along:{0.f,150.f,-150.f})for(int Side:{-1,1}){
  const FVector P=Center+Direction*Along+Right*Side*(Nearest->WidthCm*.5f+65.f);FHitResult Hit;
  bool OnPath=false;for(TActorIterator<APiedmontPathSpline> It(Owner->GetWorld());It;++It){const FVector Closest=It->Centerline->FindLocationClosestToWorldLocation(P,ESplineCoordinateSpace::World);if(FVector::DistSquared2D(P,Closest)<FMath::Square(It->WidthCm*.5f+34.f)){OnPath=true;break;}}if(OnPath)continue;
  if(!Owner->GetWorld()->LineTraceSingleByChannel(Hit,P+FVector(0,0,300),P-FVector(0,0,300),ECC_Visibility,Q)||Hit.ImpactNormal.Z<.97f)continue;
  if(!Hit.GetActor()||(!Hit.GetActor()->ActorHasTag(TEXT("RideGrass"))&&!Hit.GetActor()->ActorHasTag(TEXT("RidePath"))&&!Hit.GetActor()->ActorHasTag(TEXT("RideDirt"))))continue;
  bool Wet=false;for(TActorIterator<APiedmontWaterHazard> It(Owner->GetWorld());It;++It)if(It->ContainsBike(Hit.ImpactPoint+FVector(0,0,98))){Wet=true;break;}if(Wet)continue;
  float Low=Hit.ImpactPoint.Z,High=Low;bool Supported=true;
  for(FVector Offset:{FVector(28,0,0),FVector(-28,0,0),FVector(0,28,0),FVector(0,-28,0)}){
   FHitResult Foot;if(!Owner->GetWorld()->LineTraceSingleByChannel(Foot,Hit.ImpactPoint+Offset+FVector(0,0,30),Hit.ImpactPoint+Offset-FVector(0,0,30),ECC_Visibility,Q)){Supported=false;break;}
   Low=FMath::Min(Low,float(Foot.ImpactPoint.Z));High=FMath::Max(High,float(Foot.ImpactPoint.Z));
  }
  if(!Supported||High-Low>4.f)continue;
  Base=FVector(P.X,P.Y,Low-1);
  if(Owner->GetWorld()->OverlapBlockingTestByChannel(Base+FVector(0,0,46),FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,43),Q))continue;
  return true;
 }
 return false;
}
AActor* SpawnAmmoBin(AActor* Owner,FVector Base,UStaticMesh* Asset){
 auto* Bin=Owner->GetWorld()->SpawnActor<AActor>(Base+FVector(0,0,44),FRotator::ZeroRotator);if(!Bin)return nullptr;
 Bin->Tags.Add(TEXT("BattleAmmoBin"));Bin->SetOwner(Owner);
 auto* Collision=NewObject<UCapsuleComponent>(Bin,TEXT("BinCollision"));Bin->AddInstanceComponent(Collision);Bin->SetRootComponent(Collision);Collision->InitCapsuleSize(31,44);Collision->SetCollisionProfileName(TEXT("BlockAll"));Collision->SetCanEverAffectNavigation(false);Collision->RegisterComponent();
 Bin->SetActorLocation(Base+FVector(0,0,44));
 auto* Mesh=NewObject<UStaticMeshComponent>(Bin,TEXT("ParkTrashBin"));Bin->AddInstanceComponent(Mesh);Mesh->SetupAttachment(Collision);Mesh->SetStaticMesh(Asset);Mesh->SetRelativeLocation(FVector(0,0,-44));Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);Mesh->SetCanEverAffectNavigation(false);Mesh->RegisterComponent();
 return Bin;
}
}
bool ABattlePickupDirector::SpawnCola(FVector Surface,bool Trail,float Heal,int32 WeaponSlot,bool bLandmark){
 // Ammo may sit near other supplies, but keep a clear eight-metre separation.
 // Landmark crates are deliberately placed, so they only need breathing room.
 const float Spacing=bLandmark?600.f:(WeaponSlot==0?800.f:1600.f);
 for(const FVector& P:Locations)if(FVector::DistSquared2D(P,Surface)<FMath::Square(Spacing))return false;
 auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);if(!Pawn)return false;
 FHitResult Ground;FCollisionQueryParams Q(SCENE_QUERY_STAT(ColaPlacement),false,Pawn);
 if(!GetWorld()->LineTraceSingleByChannel(Ground,Surface+FVector(0,0,150),Surface-FVector(0,0,150),ECC_Visibility,Q)||!Ground.GetActor())return false;
 const auto* Floor=Ground.GetActor();
 const bool bRideSurface=Floor->ActorHasTag(TEXT("RidePath"))||Floor->ActorHasTag(TEXT("RideDirt"))||Floor->ActorHasTag(TEXT("RideBridge"));
 if(!bRideSurface){
  // The landmark plazas are paved with meshes the route rules never tagged.
  // A landmark crate is an authored promise, so accept dry, flat pavement.
  if(!bLandmark||Ground.ImpactNormal.Z<.85f)return false;
  for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->ContainsBike(Ground.ImpactPoint+FVector(0,0,98)))return false;
 }
 FVector Spot=Ground.ImpactPoint+FVector(0,0,65);FVector BinBase;UStaticMesh* BinAsset=nullptr;
 if(!Floor->ActorHasTag(TEXT("RideBridge")))for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->ContainsBike(Ground.ImpactPoint+FVector(0,0,98)))return false;
 if(WeaponSlot==0){
  BinAsset=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/BattleForTheA/Environment/ParkBin/ParkBin/StaticMeshes/ParkBin.ParkBin"));
  if(!BinAsset||!FindAmmoBinSite(this,Ground.ImpactPoint,BinBase))return false;
  const FVector Toward=(BinBase-Ground.ImpactPoint).GetSafeNormal2D();const FVector Supply=Ground.ImpactPoint+Toward*FMath::Max(0.f,float(FVector::Dist2D(BinBase,Ground.ImpactPoint))-110.f);
  FHitResult Near;if(!GetWorld()->LineTraceSingleByChannel(Near,Supply+FVector(0,0,150),Supply-FVector(0,0,150),ECC_Visibility,Q)||!Near.GetActor()||!Near.GetActor()->ActorHasTag(TEXT("RidePath")))return false;
  Ground=Near;Spot=Ground.ImpactPoint+FVector(0,0,65);
  if(GetWorld()->OverlapBlockingTestByChannel(Ground.ImpactPoint+FVector(0,0,98),FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,96),Q))return false;
  for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->ContainsBike(Ground.ImpactPoint+FVector(0,0,98)))return false;
 }
 if(GetWorld()->OverlapBlockingTestByChannel(Spot,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeSphere(28),Q))return false;
 // The practice street is outside the park navigation mesh. Validate supplies from the saved park start.
 auto* ParkMode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));
 const FVector RouteStart=ParkMode&&ParkMode->Quest?ParkMode->Quest->InitialStartTransform.GetLocation():Pawn->GetActorLocation();
 auto* Route=UNavigationSystemV1::FindPathToLocationSynchronously(this,RouteStart,Ground.ImpactPoint,Pawn);
 if(!Route||!Route->IsValid()||Route->IsPartial())return false;
 if(WeaponSlot==-3){if(GetWorld()->SpawnActor<ABattleHornPickup>(Spot,FRotator::ZeroRotator)){Locations.Add(Spot);HornPickups++;return true;}return false;}
 if(WeaponSlot>=0){
  if(auto* Crate=GetWorld()->SpawnActorDeferred<ABattleWeaponCrate>(ABattleWeaponCrate::StaticClass(),FTransform(Spot),this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn)){AActor* Bin=WeaponSlot==0?SpawnAmmoBin(this,BinBase,BinAsset):nullptr;
   if(WeaponSlot==0&&!Bin){Crate->Destroy();return false;}
   Crate->WeaponSlot=WeaponSlot;Crate->FinishSpawning(FTransform(Spot));if(Bin)Crate->Tags.Add(TEXT("AmmoByBin"));Locations.Add(Spot);if(WeaponSlot==0)AmmoPickups++;else WeaponCrates++;return true;}return false;
 }
 if(auto* Pickup=GetWorld()->SpawnActorDeferred<ABattleColaPickup>(ABattleColaPickup::StaticClass(),FTransform(Spot),this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn)){
  Pickup->HealAmount=Heal;Pickup->bTrailPickup=Trail;Pickup->bTimeBonus=WeaponSlot==-2;Pickup->bSpeedBonus=WeaponSlot==-4;Pickup->FinishSpawning(FTransform(Spot));Locations.Add(Spot);if(WeaponSlot==-4)SpeedPickups++;else if(WeaponSlot==-2)TimePickups++;else{Spawned++;if(Trail)TrailPickups++;else ParkPickups++;}return true;
 }return false;
}
void ABattlePickupDirector::BeginPlay(){
 Super::BeginPlay();
#if !UE_BUILD_SHIPPING
 for(const TCHAR* Flag:{TEXT("BattleMeleeAudit"),TEXT("BattleAudit"),TEXT("BattleHealthAudit"),TEXT("BattleGeographyAudit"),TEXT("BattleConnectorAudit"),TEXT("BattleEastsideAudit"),TEXT("BattleKrogAudit"),TEXT("BattleZombieAudit"),TEXT("BattleZombiePopulationAudit")})if(FParse::Param(FCommandLine::Get(),Flag))return;
#endif
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!Mode)return;
 const int32 Desired=Mode->Difficulty.HealthPickups;const int32 TrailGoal=FMath::Min(Desired,FMath::Max(2,Desired/3));const float Heal=Mode->Difficulty.ColaHealAmount;
 // A supply is promised at each landmark. Those plazas are paved with meshes
 // the route rules never tagged, so the landmark rule accepts dry flat
 // pavement; if the anchor itself is taken, walk a ring around it. These count
 // toward the trail goal, so the totals stay exactly as the difficulty asks.
 for(const auto& A:BattleCheckpoints::Anchors){
  if(TrailPickups>=TrailGoal)break;
  const FVector Anchor((float)A.X,(float)A.Y,(float)A.Z);
  if(SpawnCola(Anchor,true,Heal,-1,true))continue;
  for(const FVector& Offset:{FVector(700,0,0),FVector(-700,0,0),FVector(0,700,0),FVector(0,-700,0),
                             FVector(1300,0,0),FVector(-1300,0,0),FVector(0,1300,0),FVector(0,-1300,0)})
   if(SpawnCola(Anchor+Offset,true,Heal,-1,true))break;
 }
 TArray<FVector> Park,Trail;
 for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It){auto* S=It->Centerline.Get();auto& Candidates=It->bArtifactEligible?Park:Trail;
  for(float D=FMath::Min(500.f,S->GetSplineLength()*.5f);D<S->GetSplineLength();D+=1800)Candidates.Add(S->GetLocationAtDistanceAlongSpline(D,ESplineCoordinateSpace::World));
 }
 auto Fill=[&](TArray<FVector>& Candidates,bool IsTrail,int32 Goal){
  while(!Candidates.IsEmpty()&&(IsTrail?TrailPickups:ParkPickups)<Goal){const int32 I=FMath::RandHelper(Candidates.Num());const FVector P=Candidates[I];Candidates.RemoveAtSwap(I);if(!SpawnCola(P,IsTrail,Heal))PlacementFailures++;}
 };
 const TArray<FVector> AmmoPark=Park,AmmoTrail=Trail;
 Fill(Trail,true,TrailGoal);Fill(Park,false,Desired-TrailGoal);
#if !UE_BUILD_SHIPPING
 const bool ColaAudit=FParse::Param(FCommandLine::Get(),TEXT("BattlePickupAudit"));
#else
 const bool ColaAudit=false;
#endif
 if(!ColaAudit){
  // Reserve pistol supplies before optional weapon/time/horn pickups occupy the route.
  auto FillAmmo=[&](TArray<FVector> Candidates,bool IsTrail,int32 Goal){while(!Candidates.IsEmpty()&&AmmoPickups<Goal){const int32 I=FMath::RandHelper(Candidates.Num());const FVector P=Candidates[I];Candidates.RemoveAtSwap(I);SpawnCola(P,IsTrail,0,0);}};
  FillAmmo(AmmoPark,false,6);FillAmmo(AmmoTrail,true,12);
  TArray<FVector> Supplies=Park;Supplies.Append(Trail);
  while(!Supplies.IsEmpty()&&WeaponCrates<Mode->Difficulty.WeaponCrates){const int32 I=FMath::RandHelper(Supplies.Num());const FVector P=Supplies[I];Supplies.RemoveAtSwap(I);SpawnCola(P,false,0,1+WeaponCrates%4);}
  auto FillTime=[&](TArray<FVector> Candidates,bool IsTrail,int32 Goal){while(!Candidates.IsEmpty()&&TimePickups<Goal){const int32 I=FMath::RandHelper(Candidates.Num());const FVector P=Candidates[I];Candidates.RemoveAtSwap(I);SpawnCola(P,IsTrail,0,-2);}};
  FillTime(Park,false,2);FillTime(Trail,true,3);
  auto FillHorns=[&](TArray<FVector> Candidates,bool IsTrail,int32 Goal){while(!Candidates.IsEmpty()&&HornPickups<Goal){const int32 I=FMath::RandHelper(Candidates.Num());const FVector P=Candidates[I];Candidates.RemoveAtSwap(I);SpawnCola(P,IsTrail,0,-3);}};
  FillHorns(Park,false,3);FillHorns(Trail,true,6);
  auto FillSpeed=[&](TArray<FVector> Candidates,bool IsTrail,int32 Goal){while(!Candidates.IsEmpty()&&SpeedPickups<Goal){const int32 I=FMath::RandHelper(Candidates.Num());const FVector P=Candidates[I];Candidates.RemoveAtSwap(I);SpawnCola(P,IsTrail,0,-4);}};
  FillSpeed(AmmoPark,false,3);FillSpeed(AmmoTrail,true,6);
  UE_LOG(LogTemp,Display,TEXT("BattleSpeedPickups: spawned=%d desired=6"),SpeedPickups);
  UE_LOG(LogTemp,Display,TEXT("BattleHornPickups: spawned=%d desired=6"),HornPickups);
  UE_LOG(LogTemp,Display,TEXT("BattleAmmoPickups: spawned=%d desired=12"),AmmoPickups);
  UE_LOG(LogTemp,Display,TEXT("BattleTimePickups: spawned=%d desired=3"),TimePickups);
  UE_LOG(LogTemp,Display,TEXT("BattleCrates: spawned=%d desired=%d"),WeaponCrates,Mode->Difficulty.WeaponCrates);
 }
 if(!ColaAudit)BattleSpareBikes::SpawnStations(this);
 UE_LOG(LogTemp,Display,TEXT("BattlePickups: spawned=%d park=%d trail=%d desired=%d"),Spawned,ParkPickups,TrailPickups,Desired);
}
