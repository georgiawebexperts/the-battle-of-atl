#include "PiedmontPathSpline.h"
#include "Components/SplineComponent.h"
APiedmontPathSpline::APiedmontPathSpline(){
 PrimaryActorTick.bCanEverTick=false;
 Centerline=CreateDefaultSubobject<USplineComponent>(TEXT("OSMCenterline"));SetRootComponent(Centerline);
 Centerline->SetMobility(EComponentMobility::Static);Tags.Add(TEXT("PiedmontPathSource"));
}
void APiedmontPathSpline::SetCenterline(const TArray<FVector>& WorldPoints){
 Centerline->ClearSplinePoints(false);
 for(const FVector& Point:WorldPoints)Centerline->AddSplinePoint(Point,ESplineCoordinateSpace::World,false);
 for(int32 I=0;I<WorldPoints.Num();++I)Centerline->SetSplinePointType(I,ESplinePointType::Linear,false);
 Centerline->SetClosedLoop(false,false);Centerline->UpdateSpline();
#if WITH_EDITOR
 MarkPackageDirty();
#endif
}
