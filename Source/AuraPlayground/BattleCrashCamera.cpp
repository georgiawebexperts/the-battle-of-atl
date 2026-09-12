#include "BattleCrashCamera.h"
#include "Camera/CameraActor.h"
#include "Engine/World.h"
void UpdateBattleCrashCamera(ACameraActor* Camera,AActor* RiderOwner,AActor* FallenBike,const FVector& Focus,float Dt){
 if(!Camera||!RiderOwner)return;
 const FVector Desired=Focus+FVector(-320,-420,300);
 FCollisionQueryParams Query(SCENE_QUERY_STAT(BattleCrashCamera),false,RiderOwner);if(FallenBike)Query.AddIgnoredActor(FallenBike);
 auto ClampToClear=[&](const FVector& End){FHitResult Hit;if(Camera->GetWorld()->SweepSingleByChannel(Hit,Focus,End,FQuat::Identity,ECC_Camera,FCollisionShape::MakeSphere(12),Query))return Hit.bStartPenetrating?Focus+FVector(0,0,25):Hit.Location+Hit.ImpactNormal*3;return End;};
 const FVector Safe=ClampToClear(Desired);const FVector Smoothed=FMath::VInterpTo(Camera->GetActorLocation(),Safe,Dt,12);const FVector Position=ClampToClear(Smoothed);
 Camera->SetActorLocationAndRotation(Position,(Focus-Position).Rotation());
}
