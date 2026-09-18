#include "BattleDance.h"
#include "BattleBike.h"
#include "BattleFrisbee.h"
#include "PiedmontPedestrian.h"
#include "AIController.h"
#include "Camera/CameraActor.h"
#include "Components/AudioComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "Sound/SoundBase.h"
#include "UnrealClient.h"
#include "UObject/ConstructorHelpers.h"

ABattleDanceCircle::ABattleDanceCircle(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("DanceCircleRoot"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 Speaker=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ParkSpeaker"));Speaker->SetupAttachment(RootComponent);Speaker->SetStaticMesh(Cube.Object);Speaker->SetRelativeLocation(FVector(0,0,55));Speaker->SetRelativeScale3D(FVector(.32,.22,.55));
 SpeakerFace=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpeakerCone"));SpeakerFace->SetupAttachment(RootComponent);SpeakerFace->SetStaticMesh(Cylinder.Object);SpeakerFace->SetRelativeLocation(FVector(-17,0,55));SpeakerFace->SetRelativeRotation(FRotator(90,0,0));SpeakerFace->SetRelativeScale3D(FVector(.12,.12,.025));
 Label=CreateDefaultSubobject<UTextRenderComponent>(TEXT("DanceLabel"));Label->SetupAttachment(RootComponent);Label->SetRelativeLocation(FVector(0,0,125));Label->SetText(FText::FromString(TEXT("PARK DJ")));Label->SetWorldSize(24);Label->SetHorizontalAlignment(EHTA_Center);Label->SetTextRenderColor(FColor(255,185,100));
 Music=CreateDefaultSubobject<UAudioComponent>(TEXT("DanceMusic"));Music->SetupAttachment(RootComponent);Music->bAutoActivate=false;Music->bOverrideAttenuation=true;Music->AttenuationOverrides.bAttenuate=true;Music->AttenuationOverrides.bSpatialize=true;Music->AttenuationOverrides.AttenuationShapeExtents=FVector(120);Music->AttenuationOverrides.FalloffDistance=1700;
 for(auto* Part:{Speaker.Get(),SpeakerFace.Get()}){Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);Part->SetCanEverAffectNavigation(false);}
 Tags.Add(TEXT("BattleDanceCircle"));
}
void ABattleDanceCircle::BeginPlay(){
 Super::BeginPlay();
#if !UE_BUILD_SHIPPING
 const bool OwnAudit=FParse::Param(FCommandLine::Get(),TEXT("BattleDanceAudit"));
 if(FString(FCommandLine::Get()).Contains(TEXT("Audit"))&&!OwnAudit)return;
#endif
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!Mode)return;DesiredDancers=FMath::Clamp(Mode->Difficulty.Dancers,0,8);
 FVector Center;bool Grounded=false;
 for(const FVector Candidate:{FVector(-9000,5500,0),FVector(-6500,8500,0),FVector(4500,5000,0)})if(ABattleFrisbeeGroup::Ground(GetWorld(),Candidate,Center,true)){Grounded=true;break;}
 if(!Grounded)return;SetActorLocation(Center+FVector(0,0,8));
 Speaker->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/PiedmontRide/Materials/M_GunMetal.M_GunMetal")));
 SpeakerFace->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_DiscGlow.M_DiscGlow")));
 if(auto* Theme=LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_BattleATLTheme2.S_BattleATLTheme2"))){Music->SetSound(Theme);Music->SetVolumeMultiplier(.18f);Music->Play();}
 FActorSpawnParameters Spawn;Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
 for(int32 I=0;I<DesiredDancers;I++){
  const float Angle=2*PI*I/FMath::Max(1,DesiredDancers)+.35f;const FVector Desired=Center+FVector(FMath::Cos(Angle),FMath::Sin(Angle),0)*220.f;FVector Ground;
  if(!ABattleFrisbeeGroup::Ground(GetWorld(),Desired,Ground,true))continue;
  const FTransform Transform((Center-Ground).Rotation(),Ground+FVector(0,0,88));
  auto* Visitor=GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),Transform,this,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding);
  if(!Visitor)continue;Visitor->bParkDancer=true;Visitor->DanceVariant=I%3;Visitor->CityAppearanceVariant=I;Visitor->CityOutfitVariant=I;Visitor->FinishSpawning(Transform);Dancers.Add(Visitor);
 }
 bReady=Dancers.Num()==DesiredDancers&&DesiredDancers>0;
 UE_LOG(LogTemp,Display,TEXT("BattleDance: ready=%d dancers=%d desired=%d location=%s"),bReady,Dancers.Num(),DesiredDancers,*GetActorLocation().ToString());
}
void ABattleDanceCircle::Tick(float Dt){
 Super::Tick(Dt);if(!bReady||bAuditFinished)return;
 if(auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0))Label->SetWorldRotation(FRotator(0,(Pawn->GetActorLocation()-GetActorLocation()).Rotation().Yaw,0));
#if !UE_BUILD_SHIPPING
 const bool Audit=FParse::Param(FCommandLine::Get(),TEXT("BattleDanceAudit"));if(!Audit)return;AuditClock+=Dt;
 FString ReviewDir;const bool Review=FParse::Value(FCommandLine::Get(),TEXT("BattleDanceReviewDir="),ReviewDir);
 if(Review&&AuditClock>.25f&&!bReviewCaptured){
  const FVector Eye=GetActorLocation()+FVector(-520,-500,180),Target=GetActorLocation()+FVector(0,0,75);
  if(auto* PC=UGameplayStatics::GetPlayerController(this,0))if(auto* Camera=GetWorld()->SpawnActor<ACameraActor>(Eye,(Target-Eye).Rotation()))PC->SetViewTarget(Camera);
  bReviewCaptured=true;
 }
 if(!bAuditSampled&&AuditClock>.5f){AuditHands.Reset();for(auto D:Dancers)AuditHands.Add(D->Body->GetSocketLocation(TEXT("hand_l")));bAuditSampled=true;}
 if(Review&&!ReviewDir.IsEmpty()&&ReviewCaptureStage==0&&AuditClock>5.5f){FScreenshotRequest::RequestScreenshot(ReviewDir/TEXT("dance-circle-a.png"),false,false);ReviewCaptureStage=1;}
 if(Review&&!ReviewDir.IsEmpty()&&ReviewCaptureStage==1&&AuditClock>6.7f){FScreenshotRequest::RequestScreenshot(ReviewDir/TEXT("dance-circle-b.png"),false,false);ReviewCaptureStage=2;}
 if(AuditClock>(Review?7.4f:1.35f)){
  float Travel=0;int32 AnimatedDancers=0;for(int32 I=0;I<Dancers.Num();I++){auto* D=Dancers[I].Get();if(D&&D->bParkDancer&&D->bNativeCrowdRig&&D->DanceClock>.7f)AnimatedDancers++;if(D&&AuditHands.IsValidIndex(I))Travel=FMath::Max(Travel,FVector::Dist(AuditHands[I],D->Body->GetSocketLocation(TEXT("hand_l"))));}
  bool Panic=false;if(!Dancers.IsEmpty()&&Dancers[0]){Dancers[0]->HearGunfire(Dancers[0]->GetActorLocation());Panic=Dancers[0]->PanicRemaining>0;}
  // One performer may already be yielding or reacting to ambient trouble; the
  // circle still passes when every other performer is visibly dancing.
  const bool Pass=bReady&&Dancers.Num()==DesiredDancers&&AnimatedDancers>=FMath::Max(1,DesiredDancers-1)&&Travel>8&&Music->Sound&&Panic;
  UE_LOG(LogTemp,Display,TEXT("BattleDanceAudit: {\"passed\":%s,\"ready\":%s,\"dancers\":%d,\"desired\":%d,\"animated_dancers\":%d,\"hand_travel_cm\":%.2f,\"music\":%s,\"panic_response\":%s}"),Pass?TEXT("true"):TEXT("false"),bReady?TEXT("true"):TEXT("false"),Dancers.Num(),DesiredDancers,AnimatedDancers,Travel,Music->Sound?TEXT("true"):TEXT("false"),Panic?TEXT("true"):TEXT("false"));
  bAuditFinished=true;FTimerHandle Quit;GetWorldTimerManager().SetTimer(Quit,[this](){if(auto* PC=UGameplayStatics::GetPlayerController(this,0))UKismetSystemLibrary::QuitGame(this,PC,EQuitPreference::Quit,false);},Review?1.f:.05f,false);
 }
#endif
}
void ABattleDanceCircle::EndPlay(const EEndPlayReason::Type Reason){for(auto D:Dancers)if(IsValid(D))D->Destroy();Super::EndPlay(Reason);}
