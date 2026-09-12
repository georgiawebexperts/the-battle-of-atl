#include "BattlePothole.h"
#include "BattleBike.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
ABattlePothole::ABattlePothole(){
 PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=.02f;
 Visual=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pothole surface"));SetRootComponent(Visual);Visual->SetCollisionProfileName(TEXT("NoCollision"));
}
bool ABattlePothole::EvaluateTraversal(ABattleBike* Bike,const FVector& From,const FVector& To){
 if(!Bike||Bike->bParked||!Bike->GetController()||!Bike->Ride)return false;
 if(TrackedBike!=Bike){TrackedBike=Bike;bLatched=false;}
 const FVector Site=GetActorLocation();
 if(bLatched){if(FVector::DistSquared2D(To,Site)>FMath::Square(ContactRadius+100))bLatched=false;return false;}
 if(!Bike->Ride->IsMovingOnGround()||Bike->Ride->Recovery>0||FMath::Abs(Bike->Ride->Speed)<150)return false;
 // Sweep the wheel path so fast traversal cannot skip a small authored hazard.
 FVector A=From,B=To,C=Site;A.Z=B.Z=C.Z=0;
 const FVector Closest=FMath::ClosestPointOnSegment(C,A,B);
 const float T=FVector::DistSquared(A,B)>1?FMath::Clamp(FVector::DotProduct(Closest-A,B-A)/FVector::DistSquared(A,B),0.f,1.f):0.f;
 if(FVector::DistSquared2D(Closest,C)>FMath::Square(ContactRadius)||FMath::Abs(FMath::Lerp(From.Z,To.Z,T)-Site.Z)>180)return false;
 bLatched=true;++Contacts;
 if(bDeep&&FMath::Abs(Bike->Ride->Speed)>=CrashSpeed)Bike->Ride->Wipeout(TEXT("Deep pothole"));
 else {Bike->Ride->Speed*=.72f;Bike->RideImpact(bDeep?.65f:.35f);}
 return true;
}
void ABattlePothole::Tick(float Dt){
 Super::Tick(Dt);auto* Bike=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(this,0));
 if(!Bike||Bike->bParked){bHasPrevious=false;return;}
 const FVector Wheel=Bike->GetActorLocation()+Bike->GetActorForwardVector()*80;
 if(!bHasPrevious||TrackedBike!=Bike){TrackedBike=Bike;PreviousWheel=Wheel;bHasPrevious=true;bLatched=false;return;}
 // Teleports and respawns are not road impacts.
 if(FVector::DistSquared(PreviousWheel,Wheel)<FMath::Square(3000.f))EvaluateTraversal(Bike,PreviousWheel,Wheel);
 PreviousWheel=Wheel;
}
