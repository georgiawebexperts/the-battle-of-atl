#include "BattlePothole.h"
#include "BattleBike.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
void TickBattlePotholeAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 static TWeakObjectPtr<UWorld> World;static bool Done=false;if(World!=PC->GetWorld()){World=PC->GetWorld();Done=false;}if(Done||PC->GetWorld()->GetTimeSeconds()<5)return;Done=true;
 auto* Bike=Cast<ABattleBike>(PC->GetPawn());auto* Hole=PC->GetWorld()->SpawnActor<ABattlePothole>(FVector(0,0,0),FRotator::ZeroRotator);Hole->SetActorTickEnabled(false);
 auto Finish=[&](bool Pass,const TCHAR* Why){UE_LOG(LogTemp,Display,TEXT("PotholeAudit: {\"passed\":%s,\"reason\":\"%s\",\"contacts\":%d}"),Pass?TEXT("true"):TEXT("false"),Why,Hole->Contacts);Hole->Destroy();PC->ConsoleCommand(TEXT("quit"));};
 if(!Bike){Finish(false,TEXT("Missing ridden bike"));return;}auto* M=Bike->Ride.Get();M->Recovery=0;M->SetMovementMode(MOVE_Walking);M->Speed=800;
 const FVector A(-300,0,90),B(300,0,90);
 if(Hole->EvaluateTraversal(Bike,A+FVector(0,100,0),B+FVector(0,100,0))){Finish(false,TEXT("Near miss triggered"));return;}
 M->SetMovementMode(MOVE_Falling);if(Hole->EvaluateTraversal(Bike,A,B)){Finish(false,TEXT("Airborne crossing triggered"));return;}M->SetMovementMode(MOVE_Walking);
 Bike->bParked=true;if(Hole->EvaluateTraversal(Bike,A,B)){Finish(false,TEXT("Parked bike triggered"));return;}Bike->bParked=false;
 const int Initial=M->Wipeouts;
 if(!Hole->EvaluateTraversal(Bike,A,B)||!FMath::IsNearlyEqual(M->Speed,576.f)||M->Wipeouts!=Initial){Finish(false,TEXT("Swept shallow crossing did not scrub speed"));return;}
 if(Hole->EvaluateTraversal(Bike,FVector(0,0,90),FVector(0,0,90))||Hole->Contacts!=1){Finish(false,TEXT("Duplicate impact while inside"));return;}
 Hole->EvaluateTraversal(Bike,B,B);Hole->bDeep=true;M->Speed=1200;
 if(!Hole->EvaluateTraversal(Bike,A,B)||M->Wipeouts!=Initial+1||M->Recovery<=0||M->Speed!=0||Hole->Contacts!=2){Finish(false,TEXT("Rearmed deep crossing did not wipe out"));return;}
 Finish(true,TEXT("Swept hit, near miss, airborne/parked exclusion, latch and deep-speed recovery passed"));
#endif
}
