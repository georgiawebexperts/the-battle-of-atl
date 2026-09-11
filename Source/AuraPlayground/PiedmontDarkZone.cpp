#include "PiedmontDarkZone.h"
#include "Components/BoxComponent.h"
APiedmontDarkZone::APiedmontDarkZone(){Bounds=CreateDefaultSubobject<UBoxComponent>(TEXT("DarknessBounds"));RootComponent=Bounds;Bounds->SetBoxExtent(FVector(250,250,300));Bounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);PrimaryActorTick.bCanEverTick=false;}
bool APiedmontDarkZone::Contains(FVector Point) const{const FVector P=Bounds->GetComponentTransform().InverseTransformPosition(Point),E=Bounds->GetUnscaledBoxExtent();return FMath::Abs(P.X)<=E.X&&FMath::Abs(P.Y)<=E.Y&&FMath::Abs(P.Z)<=E.Z;}
