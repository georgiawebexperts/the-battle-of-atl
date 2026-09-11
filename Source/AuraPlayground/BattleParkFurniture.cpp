#include "BattleParkFurniture.h"
#include "PiedmontPathSpline.h"
#include "PiedmontBike.h"
#include "PiedmontPedestrian.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
ABattleParkFurniture::ABattleParkFurniture(){
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
