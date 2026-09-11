#include "BattleZombie.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "PiedmontBlood.h"
#include "PiedmontDarkZone.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"
#include "Engine/DamageEvents.h"
#include "Engine/StaticMesh.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/CommandLine.h"
#include "BattleZombieLines.h"

ABattleZombie::ABattleZombie(){
 Tags.Add(TEXT("PiedmontHostile"));Tags.Add(TEXT("BattleHostile"));Tags.Add(TEXT("BattleZombie"));
 AIControllerClass=AAIController::StaticClass();AutoPossessAI=EAutoPossessAI::PlacedInWorldOrSpawned;bUseControllerRotationYaw=false;
 GetCharacterMovement()->bOrientRotationToMovement=true;GetCharacterMovement()->RotationRate=FRotator(0,540,0);GetCharacterMovement()->MaxStepHeight=60;
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
 LeftEye=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftUndeadEye"));RightEye=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightUndeadEye"));
 int32 Side=-1;for(auto* Eye:{LeftEye.Get(),RightEye.Get()}){Eye->SetupAttachment(GetCapsuleComponent());Eye->SetStaticMesh(Sphere.Object);Eye->SetRelativeLocation(FVector(10,Side*4,64));Eye->SetRelativeScale3D(FVector(.035));Eye->SetCollisionEnabled(ECollisionEnabled::NoCollision);Eye->SetCastShadow(false);Eye->SetCanEverAffectNavigation(false);Side=1;}
}
void ABattleZombie::BeginPlay(){
 Super::BeginPlay();Weapon->SetVisibility(false);GetCharacterMovement()->MaxWalkSpeed=MoveSpeed;
 if(auto* Material=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_Zombie.M_Zombie"))){Skin=UMaterialInstanceDynamic::Create(Material,this);for(int32 I=0;I<Body->GetNumMaterials();I++)Body->SetMaterial(I,Skin);}
 for(auto* Eye:{LeftEye.Get(),RightEye.Get()})Eye->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_ZombieEye.M_ZombieEye")));
 Speak();
}
void ABattleZombie::Speak(bool Charge){
 Subtitle=Charge?TEXT("RUNNER! AAAAAH!"):BattleZombieLines::Lines[FMath::RandHelper(UE_ARRAY_COUNT(BattleZombieLines::Lines))];SubtitleRemaining=3;
 if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_ZombieGrowl.S_ZombieGrowl")))UGameplayStatics::PlaySoundAtLocation(this,Sound,GetActorLocation(),.6f,Charge?1.5f:FMath::FRandRange(.8f,1.1f));
}
void ABattleZombie::Tick(float Dt){
 Super::Tick(Dt);Weapon->SetVisibility(false);SubtitleRemaining=FMath::Max(0.f,SubtitleRemaining-Dt);
 auto* AI=Cast<AAIController>(GetController());
 if(bDead){
  DeathTime+=Dt;Body->SetRelativeRotation(FRotator(0,-90,FMath::Min(90.f,DeathTime*160)));Body->SetRelativeLocation(FVector(0,0,-85));
  if(Skin)Skin->SetScalarParameterValue(TEXT("Dissolve"),FMath::Clamp((DeathTime-3.5f)/1.5f,0.f,1.f));return;
 }
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));
 if(!Mode||Mode->bRunEnded||Mode->StartCountdown>0){if(AI)AI->StopMovement();return;}
 if(Emergence>0){Emergence=FMath::Max(0.f,Emergence-Dt);Body->AddLocalOffset(FVector(0,0,-130*Emergence));if(AI)AI->StopMovement();return;}
 auto* Target=UGameplayStatics::GetPlayerPawn(this,0);if(!Target)return;
 auto* Bike=Cast<ABattleBike>(Target);if(auto* Foot=Cast<ABattleRider>(Target))Bike=Foot->ParkedBike;
 if(!Bike||Bike->RiderHealth<=0||Bike->RespawnRemaining>0||bSwimming){if(AI)AI->StopMovement();return;}
 Flinch=FMath::Max(0.f,Flinch-Dt);AttackDelay=FMath::Max(0.f,AttackDelay-Dt);PathDelay-=Dt;
 const FVector Delta=Target->GetActorLocation()-GetActorLocation();const float Distance=Delta.Size2D();
 SpeechDelay-=Dt;if(SpeechDelay<=0&&Distance<1800){Speak();SpeechDelay=FMath::FRandRange(12.f,20.f);}
 FHitResult Sight;FCollisionQueryParams Q(SCENE_QUERY_STAT(ZombieAttack),false,this);
 const bool Clear=!GetWorld()->LineTraceSingleByChannel(Sight,GetActorLocation()+FVector(0,0,40),Target->GetActorLocation(),ECC_Visibility,Q)||Sight.GetActor()==Target;
 if(bTelegraphing){
  WarningRemaining-=Dt;if(AI)AI->StopMovement();
  Body->AddLocalRotation(FRotator(0,0,FMath::Sin(WarningRemaining*18)*10));
  if(WarningRemaining<=0){bTelegraphing=false;AttackDelay=1.6f;
   if(Distance<145&&FMath::Abs(Delta.Z)<130&&Clear){Attacks++;UGameplayStatics::ApplyDamage(Target,AttackDamage,AI,this,UDamageType::StaticClass());if(auto* Riding=Cast<ABattleBike>(Target))Riding->Ride->Wipeout(TEXT("Zombie grabbed the wheel"));}
  }return;
 }
 if(Flinch>0){if(AI)AI->StopMovement();Body->AddLocalRotation(FRotator(0,0,-20*Flinch));return;}
 if(Distance<120&&FMath::Abs(Delta.Z)<130&&Clear&&AttackDelay<=0){bTelegraphing=true;WarningRemaining=WarningSeconds;if(AI)AI->StopMovement();return;}
 if(AI&&PathDelay<=0){PathDelay=.6f;PathRequests++;AI->MoveToActor(Target,75,true,true,true,nullptr,true);}
 // Twitch the shoulders while retaining the existing moving, articulated body.
 Body->AddLocalRotation(FRotator(0,0,FMath::Sin(GetWorld()->GetTimeSeconds()*13)*3));
}
float ABattleZombie::TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer){
 if(bDead||Amount<=0||!FMath::IsFinite(Amount))return 0;
 FVector HitPoint=GetActorLocation()+FVector(0,0,35);bool Head=false;
 if(Event.IsOfType(FPointDamageEvent::ClassID)){const auto& Point=static_cast<const FPointDamageEvent&>(Event);HitPoint=Point.HitInfo.ImpactPoint;Head=HitPoint.Z>GetActorLocation().Z+45;}
 const float Applied=FMath::Min(Health,Amount*(Head?3.f:1.f));Health-=Applied;if(Head)Headshots++;
 APiedmontBlood::Burst(GetWorld(),HitPoint,Causer?(GetActorLocation()-Causer->GetActorLocation()).GetSafeNormal():FVector::UpVector);Flinch=.3f;
 if(Health<=0){bDead=true;bTelegraphing=false;SubtitleRemaining=0;if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();GetCharacterMovement()->DisableMovement();GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);LeftEye->SetVisibility(false);RightEye->SetVisibility(false);DropWeapon();SetLifeSpan(5);}
 return Applied;
}
ABattleEnemyDirector::ABattleEnemyDirector(){PrimaryActorTick.bCanEverTick=true;}
void ABattleEnemyDirector::BeginPlay(){
 Super::BeginPlay();
 #if !UE_BUILD_SHIPPING
 // Existing opt-in terrain/quest fixtures isolate the subsystem they validate.
 for(const TCHAR* Flag:{TEXT("BattleAmmoAudit"),TEXT("BattleHUDReview"),TEXT("BattleFrisbeeAudit"),TEXT("BattleDiscAudit"),TEXT("BattleInventoryAudit"),TEXT("BattleMeleeAudit"),TEXT("BattleAudit"),TEXT("BattleHealthAudit"),TEXT("BattlePickupAudit"),TEXT("BattleGeographyAudit"),TEXT("BattleConnectorAudit"),TEXT("BattleEastsideAudit"),TEXT("BattleKrogAudit")})if(FParse::Param(FCommandLine::Get(),Flag))bFreezeSpawns=true;
 #endif
}
void ABattleEnemyDirector::Tick(float Dt){
 Super::Tick(Dt);auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);
 if(!Mode||!Pawn)return;DesiredZombies=Mode->Difficulty.Zombies;LiveZombies=0;
 for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)if(!It->bDead){if(FVector::DistSquared2D(It->GetActorLocation(),Pawn->GetActorLocation())>FMath::Square(7500.f))It->Destroy();else LiveZombies++;}
 if(bFreezeSpawns||Mode->StartCountdown>0||Mode->bRunEnded)return;
 bool InTunnel=false;for(TActorIterator<APiedmontDarkZone> It(GetWorld());It;++It)if(It->Contains(Pawn->GetActorLocation())){InTunnel=true;break;}
 WaveDelay=FMath::Max(0.f,WaveDelay-Dt);
 if(!InTunnel)RemainingWave=0; // Leaving a partial wave must not block ordinary spawns.
 if(InTunnel&&WaveDelay<=0&&RemainingWave==0&&Mode->Difficulty.TunnelWaveSize>0){RemainingWave=Mode->Difficulty.TunnelWaveSize;WaveDelay=45;Waves++;SpawnDelay=0;}
 SpawnDelay-=Dt;
 if(SpawnDelay<=0){
  const bool Wave=RemainingWave>0;
  if(LiveZombies<DesiredZombies+(Wave?Mode->Difficulty.TunnelWaveSize:0)&&SpawnZombie(Wave)){if(Wave){RemainingWave--;WaveSpawned++;}SpawnDelay=Wave?.25f:Mode->Difficulty.ZombieRespawnSeconds;}
  else SpawnDelay=1;
 }
}
bool ABattleEnemyDirector::SpawnZombie(bool Wave){
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());if(!Mode||!Pawn||!Nav)return false;
 for(int32 Try=0;Try<12;Try++){
  FNavLocation Point;if(!Nav->GetRandomReachablePointInRadius(Pawn->GetActorLocation(),Wave?1600:3000,Point))continue;
  if(FVector::Dist2D(Point.Location,Pawn->GetActorLocation())<(Wave?400:900))continue;
  const FVector Position=Point.Location+FVector(0,0,90);bool Wet=false;for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->ContainsBike(Position)){Wet=true;break;}if(Wet)continue;
  if(Wave){bool Covered=false;for(TActorIterator<APiedmontDarkZone> It(GetWorld());It;++It)if(It->Contains(Position)){Covered=true;break;}if(!Covered)continue;}
  FCollisionQueryParams Q(SCENE_QUERY_STAT(ZombieSpawn),false,Pawn);
  if(GetWorld()->OverlapBlockingTestByChannel(Position,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,88),Q))continue;
  if(auto* Z=GetWorld()->SpawnActorDeferred<ABattleZombie>(ABattleZombie::StaticClass(),FTransform(Position),this,nullptr,ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding)){
   Z->bSprinter=FMath::FRand()<Mode->Difficulty.SprinterFraction;Z->MoveSpeed=Z->bSprinter?Mode->Difficulty.SprinterSpeed:Mode->Difficulty.ZombieSpeed;Z->AttackDamage=Mode->Difficulty.ZombieDamage;Z->WarningSeconds=Mode->Difficulty.ZombieWarningSeconds;Z->FinishSpawning(FTransform(Position));if(Z->bSprinter)Z->Speak(true);Spawned++;return true;
  }
 }return false;
}
