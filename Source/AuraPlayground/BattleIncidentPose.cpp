#include "PiedmontPedestrian.h"
#include "Animation/AnimSequence.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PiedmontBike.h"
#include "Kismet/GameplayStatics.h"

bool APiedmontPedestrian::BeginIncidentPose(UAnimSequence* Clip,float PoseSeconds,float HoldSeconds){
 if(!Clip||!FMath::IsFinite(PoseSeconds)||!FMath::IsFinite(HoldSeconds)||HoldSeconds<=0||PoseSeconds<0||PoseSeconds>=Clip->GetPlayLength())return false;
 if(!bNativeCrowdRig||bDead||bSwimming||KnockdownPhase||StumbleRemaining>0||SleepPhase||bBenchReaching||bIncidentPosing)return false;
 if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();
 GetCharacterMovement()->StopMovementImmediately();bHasDestination=false;PauseRemaining=0;
 SetBodySequence(Clip,false);HoldBodySequenceAt(PoseSeconds);
 IncidentHoldRemaining=HoldSeconds;bIncidentPosing=true;bIncidentRecovering=false;AnimateBody(0);
 return true;
}
void APiedmontPedestrian::ReleaseIncidentPose(){
 if(!bIncidentPosing||bIncidentRecovering)return;
 ResumeBodySequence();bIncidentRecovering=true;
}
void APiedmontPedestrian::CancelIncidentPose(){
 if(!bIncidentPosing)return;
 bIncidentPosing=false;bIncidentRecovering=false;IncidentHoldRemaining=0;StopBodySequence();
}
bool APiedmontPedestrian::TickIncidentPose(float Dt){
 if(!bIncidentPosing)return false;
 auto* Mode=Cast<APiedmontRideMode>(UGameplayStatics::GetGameMode(this));
 if(bDead||bSwimming||!Mode||Mode->bRunEnded){CancelIncidentPose();return false;}
 if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();
 GetCharacterMovement()->StopMovementImmediately();bHasDestination=false;
 if(!bIncidentRecovering){IncidentHoldRemaining-=Dt;if(IncidentHoldRemaining<=0)ReleaseIncidentPose();}
 if(bIncidentRecovering&&!IsBodySequencePlaying()){CancelIncidentPose();PauseRemaining=1.f;}
 return true;
}
