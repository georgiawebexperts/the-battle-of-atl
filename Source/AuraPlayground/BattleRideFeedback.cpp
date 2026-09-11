#include "BattleBike.h"
#include "BattleRideFX.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "EngineUtils.h"
void ABattleBike::RideImpact(float Strength,bool Water){
 FeedbackStrength=FMath::Max(FeedbackStrength,Strength);ImpactEvents++;
 if(Water&&RideEffects){FVector Surface=GetActorLocation()-FVector(0,0,90);for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->ContainsBike(GetActorLocation())){Surface.Z=It->GetActorLocation().Z+4;break;}RideEffects->Splash(Surface);}
 const TCHAR* Path=Water?TEXT("/Game/BattleForTheA/Audio/S_Splash.S_Splash"):TEXT("/Game/BattleForTheA/Audio/S_Bump.S_Bump");
 if(auto* Sound=LoadObject<USoundBase>(nullptr,Path))UGameplayStatics::PlaySoundAtLocation(this,Sound,GetActorLocation(),FMath::Clamp(Strength,.15f,1.f));
}
void ABattleBike::UpdateRideFeedback(float Dt){
 FeedbackClock+=Dt;FeedbackStrength=FMath::FInterpTo(FeedbackStrength,0.f,Dt,5);BumpCooldown=FMath::Max(0.f,BumpCooldown-Dt);
 const FVector Position=GetActorLocation(),Travel=Position-PreviousFeedbackLocation;const bool Riding=!bParked&&GetController()&&Ride->Recovery<=0;
 const bool Grounded=Ride->IsMovingOnGround();const float Vertical=Travel.Z/FMath::Max(Dt,.001f);
 const bool Continuous=Travel.Size()<FMath::Max(100.f,Ride->Speed*Dt*3);
 if(Riding&&Grounded&&Continuous&&Ride->Speed>200&&BumpCooldown<=0&&FMath::Abs(Vertical-PreviousVertical)>180&&FMath::Abs(Travel.Z)>2){RideImpact(.35f,false);Ride->Speed*=.97f;BumpCooldown=.3f;TerrainBumps++;}
 PreviousVertical=Continuous?Vertical:0;PreviousFeedbackLocation=Position;
 const float Rough=Riding&&Grounded&&Ride->bGrass?FMath::Clamp(Ride->Speed/1600.f,0.f,1.f)*.3f:0;
 const float Motion=FeedbackStrength+Rough;
 const FVector Offset(0,FMath::Sin(FeedbackClock*47)*Motion*1.4f,FMath::Sin(FeedbackClock*61)*Motion*2.2f);
 Chase->SetRelativeLocation(Offset);Chase->SetRelativeRotation(FRotator(FMath::Sin(FeedbackClock*39)*Motion*.55f,0,0));
 Handlebar->SetRelativeLocation(FVector(37,0,151)+Offset);Handlebar->SetRelativeRotation(FRotator(FMath::Sin(FeedbackClock*39)*Motion*.55f,0,0));
 Visual->SetRelativeLocation(FVector(0,0,-96+FMath::Sin(FeedbackClock*31)*Motion*1.5f));
 const float SpeedMix=Riding&&Grounded?FMath::Clamp(Ride->Speed/1600.f,0.f,1.f):0;
 AsphaltAudio->SetVolumeMultiplier(SpeedMix*(Ride->bGrass?0:.7f));GrassAudio->SetVolumeMultiplier(SpeedMix*(Ride->bGrass?.9f:0));MotorAudio->SetVolumeMultiplier(Riding&&Ride->Pedal>0?SpeedMix*.7f:0);
 for(auto* Audio:{AsphaltAudio.Get(),GrassAudio.Get(),MotorAudio.Get()})Audio->SetPitchMultiplier(.65f+SpeedMix*.7f);
 const bool Sliding=Riding&&Grounded&&Ride->SlideRemaining>0&&Ride->Speed>300;
 if(Sliding&&!WasSliding)if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_Skid.S_Skid"))){UGameplayStatics::PlaySoundAtLocation(this,Sound,Position,.7f);SkidSounds++;}
 TrackDelay-=Dt;
 if(Sliding&&TrackDelay<=0&&RideEffects){
  TrackDelay=.04f;const FVector Rear=Position-GetActorForwardVector()*60;FHitResult Ground;FCollisionQueryParams Query(SCENE_QUERY_STAT(BattleSkid),false,this);
  if(GetWorld()->LineTraceSingleByChannel(Ground,Rear,Rear-FVector(0,0,180),ECC_Visibility,Query)&&Ground.ImpactNormal.Z>.4f){if(WasSliding&&HasTrackPoint)RideEffects->AddSkid(PreviousTrackPoint,Ground.ImpactPoint,Ground.ImpactNormal);PreviousTrackPoint=Ground.ImpactPoint;HasTrackPoint=true;}else HasTrackPoint=false;
 }
 if(!Sliding)HasTrackPoint=false;WasSliding=Sliding;
}

void ABattleBike::EndPlay(const EEndPlayReason::Type Reason){if(IsValid(RideEffects))RideEffects->Destroy();Super::EndPlay(Reason);}
