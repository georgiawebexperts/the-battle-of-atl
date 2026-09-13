#include "BattleBike.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
void TickBattleBikeBalanceAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 static int Phase=0;static float Clock=0,MaxGround=0,MaxPedal=0,MaxGrip=0;static TWeakObjectPtr<ACameraActor> Camera;static int TransitionShots=0;
 if(Phase==99||PC->GetWorld()->GetTimeSeconds()<5)return;auto* B=Cast<ABattleBike>(PC->GetPawn());if(!B)return;Clock+=Dt;
 auto Key=[&](FKey K,bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Finish=[&](bool Pass,const TCHAR* Why){UE_LOG(LogTemp,Display,TEXT("BikeBalanceAudit: {\"passed\":%s,\"phase\":%d,\"ground_error_cm\":%.3f,\"pedal_error_cm\":%.3f,\"grip_error_cm\":%.3f,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Phase,MaxGround,MaxPedal,MaxGrip,Why);Phase=99;Key(EKeys::W,false);Key(EKeys::SpaceBar,false);PC->ConsoleCommand(TEXT("quit"));};
 FString Dir;FParse::Value(FCommandLine::Get(),TEXT("BattleBalanceReviewDir="),Dir);
 if(!Dir.IsEmpty()){
  if(!Camera.IsValid())Camera=PC->GetWorld()->SpawnActor<ACameraActor>();const FTransform Frame=B->Visual->GetComponentTransform();FVector Eye=Frame.TransformPosition(FVector(140,-290,100)),Target=Frame.TransformPosition(FVector(0,0,65));if(FParse::Param(FCommandLine::Get(),TEXT("BattleAerialWaterReview"))){Eye=FVector(1800,-6500,5000);Target=FVector(-3500,0,-330);}Camera->SetActorLocationAndRotation(Eye,(Target-Eye).Rotation());PC->SetViewTarget(Camera.Get());
 }
 auto Shot=[&](const TCHAR* Name){if(!Dir.IsEmpty())FScreenshotRequest::RequestScreenshot(Dir/(FString(Name)+TEXT(".png")),true,false);};
 // The old feet-only check missed the lowered pelvis passing through the seat.
 const FVector Hip=B->Visual->GetComponentTransform().InverseTransformPosition(B->Rider->GetBoneLocation(TEXT("pelvis"),EBoneSpaces::WorldSpace));
 if(Phase>0&&Phase<4&&Hip.Z<96.f&&Hip.X<8.f){Finish(false,TEXT("Lowered pelvis overlaps saddle envelope"));return;}
 if(Phase==2&&Clock>.08f*(TransitionShots+1)&&TransitionShots<4){Shot(*FString::Printf(TEXT("depart-%d"),TransitionShots++));}
 if(Phase==3&&Clock>.08f*(TransitionShots+1)&&TransitionShots<12){Shot(*FString::Printf(TEXT("brake-%d"),TransitionShots++));}
 if(Phase>0&&Phase<4){
  for(int Sign:{-1,1}){
   const FVector Wrist=B->Rider->GetBoneLocation(Sign>0?TEXT("hand_l"):TEXT("hand_r"),EBoneSpaces::WorldSpace);
   const FVector Grip=B->SteeringAssembly->GetComponentTransform().TransformPosition(FVector(31,-Sign*25,115)-B->SteeringAssembly->GetRelativeLocation());
   MaxGrip=FMath::Max(MaxGrip,float(FVector::Distance(Wrist,Grip)));
   if(MaxGrip>3.f){Finish(false,TEXT("Stop/start pose pulled wrist away from handlebar"));return;}
  }
 }
 auto CheckFeet=[&](bool Grounded){
  for(int Sign:{-1,1}){
   if(B->Rider->GetBoneIndex(Sign>0?TEXT("ball_l"):TEXT("ball_r"))<0)return false;
   const FVector Sole=B->Rider->GetBoneLocation(Sign>0?TEXT("ball_l"):TEXT("ball_r"),EBoneSpaces::WorldSpace)+B->Rider->GetComponentTransform().TransformVector(FVector(0,0,-2.184858f-(Sign>0?.750843f:.751438f)));
   float Error=999;
   if(Grounded){FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(BalanceAuditFloor),false,B);if(PC->GetWorld()->LineTraceSingleByChannel(Hit,Sole+FVector(0,0,50),Sole-FVector(0,0,100),ECC_Visibility,Q))Error=FMath::Abs(Sole.Z-Hit.ImpactPoint.Z);MaxGround=FMath::Max(MaxGround,Error);}
   else {auto* Pedal=Cast<UStaticMeshComponent>(B->GetDefaultSubobjectByName(*FString::Printf(TEXT("Pedal%d"),Sign)));if(Pedal)Error=FVector::Dist(Sole,Pedal->GetComponentTransform().TransformPosition(FVector(0,0,50)));MaxPedal=FMath::Max(MaxPedal,Error);}
   if(Error>3)return false;
  }return true;
 };
 if(Phase==0){B->Ride->StopMovementImmediately();B->Ride->Speed=B->Ride->ReverseSpeed=0;Key(EKeys::SpaceBar,true);Clock=0;Phase=1;}
 else if(Phase==1&&Clock>1){Shot(TEXT("stopped"));if(!CheckFeet(true)){Finish(false,TEXT("Stopped shoes did not reach ground"));return;}Key(EKeys::SpaceBar,false);Key(EKeys::W,true);Clock=0;Phase=2;}
 else if(Phase==2&&Clock>1.3f){Shot(TEXT("pedaling"));if(B->Ride->Speed<150||!CheckFeet(false)){Finish(false,TEXT("Moving shoes did not return to pedals"));return;}Key(EKeys::W,false);Key(EKeys::SpaceBar,true);Clock=0;TransitionShots=0;Phase=3;}
 else if(Phase==3&&Clock>1.7f){Shot(TEXT("stopped-again"));if(B->Ride->Speed>1||!CheckFeet(true)){Finish(false,TEXT("Braking did not plant shoes again"));return;}Clock=0;Phase=4;}
 else if(Phase==4&&Clock>.3f)Finish(true,TEXT("Grounded stop, moving pedal contact, braking and replant pass"));
#endif
}
