#include "BattleBike.h"
#include "PiedmontPathSpline.h"
#include "Components/SplineComponent.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
// Follow the actual curved lake bridge using only keyboard steering after placement.
void TickBattleBridgeTurnAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 static int Phase=0,Direction=0;static float Clock=0,MaxError=0;static USplineComponent* S=nullptr;
 if(Phase==99||PC->GetWorld()->GetTimeSeconds()<5)return;auto* B=Cast<ABattleBike>(PC->GetPawn());if(!B)return;auto* M=B->Ride.Get();Clock+=Dt;
 auto Key=[&](FKey K,bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Finish=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BridgeTurnAudit: {\"passed\":%s,\"direction\":%d,\"max_offset_cm\":%.2f,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Direction,MaxError,Reason);Phase=99;Key(EKeys::W,false);Key(EKeys::Left,false);Key(EKeys::Right,false);PC->ConsoleCommand(TEXT("quit"));};
 if(Phase==0){for(TActorIterator<APiedmontPathSpline> It(PC->GetWorld());It;++It)if(It->OsmWayId==TEXT("102679938")){S=It->Centerline;break;}if(!S){Finish(false,TEXT("Lake bridge spline missing"));return;}M->bRealHandling=FParse::Param(FCommandLine::Get(),TEXT("BattleRealHandlingAudit"));Phase=1;}
 if(Phase==1){const float D=Direction?S->GetSplineLength()-70:70;const FVector P=S->GetLocationAtDistanceAlongSpline(D,ESplineCoordinateSpace::World);const FVector F=S->GetDirectionAtDistanceAlongSpline(D,ESplineCoordinateSpace::World)*(Direction?-1:1);Key(EKeys::W,false);Key(EKeys::Left,false);Key(EKeys::Right,false);M->StopMovementImmediately();M->Speed=M->ReverseSpeed=M->Steer=M->SmoothedSteer=M->TurnRateDegrees=0;B->SetActorLocationAndRotation(P+FVector(0,0,100),F.Rotation(),false,nullptr,ETeleportType::TeleportPhysics);M->SetMovementMode(MOVE_Walking);M->bForceNextFloorCheck=true;Clock=0;Phase=2;return;}
 if(Phase==2){if(Clock<.6f)return;Key(EKeys::W,true);Clock=0;Phase=3;}
 if(Phase==3){const FVector Here=B->GetActorLocation();const float K=S->FindInputKeyClosestToWorldLocation(Here);const float D=S->GetDistanceAlongSplineAtSplineInputKey(K);const float Error=FVector::Dist2D(Here,S->GetLocationAtSplineInputKey(K,ESplineCoordinateSpace::World));MaxError=FMath::Max(MaxError,Error);
 const FVector Target=S->GetLocationAtDistanceAlongSpline(FMath::Clamp(D+(Direction?-160:160),0.f,S->GetSplineLength()),ESplineCoordinateSpace::World);const float Angle=FMath::FindDeltaAngleDegrees(B->GetActorRotation().Yaw,(Target-Here).Rotation().Yaw);Key(EKeys::Right,Angle>4);Key(EKeys::Left,Angle<-4);
 if(Error>115||M->Recovery>0||B->bParked){Finish(false,TEXT("Left bridge lane or crashed"));return;}if(Clock>25){Finish(false,TEXT("Bridge traversal stalled"));return;}
 if(Direction?D<110:D>S->GetSplineLength()-110){if(Direction==0){Direction=1;Phase=1;}else Finish(true,TEXT("Actual curved lake bridge traversed both directions with keyboard steering"));}
 }
#endif
}
