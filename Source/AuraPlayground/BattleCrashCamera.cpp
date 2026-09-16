#include "BattleCrashCamera.h"
#include "Camera/CameraActor.h"
#include "Engine/World.h"
void UpdateBattleCrashCamera(ACameraActor* Camera,AActor* RiderOwner,AActor* FallenBike,const FVector& Focus,float Dt){
 if(!Camera||!RiderOwner)return;
 const FVector Forward=RiderOwner->GetActorForwardVector().GetSafeNormal2D();
 const FVector Right(-Forward.Y,Forward.X,0);
 // Frame the rider and fallen bike at human scale. The old high, fixed world
 // offset made the physical fall look like a tiny overhead map event.
 const FVector Target=Focus+FVector(0,0,22);
 const FVector Desired=Target-Forward*285.f+Right*225.f+FVector(0,0,185.f);
 FCollisionQueryParams Query(SCENE_QUERY_STAT(BattleCrashCamera),false,RiderOwner);if(FallenBike)Query.AddIgnoredActor(FallenBike);
 auto ClampToClear=[&](const FVector& End){FHitResult Hit;if(Camera->GetWorld()->SweepSingleByChannel(Hit,Target,End,FQuat::Identity,ECC_Camera,FCollisionShape::MakeSphere(12),Query))return Hit.bStartPenetrating?Target+FVector(0,0,25):Hit.Location+Hit.ImpactNormal*3;return End;};
 const FVector Safe=ClampToClear(Desired);const FVector Smoothed=FMath::VInterpTo(Camera->GetActorLocation(),Safe,Dt,12);const FVector Position=ClampToClear(Smoothed);
 Camera->SetActorLocationAndRotation(Position,(Target-Position).Rotation());
}
