#include "BattleDrone.h"
#include "BattleBike.h"
#include "BattleZombie.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
ABattleDrone::ABattleDrone(){
 PrimaryActorTick.bCanEverTick=true;InitialLifeSpan=12;Tags.Add(TEXT("BattleDrone"));
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("DroneRoot"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Dark(TEXT("/Game/BeltLineGlide/Materials/M_Rubber.M_Rubber"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Glow(TEXT("/Game/BattleForTheA/Materials/M_ShotGlow.M_ShotGlow"));
 auto Part=[&](FName Name,FVector P,FVector Scale,bool Round=false){auto* M=CreateDefaultSubobject<UStaticMeshComponent>(Name);M->SetupAttachment(RootComponent);M->SetStaticMesh(Round?Cylinder.Object:Cube.Object);M->SetRelativeLocation(P);M->SetRelativeScale3D(Scale);M->SetMaterial(0,Dark.Object);M->SetCollisionEnabled(ECollisionEnabled::QueryOnly);M->SetCollisionResponseToAllChannels(ECR_Ignore);M->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);M->SetCanEverAffectNavigation(false);return M;};
 Part(TEXT("Body"),FVector::ZeroVector,FVector(.48,.32,.18));
 for(int X:{-1,1})for(int Y:{-1,1}){
  auto* Arm=Part(FName(*FString::Printf(TEXT("Arm%d%d"),X,Y)),FVector(X*22,Y*22,0),FVector(.55,.055,.045));Arm->SetRelativeRotation(FRotator(0,X*Y*45,0));
  Part(FName(*FString::Printf(TEXT("Motor%d%d"),X,Y)),FVector(X*40,Y*40,2),FVector(.11,.11,.13),true);
  Rotors.Add(Part(FName(*FString::Printf(TEXT("Rotor%d%d"),X,Y)),FVector(X*40,Y*40,10),FVector(.48,.035,.018)));
 }
 auto* Lamp=Part(TEXT("WarningLamp"),FVector(25,0,0),FVector(.035,.12,.08));Lamp->SetMaterial(0,Glow.Object);
}
float ABattleDrone::TakeDamage(float Amount,const FDamageEvent&,AController*,AActor*){
 if(!FMath::IsFinite(Amount)||Amount<=0||Health<=0)return 0;const float Applied=FMath::Min(Health,Amount);Health-=Applied;if(Health<=0){bWarning=false;bSpent=true;Destroy();}return Applied;
}
void ABattleDrone::Tick(float Dt){
 Super::Tick(Dt);auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));if(!Mode||Mode->bRunEnded||Mode->StartCountdown>0)return;
 for(auto Rotor:Rotors)Rotor->AddLocalRotation(FRotator(0,Dt*2200,0));
 auto* Bike=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(this,0));
 if(!Bike||Bike->RiderHealth<=0||Bike->StunRemaining>0||Bike->RespawnRemaining>0){bSpent=true;bWarning=false;}
 if(bSpent){AddActorWorldOffset(FVector(0,0,400)*Dt);SetActorRotation(FRotator(-20,GetActorRotation().Yaw,0));return;}
 Clock+=Dt;
 if(bWarning){SetActorRotation((Bike->GetActorLocation()-GetActorLocation()).Rotation());if(Clock<2)return;bWarning=false;Clock=0;DiveStart=GetActorLocation();DiveTarget=Bike->GetActorLocation();EscapeDirection=(DiveTarget-DiveStart).GetSafeNormal2D();}
 const float T=Clock/.8f;
 const FVector Next=T<=1?FMath::Lerp(DiveStart,DiveTarget,T):DiveTarget+EscapeDirection*((T-1)*800)+FVector(0,0,FMath::Square(T-1)*650);
 FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(DroneSweep),false,this);
 if(GetWorld()->SweepSingleByChannel(Hit,GetActorLocation(),Next,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(32),Q)){
  SetActorLocation(Hit.Location);if(Hit.GetActor()==Bike&&Bike->ApplyDroneStrike())RiderHits++;bSpent=true;SetLifeSpan(2);return;
 }
 SetActorRotation((Next-GetActorLocation()).Rotation());SetActorLocation(Next);if(T>2){bSpent=true;SetLifeSpan(2);}
}
void ABattleLabMode::TickDrones(float Dt){
 auto* Mode=Cast<ABattleParkMode>(this);auto* Bike=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(this,0));
 if(!Mode||!Bike||bRunEnded||StartCountdown>0||Bike->RiderHealth<=0||Bike->StunRemaining>0||Bike->Ride->Recovery>0||(Mode->Enemies&&Mode->Enemies->bFreezeSpawns))return;
 DroneDelay-=Dt;if(DroneDelay>0)return;
 for(TActorIterator<ABattleDrone> It(GetWorld());It;++It)if(!It->IsActorBeingDestroyed())return;
 DroneDelay=FMath::FRandRange(60.f,100.f);
 const float A=FMath::FRandRange(-PI,PI);const FVector Spot=Bike->GetActorLocation()+FVector(FMath::Cos(A)*900,FMath::Sin(A)*900,550);
 FCollisionQueryParams Q(SCENE_QUERY_STAT(DroneSpawn),false,Bike);FHitResult Hit;
 if(GetWorld()->SweepSingleByChannel(Hit,Spot,Bike->GetActorLocation(),FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(50),Q))return;
 GetWorld()->SpawnActor<ABattleDrone>(Spot,(Bike->GetActorLocation()-Spot).Rotation());
}
bool ABattleBike::ApplyDroneStrike(){
 if(bParked||StunRemaining>0||Ride->Recovery>0||ApplyRiderDamage(15)<=0)return false;
 if(RiderHealth<=0)return true;
 Dismount();StunRemaining=2;StunLabel=TEXT("DRONE IMPACT");Ride->Speed=Ride->Pedal=Ride->Steer=0;Ride->StopMovementImmediately();Ride->BoostRemaining=0;UpdateStun(0);RideImpact(.8f);return true;
}
