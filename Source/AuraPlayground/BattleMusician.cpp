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
 // The guitar is a flat soundboard: thin front to back, wide across the chest,
 // tall along the neck. Rest layout only; PlaceGuitar rebuilds it from the
 // performer's hands once the rig is posed.
 GuitarBody=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GuitarBody"));GuitarBody->SetupAttachment(RootComponent);GuitarBody->SetStaticMesh(Sphere.Object);GuitarBody->SetRelativeLocation(FVector(22,8,96));GuitarBody->SetRelativeScale3D(FVector(.12,.38,.42));
 GuitarUpper=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GuitarUpper"));GuitarUpper->SetupAttachment(RootComponent);GuitarUpper->SetStaticMesh(Sphere.Object);GuitarUpper->SetRelativeLocation(FVector(21.5,3,121));GuitarUpper->SetRelativeScale3D(FVector(.07,.30,.30));
 GuitarNeck=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GuitarNeck"));GuitarNeck->SetupAttachment(RootComponent);GuitarNeck->SetStaticMesh(Cube.Object);GuitarNeck->SetRelativeLocation(FVector(23,-10,148));GuitarNeck->SetRelativeRotation(FRotator(56,-85,0));GuitarNeck->SetRelativeScale3D(FVector(.44,.055,.05));
 GuitarHead=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GuitarHeadstock"));GuitarHead->SetupAttachment(RootComponent);GuitarHead->SetStaticMesh(Cube.Object);GuitarHead->SetRelativeLocation(FVector(23,-10,190));GuitarHead->SetRelativeRotation(FRotator(56,-85,0));GuitarHead->SetRelativeScale3D(FVector(.15,.08,.10));
 GuitarHole=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GuitarSoundHole"));GuitarHole->SetupAttachment(RootComponent);GuitarHole->SetStaticMesh(Cylinder.Object);GuitarHole->SetRelativeLocation(FVector(28.4,8,113));GuitarHole->SetRelativeRotation(FRotator(90,0,0));GuitarHole->SetRelativeScale3D(FVector(.105,.105,.012));
 GuitarHoleBack=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GuitarSoundHoleBack"));GuitarHoleBack->SetupAttachment(RootComponent);GuitarHoleBack->SetStaticMesh(Cylinder.Object);GuitarHoleBack->SetRelativeLocation(FVector(15.6,8,113));GuitarHoleBack->SetRelativeRotation(FRotator(90,0,0));GuitarHoleBack->SetRelativeScale3D(FVector(.105,.105,.012));
 // The body has to reach the performer's mouth and pass both hands, which sit
 // at 110 cm and 96 cm once the hold is applied.
 SaxBody=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SaxBody"));SaxBody->SetupAttachment(RootComponent);SaxBody->SetStaticMesh(Cylinder.Object);SaxBody->SetRelativeLocation(FVector(26,0,110));SaxBody->SetRelativeRotation(FRotator(0,0,8));SaxBody->SetRelativeScale3D(FVector(.05,.05,.46));
 SaxBell=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SaxBell"));SaxBell->SetupAttachment(RootComponent);SaxBell->SetStaticMesh(Sphere.Object);SaxBell->SetRelativeLocation(FVector(27,0,84));SaxBell->SetRelativeScale3D(FVector(.14,.14,.12));
 Music=CreateDefaultSubobject<UAudioComponent>(TEXT("InstrumentAudio"));Music->SetupAttachment(RootComponent);Music->SetRelativeLocation(FVector(0,0,110));Music->bAutoActivate=false;Music->bOverrideAttenuation=true;Music->AttenuationOverrides.bAttenuate=true;Music->AttenuationOverrides.bSpatialize=true;Music->AttenuationOverrides.AttenuationShapeExtents=FVector(150);Music->AttenuationOverrides.FalloffDistance=1700;
 for(auto* Part:{GuitarBody.Get(),GuitarUpper.Get(),GuitarNeck.Get(),GuitarHead.Get(),GuitarHole.Get(),GuitarHoleBack.Get(),SaxBody.Get(),SaxBell.Get()}){Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);Part->SetCanEverAffectNavigation(false);}
 Tags.Add(TEXT("BattleParkMusician"));
}
void ABattleMusician::BeginPlay(){
 Super::BeginPlay();
 const bool Guitar=InstrumentKind==0;GuitarBody->SetVisibility(Guitar);GuitarUpper->SetVisibility(Guitar);GuitarNeck->SetVisibility(Guitar);GuitarHead->SetVisibility(Guitar);GuitarHole->SetVisibility(Guitar);GuitarHoleBack->SetVisibility(Guitar);SaxBody->SetVisibility(!Guitar);SaxBell->SetVisibility(!Guitar);
 auto* Wood=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/PiedmontRide/Materials/M_BridgeWood.M_BridgeWood"));auto* Gold=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/PiedmontRide/Materials/M_Safety.M_Safety"));auto* Dark=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/PiedmontRide/Materials/M_Asphalt.M_Asphalt"));
 GuitarBody->SetMaterial(0,Wood);GuitarUpper->SetMaterial(0,Wood);GuitarNeck->SetMaterial(0,Wood);GuitarHead->SetMaterial(0,Wood);GuitarHole->SetMaterial(0,Dark);GuitarHoleBack->SetMaterial(0,Dark);SaxBody->SetMaterial(0,Gold);SaxBell->SetMaterial(0,Gold);
 FTransform T(GetActorRotation(),GetActorLocation()+FVector(0,0,88));auto* Person=GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),T,this,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding);
 if(Person){Person->bParkMusician=true;Person->MusicianKind=InstrumentKind;Person->CityAppearanceVariant=InstrumentKind+4;Person->CityOutfitVariant=InstrumentKind+4;Person->FinishSpawning(T);Performer=Person;}
Loop=NewObject<UBattleInstrumentLoop>(this);Loop->InstrumentKind=InstrumentKind;Music->SetSound(Loop);Music->SetVolumeMultiplier(Guitar?.22f:.16f);Music->Play();
}
// A player holds the neck up and to the left with the strumming hand over the
// sound hole. Building the instrument from those two hands keeps it on the
// body; the old fixed offsets left it hanging in front of the chest.
void ABattleMusician::PlaceGuitar(const FVector& LeftHand,const FVector& RightHand,bool bAnnounce){
 const FTransform Xf(GetActorTransform());
 const FVector L=Xf.InverseTransformPositionNoScale(LeftHand),R=Xf.InverseTransformPositionNoScale(RightHand);
 FVector A=(L-R).GetSafeNormal();
 if(A.IsNearlyZero()||A.Z<.2f)A=FVector(.05f,-.55f,.83f).GetSafeNormal();
 FVector N=FVector::CrossProduct(FVector::UpVector,A).GetSafeNormal();
 if(N.IsNearlyZero()||N.X<0.f)N=FVector::ForwardVector;
 const FQuat Root=Xf.GetRotation();
 auto Local=[&](const FMatrix& M){return (Root.Inverse()*FQuat(M)).GetNormalized().Rotator();};
 const FRotator Plate=Local(FRotationMatrix::MakeFromXZ(N,A)),Neck=Local(FRotationMatrix::MakeFromX(A)),Hole=Local(FRotationMatrix::MakeFromZ(N));
 // A thin soundboard with a waist reads as a guitar. Two fat spheres on the
 // same axis read as a pair of logs, which is what the old build looked like.
 GuitarBody->SetRelativeLocation(R);GuitarBody->SetRelativeRotation(Plate);GuitarBody->SetRelativeScale3D(FVector(.075f,.40f,.44f));
 GuitarUpper->SetRelativeLocation(R+A*16.f);GuitarUpper->SetRelativeRotation(Plate);GuitarUpper->SetRelativeScale3D(FVector(.07f,.30f,.30f));
 const FVector HolePos=R+A*13.f;
 GuitarHole->SetRelativeLocation(HolePos+N*4.6f);GuitarHole->SetRelativeRotation(Hole);GuitarHole->SetRelativeScale3D(FVector(.10f,.10f,.012f));
 GuitarHoleBack->SetRelativeLocation(HolePos-N*4.6f);GuitarHoleBack->SetRelativeRotation(Hole);GuitarHoleBack->SetRelativeScale3D(FVector(.10f,.10f,.012f));
 GuitarNeck->SetRelativeLocation(R+A*50.f);GuitarNeck->SetRelativeRotation(Neck);GuitarNeck->SetRelativeScale3D(FVector(.44f,.055f,.05f));
 GuitarHead->SetRelativeLocation(R+A*79.f);GuitarHead->SetRelativeRotation(Neck);GuitarHead->SetRelativeScale3D(FVector(.15f,.08f,.10f));
 if(bAnnounce)UE_LOG(LogTemp,Display,TEXT("BattleMusicianGuitar: fret_hand %.1f %.1f %.1f strum_hand %.1f %.1f %.1f"),L.X,L.Y,L.Z,R.X,R.Y,R.Z);
}
void ABattleMusician::Tick(float Dt){
 Super::Tick(Dt);
 const bool Performing=Performer&&!Performer->bDead&&Performer->PanicRemaining<=0&&Performer->StumbleRemaining<=0;
 // The hold settles over the first second of playing. Rebuild the instrument
 // from the hands every frame instead of freezing it wherever the first pose
 // happened to leave it, which is what left it floating beside the hip.
 if(InstrumentKind==0&&Performing&&Performer->Body&&Performer->MusicClock>.5f){
  PlaceGuitar(Performer->Body->GetSocketLocation(TEXT("hand_l")),Performer->Body->GetSocketLocation(TEXT("hand_r")),!bGuitarPlaced);
  bGuitarPlaced=true;
 }
 for(auto* Part:{GuitarBody.Get(),GuitarUpper.Get(),GuitarNeck.Get(),GuitarHead.Get(),GuitarHole.Get(),GuitarHoleBack.Get(),SaxBody.Get(),SaxBell.Get()})Part->SetVisibility(Performing&&((InstrumentKind==0)==(Part==GuitarBody||Part==GuitarUpper||Part==GuitarNeck||Part==GuitarHead||Part==GuitarHole||Part==GuitarHoleBack)));
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
 // The review camera and pose diagnostics can target either performer.
 int32 MusicianReviewIndex=0;{FString Value;if(FParse::Value(FCommandLine::Get(),TEXT("BattleMusicianReviewIndex="),Value))MusicianReviewIndex=FCString::Atoi(*Value);if(!Musicians.IsValidIndex(MusicianReviewIndex))MusicianReviewIndex=0;}
#if !UE_BUILD_SHIPPING
 if(!FParse::Param(FCommandLine::Get(),TEXT("BattleMusicianAudit")))return;AuditClock+=Dt;
 if(Musicians.Num()!=2){if(AuditClock>3){UE_LOG(LogTemp,Display,TEXT("BattleMusicianAudit: {\"passed\":false,\"musicians\":%d,\"reason\":\"location grounding failed\"}"),Musicians.Num());bAuditDone=true;if(auto* PC=UGameplayStatics::GetPlayerController(this,0))UKismetSystemLibrary::QuitGame(this,PC,EQuitPreference::Quit,false);}return;}
 FString ReviewDir;const bool Review=FParse::Value(FCommandLine::Get(),TEXT("BattleMusicianReviewDir="),ReviewDir);
 // Look at the instrument from the front. The old camera sat behind the
 // performer, so the review frame never actually showed the guitar.
 if(Review&&!bReviewSetup&&AuditClock>.25f){const FVector Focus=Musicians[MusicianReviewIndex]->GetActorLocation()+FVector(0,0,105),Fwd=FRotationMatrix(Musicians[MusicianReviewIndex]->GetActorRotation()).GetUnitAxis(EAxis::X),Side=FRotationMatrix(Musicians[MusicianReviewIndex]->GetActorRotation()).GetUnitAxis(EAxis::Y),Eye=Focus+Fwd*300.f+Side*165.f+FVector(0,0,35);if(auto* PC=UGameplayStatics::GetPlayerController(this,0))if(auto* Camera=GetWorld()->SpawnActor<ACameraActor>(Eye,(Focus-Eye).Rotation())){ReviewCamera=Camera;PC->SetViewTarget(Camera);}bReviewSetup=true;}
 if(!bBoneLogged&&AuditClock>.8f){bBoneLogged=true;if(auto* M=Musicians[MusicianReviewIndex].Get())if(M->Performer&&M->Performer->Body){const FTransform Xf(M->GetActorTransform());for(const TCHAR* Bone:{TEXT("pelvis"),TEXT("spine_02"),TEXT("spine_03"),TEXT("head"),TEXT("upperarm_l"),TEXT("lowerarm_l"),TEXT("hand_l"),TEXT("upperarm_r"),TEXT("lowerarm_r"),TEXT("hand_r")}){if(M->Performer->Body->GetBoneIndex(Bone)<0)continue;const FVector P=Xf.InverseTransformPositionNoScale(M->Performer->Body->GetSocketLocation(Bone));UE_LOG(LogTemp,Display,TEXT("BattleMusicianBone: %s %.1f %.1f %.1f"),Bone,P.X,P.Y,P.Z);}}}
 // Search the arm rotations that put the hands where a player's hands go, then
 // print them for the pose constants. Diagnostic only.
 if(!bSwept&&FParse::Param(FCommandLine::Get(),TEXT("BattleGuitarSweep"))&&AuditClock>1.6f){bSwept=true;if(auto* M=Musicians[MusicianReviewIndex].Get())if(M->Performer&&M->Performer->Body){auto P=M->Performer;const FTransform Xf(M->GetActorTransform());auto Search=[&](bool bLeft,const FVector& Target){FRotator BestArm(0),BestFore(0);float BestCost=1e9f;auto Evaluate=[&](const FRotator& Arm,const FRotator& Fore){if(bLeft){P->GuitarArmL=Arm;P->GuitarForeL=Fore;}else{P->GuitarArmR=Arm;P->GuitarForeR=Fore;}P->SamplePerformerPose(.01f);const FVector H=Xf.InverseTransformPositionNoScale(P->Body->GetSocketLocation(bLeft?TEXT("hand_l"):TEXT("hand_r")));return FVector::Dist(H,Target);};for(int32 AP=-80;AP<=-20;AP+=20)for(int32 AY=-45;AY<=45;AY+=30)for(int32 AR=-30;AR<=30;AR+=30)for(int32 EP=-80;EP<=-30;EP+=25)for(int32 ER=-20;ER<=20;ER+=40){const float C=Evaluate(FRotator(AP,AY,AR),FRotator(EP,0,ER));if(C<BestCost){BestCost=C;BestArm=FRotator(AP,AY,AR);BestFore=FRotator(EP,0,ER);}}for(int32 AP=BestArm.Pitch-10;AP<=BestArm.Pitch+10;AP+=5)for(int32 AY=BestArm.Yaw-10;AY<=BestArm.Yaw+10;AY+=5)for(int32 AR=BestArm.Roll-10;AR<=BestArm.Roll+10;AR+=10)for(int32 EP=BestFore.Pitch-15;EP<=BestFore.Pitch+15;EP+=15)for(int32 ER=BestFore.Roll-15;ER<=BestFore.Roll+15;ER+=15){const float C=Evaluate(FRotator(AP,AY,AR),FRotator(EP,0,ER));if(C<BestCost){BestCost=C;BestArm=FRotator(AP,AY,AR);BestFore=FRotator(EP,0,ER);}}Evaluate(BestArm,BestFore);UE_LOG(LogTemp,Display,TEXT("BattleGuitarSweep: %s arm %d %d %d fore %d %d err %.1f"),bLeft?TEXT("left"):TEXT("right"),FMath::RoundToInt(BestArm.Pitch),FMath::RoundToInt(BestArm.Yaw),FMath::RoundToInt(BestArm.Roll),FMath::RoundToInt(BestFore.Pitch),FMath::RoundToInt(BestFore.Roll),BestCost);};Search(true,FVector(24,-26,132));Search(false,FVector(20,4,104));}}
 if(!bAuditSampled&&AuditClock>.6f){for(auto M:Musicians)AuditHands.Add(M->Performer->Body->GetSocketLocation(TEXT("hand_r")));bAuditSampled=true;}
 if(bAuditSampled)for(int32 I=0;I<Musicians.Num()&&I<AuditHands.Num();I++)if(auto* M=Musicians[I].Get())if(M->Performer)AuditHandTravel=FMath::Max(AuditHandTravel,float(FVector::Dist(AuditHands[I],M->Performer->Body->GetSocketLocation(TEXT("hand_r")))));
 if(Review&&!ReviewDir.IsEmpty()&&AuditClock>5.2f&&AuditClock-Dt<=5.2f)FScreenshotRequest::RequestScreenshot(ReviewDir/TEXT("park-guitarist.png"),false,false);
 if(AuditClock<(Review?6.1f:1.5f))return;
 const float HandTravel=AuditHandTravel;bool Ready=true,Audio=true;for(int32 I=0;I<Musicians.Num();I++){auto* M=Musicians[I].Get();Ready&=M&&M->Performer&&M->Performer->bNativeCrowdRig&&M->Performer->MusicClock>1;Audio&=M&&M->Music->Sound==M->Loop;}
 TArray<uint8> GuitarSamples,SaxSamples;Musicians[MusicianReviewIndex]->Loop->OnGeneratePCMAudio(GuitarSamples,512);Musicians[1]->Loop->OnGeneratePCMAudio(SaxSamples,512);const bool Generated=GuitarSamples.ContainsByPredicate([](uint8 V){return V!=0;})&&SaxSamples.ContainsByPredicate([](uint8 V){return V!=0;});
 Musicians[MusicianReviewIndex]->Performer->HearGunfire(Musicians[MusicianReviewIndex]->GetActorLocation());Musicians[MusicianReviewIndex]->Tick(.01f);const bool Reacted=Musicians[MusicianReviewIndex]->Performer->PanicRemaining>0&&!Musicians[MusicianReviewIndex]->Music->IsPlaying()&&!Musicians[MusicianReviewIndex]->GuitarBody->IsVisible();
 const bool Pass=Ready&&Audio&&Generated&&HandTravel>5&&Reacted;UE_LOG(LogTemp,Display,TEXT("BattleMusicianAudit: {\"passed\":%s,\"musicians\":%d,\"hand_travel_cm\":%.2f,\"audio_generated\":%s,\"panic_response\":%s}"),Pass?TEXT("true"):TEXT("false"),Musicians.Num(),HandTravel,Generated?TEXT("true"):TEXT("false"),Reacted?TEXT("true"):TEXT("false"));bAuditDone=true;FTimerHandle Quit;GetWorldTimerManager().SetTimer(Quit,[this](){if(auto* PC=UGameplayStatics::GetPlayerController(this,0))UKismetSystemLibrary::QuitGame(this,PC,EQuitPreference::Quit,false);},Review?.8f:.05f,false);
#endif
}
