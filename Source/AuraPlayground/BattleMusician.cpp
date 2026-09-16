#include "BattleMusician.h"
#include "BattleFrisbee.h"
#include "PiedmontPedestrian.h"
#include "AIController.h"
#include "Camera/CameraActor.h"
#include "Components/AudioComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
#include "UObject/ConstructorHelpers.h"

UBattleInstrumentLoop::UBattleInstrumentLoop(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer){
 NumChannels=1;SetSampleRate(24000);Duration=INDEFINITELY_LOOPING_DURATION;bLooping=true;SoundGroup=SOUNDGROUP_Music;
}
int32 UBattleInstrumentLoop::OnGeneratePCMAudio(TArray<uint8>& OutAudio,int32 NumSamples){
 OutAudio.SetNumUninitialized(NumSamples*sizeof(int16));auto* Samples=reinterpret_cast<int16*>(OutAudio.GetData());constexpr double Rate=24000.0;
 const double GuitarNotes[]={196.00,246.94,293.66,392.00,293.66,246.94,220.00,329.63};
 const double SaxNotes[]={293.66,329.63,392.00,440.00,392.00,349.23,329.63,261.63};
 for(int32 I=0;I<NumSamples;I++,Cursor++){
  const double Time=Cursor/Rate,Beat=FMath::Fmod(Time,.48);const int32 Index=FMath::FloorToInt(Time/.48)%8;const double Frequency=InstrumentKind==0?GuitarNotes[Index]:SaxNotes[Index];
  double Value=0;
  if(InstrumentKind==0){
   const double Envelope=FMath::Exp(-Beat*5.2);Value=Envelope*(FMath::Sin(2*PI*Frequency*Time)+.45*FMath::Sin(2*PI*Frequency*2.01*Time)+.2*FMath::Sin(2*PI*Frequency*3.02*Time));
  }else{
   const double Envelope=FMath::Min(1.0,Beat/.06)*FMath::Min(1.0,(.48-Beat)/.08),Vibrato=1.0+.006*FMath::Sin(2*PI*5.2*Time);
   Value=Envelope*(FMath::Sin(2*PI*Frequency*Vibrato*Time)+.34*FMath::Sin(2*PI*Frequency*2*Time)+.12*FMath::Sin(2*PI*Frequency*3*Time));
  }
  Samples[I]=int16(FMath::Clamp(Value*(InstrumentKind==0?6200.0:5200.0),-30000.0,30000.0));
 }
 return NumSamples;
}

ABattleMusician::ABattleMusician(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("MusicianRoot"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 GuitarBody=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GuitarBody"));GuitarBody->SetupAttachment(RootComponent);GuitarBody->SetStaticMesh(Sphere.Object);GuitarBody->SetRelativeLocation(FVector(27,0,94));GuitarBody->SetRelativeRotation(FRotator(0,18,-18));GuitarBody->SetRelativeScale3D(FVector(.44,.12,.44));
 GuitarUpper=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GuitarUpper"));GuitarUpper->SetupAttachment(RootComponent);GuitarUpper->SetStaticMesh(Sphere.Object);GuitarUpper->SetRelativeLocation(FVector(27,0,121));GuitarUpper->SetRelativeRotation(FRotator(0,18,-18));GuitarUpper->SetRelativeScale3D(FVector(.34,.115,.34));
 GuitarNeck=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GuitarNeck"));GuitarNeck->SetupAttachment(RootComponent);GuitarNeck->SetStaticMesh(Cube.Object);GuitarNeck->SetRelativeLocation(FVector(30,0,158));GuitarNeck->SetRelativeRotation(FRotator(-86,0,0));GuitarNeck->SetRelativeScale3D(FVector(.70,.045,.045));
 GuitarHole=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GuitarSoundHole"));GuitarHole->SetupAttachment(RootComponent);GuitarHole->SetStaticMesh(Cylinder.Object);GuitarHole->SetRelativeLocation(FVector(27,-7,111));GuitarHole->SetRelativeRotation(FRotator(0,0,90));GuitarHole->SetRelativeScale3D(FVector(.105,.105,.012));
 GuitarHoleBack=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GuitarSoundHoleBack"));GuitarHoleBack->SetupAttachment(RootComponent);GuitarHoleBack->SetStaticMesh(Cylinder.Object);GuitarHoleBack->SetRelativeLocation(FVector(27,7,111));GuitarHoleBack->SetRelativeRotation(FRotator(0,0,90));GuitarHoleBack->SetRelativeScale3D(FVector(.105,.105,.012));
 SaxBody=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SaxBody"));SaxBody->SetupAttachment(RootComponent);SaxBody->SetStaticMesh(Cylinder.Object);SaxBody->SetRelativeLocation(FVector(28,0,105));SaxBody->SetRelativeRotation(FRotator(0,0,8));SaxBody->SetRelativeScale3D(FVector(.055,.055,.35));
 SaxBell=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SaxBell"));SaxBell->SetupAttachment(RootComponent);SaxBell->SetStaticMesh(Sphere.Object);SaxBell->SetRelativeLocation(FVector(27,0,73));SaxBell->SetRelativeScale3D(FVector(.12,.12,.10));
 Music=CreateDefaultSubobject<UAudioComponent>(TEXT("InstrumentAudio"));Music->SetupAttachment(RootComponent);Music->SetRelativeLocation(FVector(0,0,110));Music->bAutoActivate=false;Music->bOverrideAttenuation=true;Music->AttenuationOverrides.bAttenuate=true;Music->AttenuationOverrides.bSpatialize=true;Music->AttenuationOverrides.AttenuationShapeExtents=FVector(150);Music->AttenuationOverrides.FalloffDistance=1700;
 for(auto* Part:{GuitarBody.Get(),GuitarUpper.Get(),GuitarNeck.Get(),GuitarHole.Get(),GuitarHoleBack.Get(),SaxBody.Get(),SaxBell.Get()}){Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);Part->SetCanEverAffectNavigation(false);}
 Tags.Add(TEXT("BattleParkMusician"));
}
void ABattleMusician::BeginPlay(){
 Super::BeginPlay();
 const bool Guitar=InstrumentKind==0;GuitarBody->SetVisibility(Guitar);GuitarUpper->SetVisibility(Guitar);GuitarNeck->SetVisibility(Guitar);GuitarHole->SetVisibility(Guitar);GuitarHoleBack->SetVisibility(Guitar);SaxBody->SetVisibility(!Guitar);SaxBell->SetVisibility(!Guitar);
 auto* Wood=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/PiedmontRide/Materials/M_BridgeWood.M_BridgeWood"));auto* Gold=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/PiedmontRide/Materials/M_Safety.M_Safety"));auto* Dark=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/PiedmontRide/Materials/M_Asphalt.M_Asphalt"));
 GuitarBody->SetMaterial(0,Wood);GuitarUpper->SetMaterial(0,Wood);GuitarNeck->SetMaterial(0,Wood);GuitarHole->SetMaterial(0,Dark);GuitarHoleBack->SetMaterial(0,Dark);SaxBody->SetMaterial(0,Gold);SaxBell->SetMaterial(0,Gold);
 FTransform T(GetActorRotation(),GetActorLocation()+FVector(0,0,88));auto* Person=GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),T,this,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding);
 if(Person){Person->bParkMusician=true;Person->MusicianKind=InstrumentKind;Person->CityAppearanceVariant=InstrumentKind+4;Person->CityOutfitVariant=InstrumentKind+4;Person->FinishSpawning(T);Performer=Person;}
 Loop=NewObject<UBattleInstrumentLoop>(this);Loop->InstrumentKind=InstrumentKind;Music->SetSound(Loop);Music->SetVolumeMultiplier(Guitar?.22f:.16f);Music->Play();
}
void ABattleMusician::Tick(float Dt){
 Super::Tick(Dt);const bool Performing=Performer&&!Performer->bDead&&Performer->PanicRemaining<=0&&Performer->StumbleRemaining<=0;
 for(auto* Part:{GuitarBody.Get(),GuitarUpper.Get(),GuitarNeck.Get(),GuitarHole.Get(),GuitarHoleBack.Get(),SaxBody.Get(),SaxBell.Get()})Part->SetVisibility(Performing&&((InstrumentKind==0)==(Part==GuitarBody||Part==GuitarUpper||Part==GuitarNeck||Part==GuitarHole||Part==GuitarHoleBack)));
 if(Performing){if(!Music->IsPlaying())Music->Play();}else Music->Stop();
}
void ABattleMusician::EndPlay(const EEndPlayReason::Type Reason){if(IsValid(Performer))Performer->Destroy();Super::EndPlay(Reason);}

ABattleParkMusicDirector::ABattleParkMusicDirector(){PrimaryActorTick.bCanEverTick=true;}
void ABattleParkMusicDirector::BeginPlay(){
 Super::BeginPlay();
#if !UE_BUILD_SHIPPING
 const bool OwnAudit=FParse::Param(FCommandLine::Get(),TEXT("BattleMusicianAudit"));if(FString(FCommandLine::Get()).Contains(TEXT("Audit"))&&!OwnAudit)return;
#endif
 // OSM-derived one-third-scale locations: Dockside/boathouse and the inside of the 12th Street gate.
 const FVector Sites[]={FVector(-15520,3150,0),FVector(-16575,-3533,0)};
 for(int32 Kind=0;Kind<2;Kind++){FVector Ground;if(!ABattleFrisbeeGroup::Ground(GetWorld(),Sites[Kind],Ground,false))continue;auto* Spot=GetWorld()->SpawnActorDeferred<ABattleMusician>(ABattleMusician::StaticClass(),FTransform(FRotator(0,Kind?65:-25,0),Ground+FVector(0,0,8)),this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);if(Spot){Spot->InstrumentKind=Kind;Spot->FinishSpawning(FTransform(FRotator(0,Kind?65:-25,0),Ground+FVector(0,0,8)));Musicians.Add(Spot);}}
}
void ABattleParkMusicDirector::Tick(float Dt){
 Super::Tick(Dt);if(bAuditDone)return;
#if !UE_BUILD_SHIPPING
 if(!FParse::Param(FCommandLine::Get(),TEXT("BattleMusicianAudit")))return;AuditClock+=Dt;
 if(Musicians.Num()!=2){if(AuditClock>3){UE_LOG(LogTemp,Display,TEXT("BattleMusicianAudit: {\"passed\":false,\"musicians\":%d,\"reason\":\"location grounding failed\"}"),Musicians.Num());bAuditDone=true;if(auto* PC=UGameplayStatics::GetPlayerController(this,0))UKismetSystemLibrary::QuitGame(this,PC,EQuitPreference::Quit,false);}return;}
 FString ReviewDir;const bool Review=FParse::Value(FCommandLine::Get(),TEXT("BattleMusicianReviewDir="),ReviewDir);
 if(Review&&!bReviewSetup&&AuditClock>.25f){const FVector Focus=Musicians[0]->GetActorLocation()+FVector(0,0,95),Eye=Focus+FVector(-450,-420,130);if(auto* PC=UGameplayStatics::GetPlayerController(this,0))if(auto* Camera=GetWorld()->SpawnActor<ACameraActor>(Eye,(Focus-Eye).Rotation()))PC->SetViewTarget(Camera);bReviewSetup=true;}
 if(!bAuditSampled&&AuditClock>.6f){for(auto M:Musicians)AuditHands.Add(M->Performer->Body->GetSocketLocation(TEXT("hand_r")));bAuditSampled=true;}
 if(bAuditSampled)for(int32 I=0;I<Musicians.Num()&&I<AuditHands.Num();I++)if(auto* M=Musicians[I].Get())if(M->Performer)AuditHandTravel=FMath::Max(AuditHandTravel,float(FVector::Dist(AuditHands[I],M->Performer->Body->GetSocketLocation(TEXT("hand_r")))));
 if(Review&&!ReviewDir.IsEmpty()&&AuditClock>5.2f&&AuditClock-Dt<=5.2f)FScreenshotRequest::RequestScreenshot(ReviewDir/TEXT("park-guitarist.png"),false,false);
 if(AuditClock<(Review?6.1f:1.5f))return;
 const float HandTravel=AuditHandTravel;bool Ready=true,Audio=true;for(int32 I=0;I<Musicians.Num();I++){auto* M=Musicians[I].Get();Ready&=M&&M->Performer&&M->Performer->bNativeCrowdRig&&M->Performer->MusicClock>1;Audio&=M&&M->Music->Sound==M->Loop;}
 TArray<uint8> GuitarSamples,SaxSamples;Musicians[0]->Loop->OnGeneratePCMAudio(GuitarSamples,512);Musicians[1]->Loop->OnGeneratePCMAudio(SaxSamples,512);const bool Generated=GuitarSamples.ContainsByPredicate([](uint8 V){return V!=0;})&&SaxSamples.ContainsByPredicate([](uint8 V){return V!=0;});
 Musicians[0]->Performer->HearGunfire(Musicians[0]->GetActorLocation());Musicians[0]->Tick(.01f);const bool Reacted=Musicians[0]->Performer->PanicRemaining>0&&!Musicians[0]->Music->IsPlaying()&&!Musicians[0]->GuitarBody->IsVisible();
 const bool Pass=Ready&&Audio&&Generated&&HandTravel>5&&Reacted;UE_LOG(LogTemp,Display,TEXT("BattleMusicianAudit: {\"passed\":%s,\"musicians\":%d,\"hand_travel_cm\":%.2f,\"audio_generated\":%s,\"panic_response\":%s}"),Pass?TEXT("true"):TEXT("false"),Musicians.Num(),HandTravel,Generated?TEXT("true"):TEXT("false"),Reacted?TEXT("true"):TEXT("false"));bAuditDone=true;FTimerHandle Quit;GetWorldTimerManager().SetTimer(Quit,[this](){if(auto* PC=UGameplayStatics::GetPlayerController(this,0))UKismetSystemLibrary::QuitGame(this,PC,EQuitPreference::Quit,false);},Review?.8f:.05f,false);
#endif
}
