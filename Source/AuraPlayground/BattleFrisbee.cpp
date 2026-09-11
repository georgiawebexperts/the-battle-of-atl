#include "BattleFrisbee.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "PiedmontPathSpline.h"
#include "AIController.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/CommandLine.h"

ABattleFrisbeePlayer::ABattleFrisbeePlayer(){bUseControllerRotationYaw=false;GetCharacterMovement()->bOrientRotationToMovement=false;}
FVector ABattleFrisbeePlayer::AdjustVisitorHand(int32 Side,FVector Target) const {
 if(Side==1)return bCatchPose?FVector(22,40,118):Target;
 if(ThrowPose>=0)return FVector(-28+42*ThrowPose,20+35*FMath::Sin(ThrowPose*PI),115+12*FMath::Sin(ThrowPose*PI));
 return bCatchPose?FVector(-22,40,118):Target;
}
void ABattleFrisbeePlayer::Tick(float Dt){
 APiedmontExplorer::Tick(Dt);
 if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();
 auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));
 if(bDead){Body->SetRelativeRotation(FRotator(0,-90,85));return;}
 if(!Mode||Mode->bRunEnded||Mode->StartCountdown>0)return;
 GetCharacterMovement()->MaxWalkSpeed=185;
 if(StumbleRemaining>0){StumbleRemaining=FMath::Max(0.f,StumbleRemaining-Dt);Body->SetRelativeRotation(FRotator(0,-90,FMath::Sin(StumbleRemaining*5)*22));return;}
 if(bWalkToTarget){const FVector Delta=WalkTarget-GetActorLocation();if(Delta.Size2D()>35){FVector Direction=Delta.GetSafeNormal2D();AvoidRemaining=FMath::Max(0.f,AvoidRemaining-Dt);
   if(AvoidRemaining<=0){FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(FrisbeeVisitorAvoidance),false,this);if(GetWorld()->SweepSingleByChannel(Hit,GetActorLocation(),GetActorLocation()+Direction*140,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeSphere(34),Q)&&Hit.ImpactNormal.Z<.5f){AvoidDirection=(Direction*.3f+FVector::CrossProduct(FVector::UpVector,Direction)).GetSafeNormal2D();AvoidRemaining=.65f;}}
   if(AvoidRemaining>0)Direction=AvoidDirection;
   SetActorRotation(FRotator(0,Direction.Rotation().Yaw,0));AddMovementInput(Direction,1);}else bWalkToTarget=false;}
}
bool ABattleFrisbeeGroup::Ground(UWorld* World,FVector XY,FVector& Point,bool GrassOnly){
 FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(FrisbeeGround),false);
 if(!World->LineTraceSingleByChannel(Hit,FVector(XY.X,XY.Y,7000),FVector(XY.X,XY.Y,-7000),ECC_Visibility,Q)||!Hit.GetActor()||Hit.ImpactNormal.Z<.8)return false;
 const AActor* A=Hit.GetActor();if(GrassOnly&&!A->ActorHasTag(TEXT("RideGrass")))return false;
 Point=Hit.ImpactPoint;
 for(TActorIterator<APiedmontWaterHazard> It(World);It;++It)if(It->ContainsBike(Point+FVector(0,0,88)))return false;
 return true;
}
ABattleFrisbeeGroup::ABattleFrisbeeGroup(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("FrisbeeGroupRoot"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 Disc=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SocialDisc"));Disc->SetupAttachment(RootComponent);Disc->SetStaticMesh(Cylinder.Object);Disc->SetRelativeScale3D(FVector(.28,.28,.025));Disc->SetCollisionEnabled(ECollisionEnabled::NoCollision);Disc->SetCanEverAffectNavigation(false);
 SupplyBag=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DiscSupply"));SupplyBag->SetupAttachment(RootComponent);SupplyBag->SetStaticMesh(Cube.Object);SupplyBag->SetRelativeScale3D(FVector(.45,.3,.32));SupplyBag->SetCollisionEnabled(ECollisionEnabled::NoCollision);SupplyBag->SetCanEverAffectNavigation(false);
 Label=CreateDefaultSubobject<UTextRenderComponent>(TEXT("DiscSupplyLabel"));Label->SetupAttachment(RootComponent);Label->SetRelativeLocation(FVector(0,0,55));Label->SetWorldSize(13);Label->SetHorizontalAlignment(EHTA_Center);Label->SetTextRenderColor(FColor(120,255,160));Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);Label->SetCanEverAffectNavigation(false);
 Tags.Add(TEXT("BattleFrisbeeGroup"));
}
void ABattleFrisbeeGroup::BeginPlay(){
 Super::BeginPlay();Attenuation=NewObject<USoundAttenuation>(this);Attenuation->Attenuation.bAttenuate=true;Attenuation->Attenuation.bSpatialize=true;Attenuation->Attenuation.AttenuationShapeExtents=FVector(100);Attenuation->Attenuation.FalloffDistance=1600;FieldDirection=FieldDirection.GetSafeNormal2D();
 Disc->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_DiscGlow.M_DiscGlow")));SupplyBag->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_LockSteel.M_LockSteel")));
 for(int32 I=0;I<2;I++){
  FVector P;if(!Ground(GetWorld(),GetActorLocation()+FieldDirection*(I?1000.f:230.f),P,true))return;
  Homes.Add(P+FVector(0,0,100));
 }
 FActorSpawnParameters Spawn;Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
 for(int32 I=0;I<2;I++){
  auto* Player=GetWorld()->SpawnActor<ABattleFrisbeePlayer>(Homes[I],(Homes[1-I]-Homes[I]).Rotation(),Spawn);if(!Player)return;Players.Add(Player);Player->AddTickPrerequisiteActor(this);
 }
 bReady=true;DiscPosition=Players[0]->GetActorLocation()+FVector(0,0,35);PhaseTime=.2f;UE_LOG(LogTemp,Display,TEXT("BattleFrisbee: ready %s at %s"),*AreaName,*GetActorLocation().ToString());
}
void ABattleFrisbeeGroup::Sound(const TCHAR* Asset,float Volume){if(auto* S=LoadObject<USoundBase>(nullptr,Asset))UGameplayStatics::PlaySoundAtLocation(this,S,DiscPosition,FRotator::ZeroRotator,Volume,1,0,Attenuation,nullptr,this);}
void ABattleFrisbeeGroup::BeginThrow(){
 Throws++;bMissThrow=(Throws%4)==0;FlightStart=Players[Holder]->Body->GetSocketLocation(TEXT("Hand_R"));FlightEnd=bMissThrow?PathLanding+FVector(0,0,3):Players[1-Holder]->GetActorLocation()+FVector(0,0,35);
 FlightDuration=FMath::Clamp(FVector::Dist(FlightStart,FlightEnd)/650.f,1.2f,2.5f);Phase=1;PhaseTime=0;
 Sound(TEXT("/Game/BattleForTheA/Audio/S_DiscLaunch.S_DiscLaunch"),.15f);
}
bool ABattleFrisbeeGroup::TrySupply(APawn* Pawn){
 if(!bReady||Supply<=0||SupplyCooldown>0||!Pawn||!Pawn->IsPlayerControlled()||FVector::DistSquared(Pawn->GetActorLocation(),GetActorLocation())>FMath::Square(170.f))return false;
 auto* Bike=Cast<ABattleBike>(Pawn);if(auto* Rider=Cast<ABattleRider>(Pawn))Bike=Rider->ParkedBike;if(!Bike||Bike->Inventory.Num()!=BattleWeapons::Count||!Bike->Inventory[3].Owned)return false;
 FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(FrisbeeSupplySight),false,Pawn);Q.AddIgnoredActor(this);Q.AddIgnoredActor(Bike);
 if(GetWorld()->LineTraceSingleByChannel(Hit,Pawn->GetActorLocation(),GetActorLocation()+FVector(0,0,10),ECC_Visibility,Q))return false;
 const int32 Amount=FMath::Min3(8,Supply,BattleWeapons::ReserveLimit(3)-Bike->Inventory[3].Reserve);if(Amount<=0||!Bike->GiveWeapon(3,Amount))return false;
 Supply-=Amount;AmmoGiven+=Amount;SupplyCooldown=8;Sound(TEXT("/Game/BattleForTheA/Audio/S_ColaOpen.S_ColaOpen"),.25f);return true;
}
void ABattleFrisbeeGroup::Tick(float Dt){
 Super::Tick(Dt);if(!bReady)return;
 const auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));if(!Mode||Mode->bRunEnded||Mode->StartCountdown>0)return;
 SupplyCooldown=FMath::Max(0.f,SupplyCooldown-Dt);TrySupply(UGameplayStatics::GetPlayerPawn(this,0));
 FString Hint=SupplyCooldown>0?FString::Printf(TEXT("Next handful in %ds"),FMath::CeilToInt(SupplyCooldown)):TEXT("Launcher ammo | stand near bag");
 if(auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0)){auto* B=Cast<ABattleBike>(Pawn);if(auto* P=Cast<ABattleRider>(Pawn))B=P->ParkedBike;if(B&&!B->Inventory[3].Owned)Hint=TEXT("Find a launcher crate first");}
 Label->SetText(FText::FromString(FString::Printf(TEXT("SPARE DISCS: %d\n%s"),Supply,*Hint)));
 if(auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0))Label->SetWorldRotation(FRotator(0,(Pawn->GetActorLocation()-GetActorLocation()).Rotation().Yaw,0));
 for(auto P:Players)if(!IsValid(P)||P->bDead){Disc->SetVisibility(false);return;}
 PhaseTime+=Dt;for(int32 I=0;I<2;I++){Players[I]->ThrowPose=-1;Players[I]->bCatchPose=Phase==1&&I!=Holder;if(!Players[I]->bWalkToTarget)Players[I]->SetActorRotation(FRotator(0,(Players[1-I]->GetActorLocation()-Players[I]->GetActorLocation()).Rotation().Yaw,0));}
 if(Phase==0){
  Players[Holder]->ThrowPose=FMath::Clamp(PhaseTime/.85f,0.f,1.f);DiscPosition=Players[Holder]->Body->GetSocketLocation(TEXT("Hand_R"));if(PhaseTime>=.85f)BeginThrow();
 }else if(Phase==1){
  const float T=FMath::Clamp(PhaseTime/FlightDuration,0.f,1.f);DiscPosition=FMath::Lerp(FlightStart,FlightEnd,T)+FVector(0,0,4*T*(1-T)*130);
  if(T>=1){PhaseTime=0;if(bMissThrow){Misses++;bDiscOnGround=true;Phase=2;Sound(TEXT("/Game/BattleForTheA/Audio/S_DiscBounce.S_DiscBounce"),.2f);}else{Catches++;Holder=1-Holder;Phase=0;Sound(TEXT("/Game/BattleForTheA/Audio/S_LockHit.S_LockHit"),.08f);}}
 }else if(Phase==2){
  if(PhaseTime>2){Players[1-Holder]->WalkTarget=PathLanding+FVector(0,0,88);Players[1-Holder]->bWalkToTarget=true;Phase=3;PhaseTime=0;}
 }else if(Phase==3){
  auto* P=Players[1-Holder].Get();if(FVector::Dist2D(P->GetActorLocation(),PathLanding)<80){Retrievals++;Holder=1-Holder;bDiscOnGround=false;P->WalkTarget=Homes[Holder];P->bWalkToTarget=true;Phase=4;PhaseTime=0;}
 }else if(Phase==4){DiscPosition=Players[Holder]->Body->GetSocketLocation(TEXT("Hand_R"));if(!Players[Holder]->bWalkToTarget){Phase=0;PhaseTime=0;}}
 Disc->SetWorldLocation(DiscPosition);Disc->AddWorldRotation(FRotator(0,Dt*600,0));
}
void ABattleFrisbeeGroup::EndPlay(const EEndPlayReason::Type Reason){for(auto P:Players)if(IsValid(P))P->Destroy();Super::EndPlay(Reason);}
void ABattleParkLifeDirector::BeginPlay(){
 Super::BeginPlay();
#if !UE_BUILD_SHIPPING
 if(FString(FCommandLine::Get()).Contains(TEXT("Audit"))&&!FParse::Param(FCommandLine::Get(),TEXT("BattleFrisbeeAudit")))return;
#endif
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!Mode)return;
 // Authored lawn locations within the Meadow and Oak Hill shown on the Conservancy map.
 const FVector Centers[]={FVector(4500,5000,0),FVector(-6500,8500,0),FVector(6200,8000,0),FVector(-10500,7600,0),FVector(4000,1800,0)};
 for(int32 I=0;I<FMath::Min(Mode->Difficulty.FrisbeeGroups,5);I++){
  FVector Closest;float Best=MAX_flt;
  for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It)if(It->bArtifactEligible){FVector P=It->Centerline->FindLocationClosestToWorldLocation(Centers[I],ESplineCoordinateSpace::World);float D=FVector::DistSquared2D(P,Centers[I]);if(D<Best){Best=D;Closest=P;}}
  FVector Direction=(Centers[I]-Closest).GetSafeNormal2D(),Surface,Path;
  if(Best==MAX_flt||!ABattleFrisbeeGroup::Ground(GetWorld(),Closest,Path)||!ABattleFrisbeeGroup::Ground(GetWorld(),Closest+Direction*240,Surface,true)){PlacementFailures++;continue;}
  FTransform Transform(Direction.Rotation(),Surface+FVector(0,0,18));
  auto* Group=GetWorld()->SpawnActorDeferred<ABattleFrisbeeGroup>(ABattleFrisbeeGroup::StaticClass(),Transform,this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  if(!Group){PlacementFailures++;continue;}Group->FieldDirection=Direction;Group->PathLanding=Path;Group->AreaName=(I==1||I==3)?TEXT("Oak Hill"):TEXT("Meadow");Group->FinishSpawning(Transform);
  if(Group->bReady)FrisbeeGroups++;else{Group->Destroy();PlacementFailures++;}
 }
 UE_LOG(LogTemp,Display,TEXT("BattleParkLife: frisbeeGroups=%d desired=%d failures=%d"),FrisbeeGroups,Mode->Difficulty.FrisbeeGroups,PlacementFailures);
}
