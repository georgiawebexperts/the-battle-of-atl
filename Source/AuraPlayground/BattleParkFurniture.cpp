#include "BattleParkFurniture.h"
#include "PiedmontPathSpline.h"
#include "PiedmontBike.h"
#include "BattleBike.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "PiedmontPedestrian.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
ABattleParkFurniture::ABattleParkFurniture(){
 PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=.5f;
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("FurnitureRoot"));
 Wood=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WoodenSlats"));Frame=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("IronFrame"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Timber(TEXT("/Game/BattleForTheA/Furniture/M_BenchWood.M_BenchWood"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Iron(TEXT("/Game/BattleForTheA/Furniture/M_BenchFrame.M_BenchFrame"));
 Wood->SetStaticMesh(Cube.Object);Wood->SetMaterial(0,Timber.Object);Frame->SetStaticMesh(Cylinder.Object);Frame->SetMaterial(0,Iron.Object);
 for(auto M:{Wood,Frame}){M->SetupAttachment(RootComponent);M->SetCollisionProfileName(TEXT("BlockAll"));M->SetCanEverAffectNavigation(true);}
 Tags.Add(TEXT("BattleParkBenches"));
}
void ABattleParkFurniture::AddBench(const FTransform& T){
 Benches.Add(T);
 auto Slat=[&](FVector P,FVector Scale,FRotator R=FRotator::ZeroRotator){Wood->AddInstance(FTransform(R,P,Scale)*T,true);};
 auto Tube=[&](FVector A,FVector B,float Radius){Frame->AddInstance(FTransform(FQuat::FindBetweenNormals(FVector::UpVector,(B-A).GetSafeNormal()),(A+B)*.5,FVector(Radius/50,Radius/50,(B-A).Size()/100))*T,true);};
 for(int I=0;I<5;I++)Slat(FVector(0,-20+I*10,45),FVector(1.8,.085,.035));
 for(int I=0;I<4;I++)Slat(FVector(0,-29-I*2,57+I*10),FVector(1.8,.035,.085),FRotator(0,0,-11));
 for(int Side:{-1,1}){
  const float X=Side*78;Tube(FVector(X,-22,-3),FVector(X,-22,49),2.5);Tube(FVector(X,22,-3),FVector(X,22,49),2.5);
  Tube(FVector(X,-22,38),FVector(X,22,38),2.5);Tube(FVector(X,-22,40),FVector(X,-38,94),2.2);
  Tube(FVector(X,20,45),FVector(X,20,67),2);Tube(FVector(X,20,67),FVector(X,-30,67),2);
  Tube(FVector(X,-32,0),FVector(X,-12,0),3);Tube(FVector(X,12,0),FVector(X,32,0),3);
 }
}
void ABattleParkFurniture::BeginPlay(){
 Super::BeginPlay();
 TArray<APiedmontPathSpline*> Paths;for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It)Paths.Add(*It);
 Paths.Sort([](const APiedmontPathSpline& A,const APiedmontPathSpline& B){return A.OsmWayId<B.OsmWayId;});
 TArray<FTransform> Candidates;
 for(auto* Path:Paths){if(Path->bBridge||!Path->bArtifactEligible)continue;auto* S=Path->Centerline.Get();for(float D=1000;D<S->GetSplineLength()-500;D+=4500){
  const FVector P=S->GetLocationAtDistanceAlongSpline(D,ESplineCoordinateSpace::World);const FVector F=S->GetDirectionAtDistanceAlongSpline(D,ESplineCoordinateSpace::World).GetSafeNormal2D();
  for(int Side:{-1,1}){const FVector R(-F.Y*Side,F.X*Side,0);Candidates.Add(FTransform(FRotator(0,(-R).Rotation().Yaw-90,0),P+R*(Path->WidthCm*.5f+120)));}
 }}
 FRandomStream Random(3701);for(int I=Candidates.Num()-1;I>0;I--)Candidates.Swap(I,Random.RandRange(0,I));
 FCollisionQueryParams Q(SCENE_QUERY_STAT(BenchPlacement),false,this);
 for(auto T:Candidates){if(Benches.Num()>=48)break;FVector P=T.GetLocation();bool Clear=true;
  for(const auto& Existing:Benches)if(FVector::Dist2D(P,Existing.GetLocation())<2200){Clear=false;break;}if(!Clear)continue;
  for(auto* Path:Paths){const FVector Near=Path->Centerline->FindLocationClosestToWorldLocation(P,ESplineCoordinateSpace::World);if(FVector::Dist2D(P,Near)<Path->WidthCm*.5f+75){Clear=false;break;}}if(!Clear)continue;
  float Low=BIG_NUMBER,High=-BIG_NUMBER;for(int X:{-1,1}){for(int Y:{-1,1}){const FVector Corner=P+T.TransformVector(FVector(X*90,Y*38,0));FHitResult H;
   if(!GetWorld()->LineTraceSingleByChannel(H,Corner+FVector(0,0,250),Corner-FVector(0,0,250),ECC_Visibility,Q)||H.ImpactNormal.Z<.98f||!H.GetActor()||!H.GetActor()->ActorHasTag(TEXT("RideGrass"))){Clear=false;break;}
   Low=FMath::Min(Low,float(H.ImpactPoint.Z));High=FMath::Max(High,float(H.ImpactPoint.Z));
  }}
  if(!Clear||High-Low>5)continue;
  P.Z=(Low+High)*.5f;
  for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->ContainsBike(P+FVector(0,0,96))){Clear=false;break;}if(!Clear)continue;
  if(GetWorld()->OverlapBlockingTestByChannel(P+FVector(0,0,48),T.GetRotation(),ECC_Pawn,FCollisionShape::MakeBox(FVector(95,43,45)),Q))continue;
  T.SetLocation(P);AddBench(T);
 }
 UE_LOG(LogTemp,Display,TEXT("BattleBenches: count=%d wood_instances=%d frame_instances=%d"),Benches.Num(),Wood->GetInstanceCount(),Frame->GetInstanceCount());
}


bool ABattleParkFurniture::IsBenchAvailable(int32 Index) const{
 if(!Benches.IsValidIndex(Index))return false;
 const auto* Claim=Reservations.Find(Index);
 return !Claim||!Claim->IsValid()||Claim->Get()->IsActorBeingDestroyed();
}
bool ABattleParkFurniture::ReserveBench(int32 Index,AActor* Claimant){
 if(!IsValid(Claimant)||Claimant->IsActorBeingDestroyed()||Claimant->GetWorld()!=GetWorld()||!Benches.IsValidIndex(Index))return false;
 const auto* Existing=Reservations.Find(Index);
 if(Existing&&Existing->Get()==Claimant)return true;
 if(!IsBenchAvailable(Index))return false;
 Reservations.Add(Index,Claimant);return true;
}
void ABattleParkFurniture::ReleaseBench(int32 Index,AActor* Claimant){
 const auto* Existing=Reservations.Find(Index);
 if(Existing&&Existing->Get()==Claimant)Reservations.Remove(Index);
}

void ABattleParkFurniture::Tick(float Dt){
 Super::Tick(Dt);
 auto* Mode=Cast<APiedmontRideMode>(UGameplayStatics::GetGameMode(this));auto* PC=UGameplayStatics::GetPlayerController(this,0);APawn* Player=PC?PC->GetPawn():nullptr;
 if(!Mode||!Player)return;
 if(EncounterRun!=Mode->RunNumber){
  if(EncounterVisitor.IsValid())EncounterVisitor->Destroy();EncounterVisitor.Reset();EncounterBench=INDEX_NONE;
  EncounterRun=Mode->RunNumber;EncounterEligibleTime=0;EncounterWait=0;bEncounterRolled=false;bEncounterAllowed=false;BenchEncounterAttempts=0;
 }
 const auto* Lab=Cast<ABattleLabMode>(Mode);
 bool Eligible=bAmbientBenchFire&&!Mode->bRunEnded&&Mode->StartCountdown<=0&&(!Lab||!Lab->bTutorialActive);
 if(auto* Person=Cast<APiedmontExplorer>(Player))Eligible=Eligible&&!Person->bDead&&!Person->bSwimming;
 if(auto* Bike=Cast<ABattleBike>(Player))Eligible=Eligible&&Bike->RiderHealth>0;
 if(!Eligible){
  if(EncounterBench!=INDEX_NONE&&EncounterVisitor.IsValid())EncounterVisitor->Destroy();EncounterVisitor.Reset();EncounterBench=INDEX_NONE;return;
 }
 EncounterEligibleTime+=Dt;if(EncounterEligibleTime<45)return;
 if(!bEncounterRolled){bEncounterRolled=true;bEncounterAllowed=FMath::FRand()<FMath::Clamp(BenchFireRunChance,0.f,1.f);}
 if(!bEncounterAllowed)return;
 if(EncounterBench!=INDEX_NONE){
  auto* Visitor=EncounterVisitor.Get();if(!Visitor){EncounterBench=INDEX_NONE;return;}
  EncounterWait+=Dt;
  if(Visitor->bDead||Visitor->StumbleRemaining>0||Visitor->HornReactions>0){ReleaseBench(EncounterBench,Visitor);EncounterBench=INDEX_NONE;return;}
  if(FVector::Dist2D(Player->GetActorLocation(),Visitor->GetActorLocation())<650&&FMath::Abs(Player->GetActorLocation().Z-Visitor->GetActorLocation().Z)<140){
   const int32 Index=EncounterBench;EncounterBench=INDEX_NONE;
   if(!Visitor->BeginBenchIgnition(this,Index)){ReleaseBench(Index,Visitor);Visitor->PauseRemaining=0;}
   return;
  }
  if(EncounterWait>60){ReleaseBench(EncounterBench,Visitor);Visitor->PauseRemaining=0;EncounterBench=INDEX_NONE;}
  return;
 }
 if(BenchEncounterAttempts>0)return;
 FVector Eye;FRotator View;PC->GetPlayerViewPoint(Eye,View);
 for(int32 Index=0;Index<Benches.Num();++Index){
  if(!IsBenchAvailable(Index))continue;const auto& T=Benches[Index];const FVector Position=T.TransformPosition(FVector(0,55,90));
  const float Distance=FVector::Dist2D(Player->GetActorLocation(),Position);
  if(Distance<900||Distance>1700||FMath::Abs(Player->GetActorLocation().Z-Position.Z)>200)continue;
  if(FVector::DotProduct((Position-Eye).GetSafeNormal(),View.Vector())>-.1f)continue;
  const FTransform Spawn(T.TransformVectorNoScale(FVector(0,-1,0)).Rotation(),Position);
  auto* Visitor=GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),Spawn,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding);
  if(!Visitor)continue;
  Visitor->CityAppearanceVariant=0;Visitor->Tags.Add(TEXT("AmbientBenchIgniter"));UGameplayStatics::FinishSpawningActor(Visitor,Spawn);
  if(!IsValid(Visitor)||Visitor->IsActorBeingDestroyed())continue;
  if(!ReserveBench(Index,Visitor)){Visitor->Destroy();continue;}
  Visitor->PauseRemaining=65;EncounterVisitor=Visitor;EncounterBench=Index;EncounterWait=0;++BenchEncounterAttempts;break;
 }
}
