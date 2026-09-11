#include "BattleBike.h"
#include "BattleMacController.h"
#include "BattleQuest.h"
#include "BattleZombie.h"
#include "BattlePickup.h"
#include "BattleRideFX.h"
#include "Components/AudioComponent.h"
#include "PiedmontDarkZone.h"
#include "Engine/DirectionalLight.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"
#include "BattleShot.h"
#include "PiedmontPedestrian.h"
#include "Sound/SoundBase.h"
#include "BattleRider.h"
#include "PiedmontPathSpline.h"
#include "PiedmontTrafficDirector.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
ABattleBike::ABattleBike(const FObjectInitializer& Init):Super(Init.SetDefaultSubobjectClass<UBattleBikeMovement>(ACharacter::CharacterMovementComponentName)){
 Inventory.SetNum(4);Inventory[0].Owned=true;Inventory[0].Magazine=12;Inventory[0].Reserve=-1;
 AsphaltAudio=CreateDefaultSubobject<UAudioComponent>(TEXT("AsphaltTires"));GrassAudio=CreateDefaultSubobject<UAudioComponent>(TEXT("GrassTires"));MotorAudio=CreateDefaultSubobject<UAudioComponent>(TEXT("ElectricMotor"));
 for(auto* Audio:{AsphaltAudio.Get(),GrassAudio.Get(),MotorAudio.Get()}){Audio->SetupAttachment(GetCapsuleComponent());Audio->bAutoActivate=false;Audio->SetVolumeMultiplier(0);}
 PrimaryActorTick.bCanEverTick=true;bUseControllerRotationYaw=false;
 Capsule=GetCapsuleComponent();Capsule->InitCapsuleSize(32,96);Capsule->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
 GetMesh()->SetVisibility(false);GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Ride=Cast<UBattleBikeMovement>(GetCharacterMovement());Tags.Add(TEXT("RideBike"));
 Visual=CreateDefaultSubobject<USceneComponent>(TEXT("LeanAssembly"));Visual->SetupAttachment(Capsule);Visual->SetRelativeLocation(FVector(0,0,-96));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Paint(TEXT("/Game/BeltLineGlide/Materials/M_Bike.M_Bike"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Rubber(TEXT("/Game/BeltLineGlide/Materials/M_Rubber.M_Rubber"));
 auto Part=[&](FString Name,FVector Loc,FVector Scale,bool Round,UMaterialInterface* Mat){auto* M=CreateDefaultSubobject<UStaticMeshComponent>(*Name);M->SetupAttachment(Visual);M->SetStaticMesh(Round?Cylinder.Object:Cube.Object);M->SetRelativeLocation(Loc);M->SetRelativeScale3D(Scale);M->SetCollisionEnabled(ECollisionEnabled::NoCollision);M->SetMaterial(0,Mat);return M;};
 auto Tube=[&](FString Name,FVector A,FVector B,float Radius,UMaterialInterface* Mat){auto* M=Part(Name,(A+B)*.5,FVector(Radius/50,Radius/50,(B-A).Size()/100),true,Mat);M->SetRelativeRotation(FQuat::FindBetweenNormals(FVector::UpVector,(B-A).GetSafeNormal()));return M;};
 const FVector Back(-60,0,35),Front(60,0,35),Crank(-5,0,36),Seat(-23,0,92),Head(43,0,91);
 Tube(TEXT("SeatTube"),Crank,Seat,2.2,Paint.Object);Tube(TEXT("TopTube"),Seat,Head,2,Paint.Object);Tube(TEXT("DownTube"),Crank,Head,3,Paint.Object);
 for(int S:{-1,1}){Tube(FString::Printf(TEXT("Stay%d"),S),Back+FVector(0,S*7,0),Seat,1.4,Paint.Object);Tube(FString::Printf(TEXT("ChainStay%d"),S),Back+FVector(0,S*7,0),Crank,1.4,Paint.Object);Tube(FString::Printf(TEXT("Fork%d"),S),Head,Front+FVector(0,S*6,0),1.6,Rubber.Object);}
 Part(TEXT("Battery"),FVector(16,0,59),FVector(.14,.1,.42),false,Rubber.Object)->SetRelativeRotation(FRotator(35,0,0));
 Part(TEXT("Saddle"),FVector(-23,0,98),FVector(.29,.19,.055),false,Rubber.Object);
 Tube(TEXT("Stem"),Head,FVector(40,0,112),2,Rubber.Object);Tube(TEXT("Handlebar"),FVector(40,-30,112),FVector(40,30,112),1.5,Rubber.Object);
 FrontWheel=Part(TEXT("FrontWheel"),Front,FVector(.70,.70,.055),true,Rubber.Object);FrontWheel->SetRelativeRotation(FRotator(0,0,90));
 RearWheel=Part(TEXT("RearWheel"),Back,FVector(.70,.70,.055),true,Rubber.Object);RearWheel->SetRelativeRotation(FRotator(0,0,90));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> DetailedWheel(TEXT("/Game/PiedmontRide/Bike/SM_BikeWheel.SM_BikeWheel"));
 if(DetailedWheel.Succeeded()){FrontWheel->SetStaticMesh(DetailedWheel.Object);RearWheel->SetStaticMesh(DetailedWheel.Object);FrontWheel->SetRelativeScale3D(FVector(1));RearWheel->SetRelativeScale3D(FVector(1));}
 Tube(TEXT("HubMotor"),Back-FVector(0,6,0),Back+FVector(0,6,0),9,Rubber.Object);
 Headlight=CreateDefaultSubobject<USpotLightComponent>(TEXT("AutomaticHeadlight"));Headlight->SetupAttachment(Capsule);Headlight->SetRelativeLocation(FVector(58,0,18));Headlight->SetIntensity(8000);Headlight->SetAttenuationRadius(3000);Headlight->SetInnerConeAngle(16);Headlight->SetOuterConeAngle(28);Headlight->SetLightColor(FLinearColor(1,.93,.8));Headlight->SetVisibility(false);
 TailLight=CreateDefaultSubobject<UPointLightComponent>(TEXT("AutomaticRearLight"));TailLight->SetupAttachment(Capsule);TailLight->SetRelativeLocation(FVector(-65,0,-15));TailLight->SetIntensity(25);TailLight->SetAttenuationRadius(90);TailLight->SetLightColor(FLinearColor(1,.015,.01));TailLight->SetVisibility(false);
 Pistol=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RidingPistol"));Pistol->SetupAttachment(Visual);Pistol->SetCollisionEnabled(ECollisionEnabled::NoCollision);Pistol->SetVisibility(false);
 Rider=CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("RiggedRider"));Rider->SetupAttachment(Visual);Rider->SetRelativeRotation(FRotator(0,-90,0));Rider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 static ConstructorHelpers::FObjectFinder<USkeletalMesh> Human(TEXT("/Game/PiedmontRide/Rider/Casual.Casual"));
 Rider->SetSkinnedAssetAndUpdate(Human.Object);
 Arm=CreateDefaultSubobject<USpringArmComponent>(TEXT("ChaseArm"));Arm->SetupAttachment(Capsule);Arm->TargetArmLength=470;Arm->SetRelativeLocation(FVector(0,0,130));Arm->SetRelativeRotation(FRotator(-12,0,0));Arm->bEnableCameraLag=true;Arm->CameraLagSpeed=7;
 Chase=CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));Chase->SetupAttachment(Arm);Chase->FieldOfView=85;
 Handlebar=CreateDefaultSubobject<UCameraComponent>(TEXT("HandlebarCamera"));Handlebar->SetupAttachment(Visual);Handlebar->SetRelativeLocation(FVector(37,0,151));Handlebar->FieldOfView=95;Handlebar->SetAutoActivate(false);
}
void ABattleBike::BeginPlay(){
 Super::BeginPlay();
 if(auto* Gun=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/PiedmontRide/Bike/SM_Pistol.SM_Pistol"))){const FVector Size=Gun->GetBounds().BoxExtent*2;const float Scale=28/FMath::Max(Size.X,Size.Y);const FRotator Rot(0,Size.Y>Size.X?-90:0,0);Pistol->SetStaticMesh(Gun);Pistol->SetRelativeScale3D(FVector(Scale));Pistol->SetRelativeRotation(Rot);Pistol->SetRelativeLocation(FVector(45,28,130)-Rot.RotateVector(Gun->GetBounds().Origin)*Scale);}
 CheckpointTransform=GetActorTransform();Ride->LastSafeLocation=GetActorLocation();PreviousFeedbackLocation=GetActorLocation();
 RideEffects=GetWorld()->SpawnActor<ABattleRideFX>();
 AsphaltAudio->SetSound(LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_Asphalt.S_Asphalt")));GrassAudio->SetSound(LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_Grass.S_Grass")));MotorAudio->SetSound(LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_Motor.S_Motor")));
 for(auto* Audio:{AsphaltAudio.Get(),GrassAudio.Get(),MotorAudio.Get()})Audio->Play();
 for(TActorIterator<AActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("RideWater")))Capsule->IgnoreActorWhenMoving(*It,true);
 if(auto* Mesh=Cast<USkeletalMesh>(Rider->GetSkinnedAsset())){
  const auto& Ref=Mesh->GetRefSkeleton();for(int32 I=0;I<Ref.GetNum();++I){Parents.Add(Ref.GetParentIndex(I));BoneNames.Add(Ref.GetBoneName(I));FTransform T=Ref.GetRefBonePose()[I];if(Parents[I]>=0)T=T*ReferencePose[Parents[I]];ReferencePose.Add(T);}
 }
}
void ABattleBike::SetupPlayerInputComponent(UInputComponent* I){
 Super::SetupPlayerInputComponent(I);I->BindKey(EKeys::H,IE_Pressed,this,&ABattleBike::Horn);I->BindKey(EKeys::LeftShift,IE_Pressed,this,&ABattleBike::StartBoost);I->BindKey(EKeys::E,IE_Pressed,this,&ABattleBike::Interact);I->BindKey(EKeys::Up,IE_Pressed,this,&ABattleBike::GearUp);I->BindKey(EKeys::Down,IE_Pressed,this,&ABattleBike::GearDown);I->BindKey(EKeys::Tab,IE_Pressed,this,&ABattleBike::ToggleCamera);
}
void ABattleBike::ToggleCamera(){bFirstPerson=!bFirstPerson;Chase->SetActive(!bFirstPerson);Handlebar->SetActive(bFirstPerson);Rider->SetVisibility(!bFirstPerson);}
void ABattleBike::Tick(float Dt){
 Super::Tick(Dt);UpdateHealth(Dt);UpdateLights(Dt);UpdateRideFeedback(Dt);HornCooldown=FMath::Max(0.f,HornCooldown-Dt);ShotCooldown=FMath::Max(0.f,ShotCooldown-Dt);HitFeedback=FMath::Max(0.f,HitFeedback-Dt);GunHold=FMath::Max(0.f,GunHold-Dt);
 if(ReloadTimer>0){ReloadTimer=FMath::Max(0.f,ReloadTimer-Dt);if(ReloadTimer<=0)PistolAmmo=12;}
 Pistol->SetVisibility(!bParked&&GunHold>0);if(bParked)return;UpdateNearMisses();
 for(auto* View:{Chase.Get(),Handlebar.Get()}){View->PostProcessSettings.bOverride_MotionBlurAmount=true;View->PostProcessSettings.MotionBlurAmount=Ride->BoostRemaining>0?.4f:.1f;}
 Chase->SetFieldOfView(FMath::FInterpTo(Chase->FieldOfView,Ride->BoostRemaining>0?98.f:85.f,Dt,5));
 Handlebar->SetFieldOfView(FMath::FInterpTo(Handlebar->FieldOfView,Ride->BoostRemaining>0?108.f:95.f,Dt,5));
 if(auto* PC=Cast<APlayerController>(GetController())){
  if(PC->IsInputKeyDown(EKeys::LeftMouseButton))FirePistol();
  Ride->Pedal=PC->IsInputKeyDown(EKeys::W)?1:0;
  Ride->Steer=(PC->IsInputKeyDown(EKeys::D)||PC->IsInputKeyDown(EKeys::Right)?1.f:0.f)-(PC->IsInputKeyDown(EKeys::A)||PC->IsInputKeyDown(EKeys::Left)?1.f:0.f);
  Ride->Brake=PC->IsInputKeyDown(EKeys::SpaceBar)?1:0;
 }
 if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this)))if(Mode->StartCountdown>0||Mode->bRunEnded||RiderHealth<=0)Ride->Pedal=Ride->Steer=0;
 const float Fall=Ride->Recovery>0?FMath::Sin((2-Ride->Recovery)*PI/2):0;
 const float Lean=Ride->Steer*FMath::Min(24.f,Ride->Speed*.035f);
 Visual->SetRelativeRotation(FRotator(0,0,Lean+Fall*65));Rider->SetRelativeLocation(FVector(0,-50*Fall,15*Fall));
 WheelAngle+=Ride->Speed*Dt/35*180/PI;FrontWheel->SetRelativeRotation(FRotator(WheelAngle,Ride->Steer*20,90));RearWheel->SetRelativeRotation(FRotator(WheelAngle,0,90));PoseRider(Dt);
}
FVector ABattleBike::FindPathReturn() const{
 FVector Best=Ride->LastSafeLocation;double Distance=TNumericLimits<double>::Max();
 FCollisionQueryParams Q(SCENE_QUERY_STAT(ArcadePathReturn),false,this);
 for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It){
  const FVector Point=It->Centerline->FindLocationClosestToWorldLocation(GetActorLocation(),ESplineCoordinateSpace::World);const double D=FVector::DistSquared2D(Point,GetActorLocation());if(D>=Distance)continue;
  FHitResult Ground;if(!GetWorld()->LineTraceSingleByChannel(Ground,Point+FVector(0,0,150),Point-FVector(0,0,150),ECC_Visibility,Q)||!Ground.GetActor()||Ground.ImpactNormal.Z<.4f)continue;
  if(!Ground.GetActor()->ActorHasTag(TEXT("RidePath"))&&!Ground.GetActor()->ActorHasTag(TEXT("RideDirt"))&&!Ground.GetActor()->ActorHasTag(TEXT("RideBridge")))continue;
  const FVector Return=Ground.ImpactPoint+FVector(0,0,98);if(GetWorld()->OverlapBlockingTestByChannel(Return,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,96),Q))continue;
  Best=Return;Distance=D;
 }
 return Best;
}
void ABattleBike::ValidationKey(FName Key,bool Pressed){
#if WITH_EDITOR
 if(auto* PC=Cast<APlayerController>(GetController()))PC->InputKey(FInputKeyParams(FKey(Key),Pressed?IE_Pressed:IE_Released,Pressed?1.0:0.0));
#endif
}
ABattleParkMode::ABattleParkMode(){CourseLabel=TEXT("PIEDMONT PARK | DEVELOPMENT");PlayerControllerClass=ABattleMacController::StaticClass();}
void ABattleParkMode::InitGame(const FString& MapName,const FString& Options,FString& ErrorMessage){
 Super::InitGame(MapName,Options,ErrorMessage);
 FString Choice=UGameplayStatics::ParseOption(Options,TEXT("Difficulty"));
 if(Choice!=TEXT("Easy")&&Choice!=TEXT("Medium")&&Choice!=TEXT("Hard"))Choice=TEXT("Easy");
 DifficultyName=FName(*Choice);
 if(auto* Table=LoadObject<UDataTable>(nullptr,TEXT("/Game/BattleForTheA/Data/DT_Difficulty.DT_Difficulty"))){
  if(const auto* Row=Table->FindRow<FBattleDifficultyRow>(DifficultyName,TEXT("Battle start")))Difficulty=*Row;
 }else UE_LOG(LogTemp,Error,TEXT("Battle difficulty table is missing"));
 TimeRemaining=Difficulty.TimeLimitSeconds;StartCountdown=3;
 UE_LOG(LogTemp,Display,TEXT("BattleDifficulty: %s timer=%.0f crowd=%d radar=%.0f"),*Choice,TimeRemaining,Difficulty.Walkers+Difficulty.Joggers,Difficulty.RadarRange);
}
void ABattleParkMode::StartPlay(){
 Super::StartPlay();
 for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It){
  It->DesiredPopulation=Difficulty.Walkers+Difficulty.Joggers;
  It->JoggerShare=float(Difficulty.Joggers)/FMath::Max(1,It->DesiredPopulation);
 }
 Quest=GetWorld()->SpawnActor<ABattleQuest>();if(Quest)Quest->RadarRange=Difficulty.RadarRange;
 Enemies=GetWorld()->SpawnActor<ABattleEnemyDirector>();
 Pickups=GetWorld()->SpawnActor<ABattlePickupDirector>();
}
ABattleLabMode::ABattleLabMode(){DefaultPawnClass=ABattleBike::StaticClass();HUDClass=ABattleLabHUD::StaticClass();}
void ABattleLabMode::StartPlay(){AGameModeBase::StartPlay();if(TActorIterator<APiedmontPathSpline>(GetWorld()))if(auto* Director=GetWorld()->SpawnActor<APiedmontTrafficDirector>())Director->DesiredPopulation=50;}
void ABattleLabMode::Tick(float Dt){
 AGameModeBase::Tick(Dt);if(StartCountdown>0){StartCountdown=FMath::Max(0.f,StartCountdown-Dt);return;}if(!bRunEnded){TimeRemaining=FMath::Max(0.f,TimeRemaining-Dt);if(TimeRemaining<=0)bRunEnded=true;}
}
void ABattleLabHUD::DrawHUD(){
 Super::DrawHUD();if(!Canvas)return;
 if(auto* Park=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this))){
  if(Park->Quest)Park->Quest->DrawRadar(this,Canvas);
  if(auto* Viewer=GetOwningPawn()){
   const ABattleZombie* Speaking=nullptr;float Best=2500;
   for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)if(It->SubtitleRemaining>0){const float D=FVector::Dist2D(Viewer->GetActorLocation(),It->GetActorLocation());if(D<Best){Best=D;Speaking=*It;}}
   if(Speaking)DrawText(TEXT("Zombie: ")+Speaking->Subtitle,FColor(200,255,160),Canvas->SizeX*.25f,Canvas->SizeY-60,nullptr,1.2);
  }
  const int Seconds=FMath::CeilToInt(Park->TimeRemaining);
  DrawText(FString::Printf(TEXT("%02d:%02d  |  %s"),Seconds/60,Seconds%60,*Park->DifficultyName.ToString()),Seconds<60?FColor::Red:FColor::White,Canvas->SizeX-235,28,nullptr,1.7);
  if(Park->StartCountdown>0)DrawText(FString::FromInt(FMath::CeilToInt(Park->StartCountdown)),FColor::Yellow,Canvas->SizeX*.5f,Canvas->SizeY*.35f,nullptr,4);
 }
 auto* Bike=Cast<ABattleBike>(GetOwningPawn());
 auto* Foot=Cast<ABattleRider>(GetOwningPawn());auto* HealthBike=Bike?Bike:(Foot?Foot->ParkedBike.Get():nullptr);
 if(HealthBike){
  const float Y=Canvas->SizeY-90;
  if(HealthBike->PickupNoticeRemaining>0)DrawText(FString::Printf(TEXT("Coca-Cola +%.0f HP"),HealthBike->LastHealAmount),FColor::Green,35,Y-50,nullptr,1.1);
  DrawText(FString::Printf(TEXT("HEALTH %.0f"),HealthBike->RiderHealth),FColor::White,35,Y-22,nullptr,1.1);
  DrawRect(FLinearColor(.02,.02,.02,.9),35,Y,204,16);
  DrawRect(HealthBike->RiderHealth<25?FLinearColor(1,.1,.08):FLinearColor(.1,.85,.3),37,Y+2,FMath::Clamp(HealthBike->RiderHealth,0.f,100.f)*2,12);
  DrawText(TEXT("NITRO"),FColor::Cyan,35,Y+23,nullptr,.9);
  DrawRect(FLinearColor(.02,.02,.02,.9),96,Y+24,143,12);DrawRect(FLinearColor(.05,.8,1),98,Y+26,FMath::Clamp(HealthBike->Nitro,0.f,100.f)*1.39f,8);
  if(HealthBike->RespawnRemaining>0)DrawText(TEXT("Wipeout | checkpoint -10 seconds"),FColor::Yellow,Canvas->SizeX*.32f,Canvas->SizeY*.6f,nullptr,1.5);
 }
 if(!Bike){
  if(auto* Person=Cast<ABattleRider>(GetOwningPawn())){
   DrawRect(FLinearColor(.03,.015,.02,.85),20,20,760,125);
   DrawText(TEXT("BATTLE FOR THE A | FIRST PERSON"),FColor::White,35,30,nullptr,1.8);
   DrawText(FString::Printf(TEXT("%s %d / %s   HEALTH %.0f%s   F: U-lock"),BattleWeapons::Name(Person->CurrentWeapon),Person->Ammo,*Person->ReserveLabel(),Person->Health,Person->ReloadRemaining>0?TEXT("   RELOADING"):TEXT("")),FColor(255,190,80),35,65,nullptr,1.4);
   DrawText(FString::Printf(TEXT("1 Pistol | 2 Shotgun%s | 3 SMG%s | 4 Frisbee | F U-lock | E bike"),Person->ParkedBike&&Person->ParkedBike->Inventory[1].Owned?TEXT(""):TEXT(" (find crate)"),Person->ParkedBike&&Person->ParkedBike->Inventory[2].Owned?TEXT(""):TEXT(" (find crate)")),FColor::White,35,105,nullptr,1.1);
   const float X=Canvas->SizeX*.5f,Y=Canvas->SizeY*.5f;DrawLine(X-8,Y,X+8,Y,FColor::White,1.5);DrawLine(X,Y-8,X,Y+8,FColor::White,1.5);
   if(Person->HitFeedback>0){DrawLine(X-12,Y-12,X+12,Y+12,FColor::Orange,2);DrawLine(X+12,Y-12,X-12,Y+12,FColor::Orange,2);}
  }return;
 }
 DrawRect(FLinearColor(.03,.015,.02,.85),20,20,720,125);const auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));DrawText(TEXT("BATTLE FOR THE A | ")+(Mode?Mode->CourseLabel:TEXT("RIDE")),FColor::White,35,30,nullptr,1.8);
 DrawText(FString::Printf(TEXT("GEAR %d / 5   %.0f MPH   Wipeouts %d"),Bike->Ride->Gear,Bike->Ride->Speed*.0223694f,Bike->Ride->Wipeouts),FColor(255,190,80),35,65,nullptr,1.5);
 DrawText(TEXT("W pedal | Arrows or A/D steer | Up/Down gears | Space brake | Tab view"),FColor::White,35,105,nullptr,1.1);
 DrawText(FString::Printf(TEXT("NITRO %.0f%% | Shift boost | H horn | E on/off | Click pistol %d"),Bike->Nitro,Bike->PistolAmmo),FColor::Cyan,35,155,nullptr,1.2);

 const float CrossX=Canvas->SizeX*.5f,CrossY=Canvas->SizeY*.5f;
 DrawLine(CrossX-5,CrossY,CrossX+5,CrossY,FColor::White,1);DrawLine(CrossX,CrossY-5,CrossX,CrossY+5,FColor::White,1);
 if(Bike->HitFeedback>0){DrawLine(CrossX-12,CrossY-12,CrossX+12,CrossY+12,FColor::Orange,2);DrawLine(CrossX+12,CrossY-12,CrossX-12,CrossY+12,FColor::Orange,2);}
 if(Bike->Ride->Recovery>0)DrawText(FString::Printf(TEXT("%s — back in %.1f"),*Bike->Ride->RecoveryReason,Bike->Ride->Recovery),FColor::Yellow,Canvas->SizeX*.4,Canvas->SizeY*.5,nullptr,2);
}
void ABattleBike::PoseRider(float Dt){
 if(ReferencePose.IsEmpty())return;
 TArray<FTransform> Pose=ReferencePose;
 auto Index=[&](const TCHAR* Name){return BoneNames.IndexOfByKey(FName(Name));};
 auto Descendant=[&](int I,int Root){while(I>=0){if(I==Root)return true;I=Parents[I];}return false;};
 auto MoveBranch=[&](int Root,FVector Target,FQuat Rotation){if(Root<0)return;FVector Old=Pose[Root].GetLocation();for(int I=Root;I<Pose.Num();++I)if(Descendant(I,Root)){Pose[I].SetLocation(Target+Rotation.RotateVector(Pose[I].GetLocation()-Old));Pose[I].SetRotation(Rotation*Pose[I].GetRotation());}};
 const int Pelvis=Index(TEXT("Hips"));MoveBranch(Pelvis,FVector(0,-23,99),FQuat(FVector::ForwardVector,FMath::DegreesToRadians(-28.f-(Ride->Brake>0?8.f:0.f))));
 auto Limb=[&](const TCHAR* UpperName,const TCHAR* LowerName,const TCHAR* EndName,FVector Target,FVector Bend){
  int U=Index(UpperName),L=Index(LowerName),E=Index(EndName);if(U<0||L<0||E<0)return;
  FVector Origin=Pose[U].GetLocation();float A=FVector::Distance(Origin,Pose[L].GetLocation()),B=FVector::Distance(Pose[L].GetLocation(),Pose[E].GetLocation());
  FVector Direction=(Target-Origin).GetSafeNormal();float D=FMath::Clamp(FVector::Distance(Origin,Target),FMath::Abs(A-B)+.1f,A+B-.1f);Target=Origin+Direction*D;
  float Along=(A*A-B*B+D*D)/(2*D),Height=FMath::Sqrt(FMath::Max(0.f,A*A-Along*Along));FVector BendNormal=(Bend-Direction*FVector::DotProduct(Bend,Direction)).GetSafeNormal();FVector Joint=Origin+Direction*Along+BendNormal*Height;
  MoveBranch(U,Origin,FQuat::FindBetweenVectors(Pose[L].GetLocation()-Origin,Joint-Origin));
  MoveBranch(L,Joint,FQuat::FindBetweenVectors(Pose[E].GetLocation()-Pose[L].GetLocation(),Target-Joint));
 };
 const float C=Ride->Cadence;
 Limb(TEXT("UpperLeg_L"),TEXT("LowerLeg_L"),TEXT("Foot_L"),FVector(12,-5+FMath::Sin(C)*16,32+FMath::Cos(C)*16),FVector(0,1,0));
 Limb(TEXT("UpperLeg_R"),TEXT("LowerLeg_R"),TEXT("Foot_R"),FVector(-12,-5-FMath::Sin(C)*16,32-FMath::Cos(C)*16),FVector(0,1,0));
 Limb(TEXT("UpperArm_L"),TEXT("LowerArm_L"),TEXT("Hand_L"),FVector(28,29,114),FVector(1,0,-.4));
 Limb(TEXT("UpperArm_R"),TEXT("LowerArm_R"),TEXT("Hand_R"),GunHold>0?FVector(-28,45,130):FVector(-28,29,114),FVector(-1,0,-.4));
 for(int I=0;I<Pose.Num();++I)Rider->BoneSpaceTransforms[I]=Parents[I]>=0?Pose[I].GetRelativeTransform(Pose[Parents[I]]):Pose[I];
 Rider->MarkRefreshTransformDirty();
}

bool ABattleBike::FirePistol(){
 if(!GetController()||bParked||RiderHealth<=0||Ride->Recovery>0||ShotCooldown>0||ReloadTimer>0)return false;
 if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this)))if(Mode->StartCountdown>0||Mode->bRunEnded)return false;
 if(PistolAmmo<=0){ReloadTimer=1.5f;return false;}
 PistolAmmo--;ShotsFired++;ShotCooldown=.25f;GunHold=.8f;PistolSpread=FMath::Lerp(.15f,3.f,FMath::Clamp(Ride->Speed/1600.f,0.f,1.f));
 const auto Shot=FireBattlePistol(this,Pistol,PistolSpread);LastShotEnd=Shot.End;if(Shot.Damage>0)HitFeedback=.2f;if(Shot.EnemyKilled)AwardEnemyKill();return true;
}
bool ABattleBike::Boost(){
 if(!GetController()||bParked||Ride->Recovery>0||Ride->BoostRemaining>0||Nitro<100)return false;
 if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this)))if(Mode->StartCountdown>0||Mode->bRunEnded)return false;
 Nitro=0;Ride->BoostRemaining=3;
 if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_Boost.S_Boost")))UGameplayStatics::PlaySound2D(this,Sound);return true;
}
void ABattleBike::UpdateNearMisses(){
 if(Ride->Speed<500||Ride->Recovery>0){Passes.Empty();return;}
 const FVector2D Forward(GetActorForwardVector());const float Now=GetWorld()->GetTimeSeconds();
 for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It){
  if(It->bDead)continue;const FVector2D Delta(It->GetActorLocation()-GetActorLocation());
  if(Delta.SizeSquared()>FMath::Square(1000.f)){Passes.Remove(*It);continue;}
  if(const FVector2D* Previous=Passes.Find(*It))if(FVector2D::DotProduct(*Previous,Forward)>0&&FVector2D::DotProduct(Delta,Forward)<=0){
   const FVector2D Segment=Delta-*Previous;const float T=FMath::Clamp(-FVector2D::DotProduct(*Previous,Segment)/FMath::Max(Segment.SizeSquared(),.01),0.,1.);const float Clearance=(*Previous+Segment*T).Size();
   if(Clearance>90&&Clearance<200&&(!RewardTimes.Contains(*It)||Now-RewardTimes[*It]>20)){
    FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(BattleNearMiss),false,this);
    const bool Blocked=GetWorld()->LineTraceSingleByChannel(Hit,GetActorLocation(),It->GetActorLocation(),ECC_Visibility,Q)&&Hit.GetActor()!=*It;
    if(!Blocked){NearMisses++;Nitro=FMath::Min(100.f,Nitro+20);RewardTimes.Add(*It,Now);}
   }
  }
  Passes.Add(*It,Delta);
 }
 for(auto It=Passes.CreateIterator();It;++It)if(!It.Key().IsValid())It.RemoveCurrent();
 for(auto It=RewardTimes.CreateIterator();It;++It)if(!It.Key().IsValid())It.RemoveCurrent();
}

void ABattleBike::Horn(){
 if(!GetController()||bParked||HornCooldown>0)return;
 if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this)))if(Mode->bRunEnded)return;
 HornCooldown=.65f;HornCount++;
 for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->HearHorn(this);
 if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/PiedmontRide/Audio/S_Horn.S_Horn")))UGameplayStatics::PlaySoundAtLocation(this,Sound,GetActorLocation());
}
void ABattleBike::UpdateLights(float Dt){
 LightOffDelay=FMath::Max(0.f,LightOffDelay-Dt);LightCheck-=Dt;if(LightCheck>0)return;LightCheck=.2f;bool Dark=false;
 for(TActorIterator<APiedmontDarkZone> It(GetWorld());It;++It)if(It->Contains(GetActorLocation())){Dark=true;break;}
 if(!Dark){TActorIterator<ADirectionalLight> Sun(GetWorld());if(Sun)Dark=Sun->GetActorForwardVector().Z>0;}
 if(Dark){LightOffDelay=1.5f;bLightsOn=true;}else if(LightOffDelay<=0)bLightsOn=false;
 Headlight->SetVisibility(bLightsOn);TailLight->SetVisibility(bLightsOn);
}
