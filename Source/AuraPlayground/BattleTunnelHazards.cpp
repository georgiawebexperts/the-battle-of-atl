#include "BattleTunnelHazards.h"
#include "BattleScooterProp.h"
#include "BattlePothole.h"
#include "BattleHomeData.h"
#include "BattleBike.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UnrealClient.h"

ABattleTunnelHazards::ABattleTunnelHazards(){
 PrimaryActorTick.bCanEverTick=false;
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("TunnelHazardRoot"));
 Tags.Add(TEXT("BattleTunnelHazards"));
#if !UE_BUILD_SHIPPING
 // The rail yard sits on top of this tunnel, so the generic spot review - which
 // traces down from 20 m above - photographs the deck instead of the bore.
 // This is the only way to stand inside the tunnel and look at it.
 bReview=FParse::Param(FCommandLine::Get(),TEXT("BattleTunnelReview"));
 if(bReview)PrimaryActorTick.bCanEverTick=true;
#endif
}

// The tunnel floor is baked geometry that has been rebuilt more than once, so
// nothing here is placed at an authored height: each station is traced down from
// inside the bore and only accepted when it lands near the portal-to-portal
// profile. A trace that catches the rail deck over the tunnel is 3 m above that
// profile and is rejected rather than turned into a pothole on the roof.
static bool TunnelFloorAt(UWorld* World,const AActor* Owner,const FVector& PortalA,const FVector& PortalB,float T,float Lateral,FVector& Out){
 const FVector Base=FMath::Lerp(PortalA,PortalB,T);
 const FVector FlatA(PortalA.X,PortalA.Y,0),FlatB(PortalB.X,PortalB.Y,0);
 const FVector Dir=((FlatB-FlatA).GetSafeNormal());
 const FVector Side=FVector::CrossProduct(FVector::UpVector,Dir).GetSafeNormal();
 const float ProfileZ=FMath::Lerp(PortalA.Z,PortalB.Z,T);
 const FVector XY=Base+Side*Lateral;
 for(const float Lift:{120.f,200.f,60.f}){
  FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(TunnelFloor),false,Owner);
  const FVector Start(XY.X,XY.Y,ProfileZ+Lift);
  if(World->LineTraceSingleByChannel(Hit,Start,Start-FVector(0,0,420),ECC_Visibility,Q)
   &&Hit.ImpactNormal.Z>=.6f&&FMath::Abs(Hit.ImpactPoint.Z-ProfileZ)<=200.f){Out=Hit.ImpactPoint;return true;}
 }
 return false;
}

void ABattleTunnelHazards::BeginPlay(){
 Super::BeginPlay();
 const FVector A=BattleHomeData::TunnelEntry,B=BattleHomeData::TunnelExit;
 BoreLengthCm=FVector::Dist2D(A,B);
 BoreAxis=(FVector(B.X,B.Y,0)-FVector(A.X,A.Y,0)).GetSafeNormal();
 CarveHoles();
 StrewnScooters();
 UE_LOG(LogTemp,Display,TEXT("BattleTunnelHazards: length_cm=%.0f stations=%d grounded=%d deep=%d shallow=%d scooters=%d first_deep=%s audit_approach=%s"),
  BoreLengthCm,StationsProbed,StationsGrounded,DeepHoles,ShallowHoles,Scooters,*FirstDeepHole.ToString(),*FirstDeepHoleApproach.ToString());
}

void ABattleTunnelHazards::CarveHoles(){
 UStaticMesh* Disc=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 UStaticMesh* Chunk=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));
 UMaterialInterface* BaseMat=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
 UMaterialInstanceDynamic* HoleMat=BaseMat?UMaterialInstanceDynamic::Create(BaseMat,this):nullptr;
 if(HoleMat)HoleMat->SetVectorParameterValue(TEXT("Color"),FLinearColor(.012f,.013f,.014f));
 UMaterialInstanceDynamic* RubbleMat=BaseMat?UMaterialInstanceDynamic::Create(BaseMat,this):nullptr;
 if(RubbleMat)RubbleMat->SetVectorParameterValue(TEXT("Color"),FLinearColor(.055f,.056f,.058f));

 const FVector A=BattleHomeData::TunnelEntry,B=BattleHomeData::TunnelExit;
 const FVector Side=FVector::CrossProduct(FVector::UpVector,BoreAxis).GetSafeNormal();
 constexpr int32 Stations=12;
 float BestMiddle=1000.f;
 for(int32 I=0;I<Stations;++I){
  const float T=.16f+.0625f*I;
  // Staggered across the lane: a rider holding one line meets every hole, a
  // rider weaving meets none. The offsets alternate shoulders and the radius
  // grows toward the middle of the bore, where speed is highest.
  const float Lateral=((I%2)?1.f:-1.f)*(95.f+25.f*(I%3));
  ++StationsProbed;
  FVector Ground;
  if(!TunnelFloorAt(GetWorld(),this,A,B,T,Lateral,Ground))continue;
  ++StationsGrounded;
  const bool bDeep=I%2==0;
  const float Radius=bDeep?(78.f+12.f*(I%3)):(52.f+8.f*(I%3));
  FActorSpawnParameters Params;Params.Owner=this;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  auto* Hole=GetWorld()->SpawnActor<ABattlePothole>(ABattlePothole::StaticClass(),FTransform(Ground),Params);
  if(!Hole)continue;
  Hole->Tags.Add(TEXT("TunnelPothole"));
  Hole->ContactRadius=Radius;
  Hole->bDeep=bDeep;
  // A pothole only takes the rider off the bike above this speed; below it the
  // crossing still scrubs 28% of the speed, which is what "treacherous" means.
  Hole->CrashSpeed=bDeep?520.f:2000.f;
  // The pothole's own component is its root, and the traversal test reads the
  // actor location, so nothing here may move or scale the root. The crater
  // hangs off an unscaled child instead.
  auto* Crater=NewObject<USceneComponent>(Hole,TEXT("TunnelCrater"));
  Crater->SetupAttachment(Hole->GetRootComponent());
  Crater->SetRelativeLocation(FVector(0,0,1.f));
  Crater->RegisterComponent();
  auto* Face=NewObject<UStaticMeshComponent>(Hole,TEXT("TunnelCraterFace"));
  Face->SetupAttachment(Crater);
  Face->SetStaticMesh(Disc);
  Face->SetRelativeScale3D(FVector(Radius/40.f,Radius/40.f,.06f));
  Face->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  Face->SetCanEverAffectNavigation(false);
  Face->RegisterComponent();
  if(HoleMat)Face->SetMaterial(0,HoleMat);
  // Rubble around the rim so the crater reads as damage rather than a decal.
  for(int32 K=0;K<6;++K){
   const float Angle=K*1.047f+(I%3)*.4f;
   auto* Rubble=NewObject<UStaticMeshComponent>(Hole,*FString::Printf(TEXT("TunnelRubble%d"),K));
   Rubble->SetupAttachment(Crater);
   Rubble->SetStaticMesh(Chunk);
   Rubble->SetCollisionEnabled(ECollisionEnabled::NoCollision);
   Rubble->SetCanEverAffectNavigation(false);
   Rubble->RegisterComponent();
   Rubble->SetRelativeLocation(FVector(FMath::Cos(Angle)*(Radius*.92f),FMath::Sin(Angle)*(Radius*.92f),2.f));
   Rubble->SetRelativeScale3D(FVector(.07f+.05f*((I+K)%3),.05f,.03f));
   Rubble->SetRelativeRotation(FRotator((K*23)%60,(K*57)%360,(K*41)%80));
   if(RubbleMat)Rubble->SetMaterial(0,RubbleMat);
   CraterTrim.Add(Rubble);
  }
  Holes.Add(Hole);
  if(bDeep){
   ++DeepHoles;
   const float Middleness=FMath::Abs(T-.5f);
   if(Middleness<BestMiddle){
    BestMiddle=Middleness;
    FirstDeepHole=Ground;
    // Fourteen metres of run-up: the audit starts inside the bore and reaches
    // the hole at riding speed rather than at walking pace.
    FirstDeepHoleApproach=Ground-BoreAxis*1400.f;
   }
  }else ++ShallowHoles;
 }
}

void ABattleTunnelHazards::StrewnScooters(){
 const FVector A=BattleHomeData::TunnelEntry,B=BattleHomeData::TunnelExit;
 const FVector Side=FVector::CrossProduct(FVector::UpVector,BoreAxis).GetSafeNormal();
 const float Yaw=BoreAxis.Rotation().Yaw;
 for(int32 I=0;I<4;++I){
  const float T=.22f+.21f*I;
  const float Lateral=(I%2?-1.f:1.f)*(205.f+20.f*(I%2));
  FVector Ground;
  if(!TunnelFloorAt(GetWorld(),this,A,B,T,Lateral,Ground))continue;
  // Laid over on the shoulder, not parked: yawed across the bore and rolled onto
  // its side. The tilt belongs in Roll - the prop's local X runs along the deck,
  // so rolling about it lays the deck on its side with the wheels flat on the
  // pavement. The original pose put 80-96 in Pitch instead, which stood the
  // scooter on its nose with the deck vertical and the stem out sideways: a
  // plank with a bar through it, floating, which is not a scooter from any
  // angle. The parts carry no collision so a heap of them changes the look of
  // the tunnel without ever becoming a wall the rider cannot pass.
  const FRotator Down(2.f+(I%3)*3.f,Yaw+(I%2?34.f:-26.f),92.f+(I%2?-10.f:8.f));
  if(BuildBattleScooter(this,GetRootComponent(),Ground+FVector(0,0,12.f),Down,I,false)>0){
   if(!bHasScooterSpot){FirstScooterSpot=Ground;bHasScooterSpot=true;}
   ++Scooters;
  }
 }
 // And one parked upright against the wall, which is the one a rider reads as
 // "there is a scooter here" rather than as debris.
 FVector Lean;
 if(TunnelFloorAt(GetWorld(),this,A,B,.55f,215.f,Lean))
  if(BuildBattleScooter(this,GetRootComponent(),Lean+FVector(0,0,18.f),FRotator(4.f,Yaw+96.f,-12.f),1,false)>0)++Scooters;
}

void ABattleTunnelHazards::Tick(float Dt){
 Super::Tick(Dt);
#if !UE_BUILD_SHIPPING
 if(!bReview)return;
 ReviewClock+=Dt;
 auto* PC=UGameplayStatics::GetPlayerController(this,0);
 auto* Bike=PC?Cast<ABattleBike>(PC->GetPawn()):nullptr;
 if(!PC||!Bike||ReviewClock<1.f)return;
 if(!bReviewPlaced){
  const FRotator Facing(0,BoreAxis.Rotation().Yaw,0);
  Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->Ride->Recovery=0;
  Bike->SetActorLocationAndRotation(FirstDeepHoleApproach+FVector(0,0,98),Facing,false,nullptr,ETeleportType::TeleportPhysics);
  Bike->Ride->SetMovementMode(MOVE_Walking);Bike->Ride->bForceNextFloorCheck=true;
  bReviewPlaced=true;ReviewClock=0;
  return;
 }
 FString Dir;FParse::Value(FCommandLine::Get(),TEXT("BattleTunnelReviewDir="),Dir);
 if(Dir.IsEmpty())return;
 auto Capture=[&](const TCHAR* Name){FScreenshotRequest::RequestScreenshot(Dir/FString(Name),false,false);};
 if(ReviewStage==0&&ReviewClock>1.2f){
  Capture(TEXT("tunnel-holes.png"));++ReviewStage;ReviewClock=0;return;
 }
 if(ReviewStage==1&&ReviewClock>.3f&&bHasScooterSpot){
  // Shoot the scooters from the middle of the bore. A rider's own view is no
  // use here: the scooters lie on the shoulder, and standing on the shoulder
  // puts the camera inside a column.
  const FVector A=BattleHomeData::TunnelEntry,B=BattleHomeData::TunnelExit;
  const FVector FlatA(A.X,A.Y,0),FlatB(B.X,B.Y,0),Flat(FirstScooterSpot.X,FirstScooterSpot.Y,0);
  const FVector Axis=(FlatB-FlatA).GetSafeNormal();
  const float T=FMath::Clamp(FVector::DotProduct(Flat-FlatA,Axis)/FMath::Max(1.f,FVector::Dist2D(FlatA,FlatB)),0.f,1.f);
  const FVector Centre=FMath::Lerp(A,B,T);
  const FVector Eye=Centre-Axis*430.f+FVector(0,0,175.f);
  const FVector Look=FirstScooterSpot+FVector(0,0,45.f);
  const FRotator Aim=(Look-Eye).Rotation();
  if(!ReviewCamera)ReviewCamera=GetWorld()->SpawnActor<ACameraActor>(Eye,Aim);
  if(ReviewCamera){ReviewCamera->SetActorLocationAndRotation(Eye,Aim);PC->SetViewTarget(ReviewCamera);}
  Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->Ride->bForceNextFloorCheck=true;
  ++ReviewStage;ReviewClock=0;return;
 }
 if(ReviewStage==2&&ReviewClock>1.2f){
  Capture(TEXT("tunnel-scooters.png"));++ReviewStage;ReviewClock=0;return;
 }
 if(ReviewStage==3&&ReviewClock>1.f)UKismetSystemLibrary::QuitGame(this,PC,EQuitPreference::Quit,false);
#endif
}

void TickBattleTunnelHazardAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;int32 Phase=0,Wipeouts=0;float Clock=0;bool Done=false;};
 static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}
 if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;
 S.Clock+=Dt;
 auto Key=[&](bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::W,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Finish=[&](bool Pass,const TCHAR* Reason,const ABattleTunnelHazards* Hazards,int32 Wipeouts){
  Key(false);S.Done=true;
  UE_LOG(LogTemp,Display,TEXT("TunnelHazardAudit: {\"passed\":%s,\"reason\":\"%s\",\"phase\":%d,\"bore_cm\":%.0f,\"grounded_stations\":%d,\"deep_holes\":%d,\"shallow_holes\":%d,\"scooters\":%d,\"wipeouts\":%d}"),
   Pass?TEXT("true"):TEXT("false"),Reason,S.Phase,Hazards?Hazards->BoreLengthCm:0.f,Hazards?Hazards->StationsGrounded:0,Hazards?Hazards->DeepHoles:0,Hazards?Hazards->ShallowHoles:0,Hazards?Hazards->Scooters:0,Wipeouts);
  PC->ConsoleCommand(TEXT("quit"));
 };
 auto* Bike=Cast<ABattleBike>(PC->GetPawn());
 TActorIterator<ABattleTunnelHazards> HazardsIt(PC->GetWorld());
 ABattleTunnelHazards* Hazards=HazardsIt?*HazardsIt:nullptr;
 if(!Hazards){Finish(false,TEXT("No tunnel hazards actor in the world"),Hazards,0);return;}
 if(!Bike||!Bike->Ride){Finish(false,TEXT("Missing ridden bike"),Hazards,0);return;}
 if(S.Phase==0){
  // Two things have to be true before the ride is worth judging: the hazards
  // were placed, and they were placed in the tunnel rather than on the deck
  // above it. The second is re-checked here instead of trusted.
  int32 InBore=0,Deep=0;
  const FVector A=BattleHomeData::TunnelEntry,B=BattleHomeData::TunnelExit;
  for(TActorIterator<ABattlePothole> It(PC->GetWorld());It;++It){
   if(!It->ActorHasTag(TEXT("TunnelPothole")))continue;
   const FVector P=It->GetActorLocation();
   const FVector FlatA(A.X,A.Y,0),FlatB(B.X,B.Y,0),Flat(P.X,P.Y,0);
   const float Along=FMath::Clamp(FVector::DotProduct(Flat-FlatA,(FlatB-FlatA).GetSafeNormal())/FVector::Dist2D(FlatA,FlatB),0.f,1.f);
   const float ProfileZ=FMath::Lerp(A.Z,B.Z,Along);
   if(FMath::Abs(P.Z-ProfileZ)<=200.f&&FVector::Dist2D(P,FMath::Lerp(FlatA,FlatB,Along))<=450.f)++InBore;
   if(It->bDeep)++Deep;
  }
  if(Deep<3){Finish(false,TEXT("Fewer than three deep holes were placed in the tunnel"),Hazards,0);return;}
  if(InBore<Deep){Finish(false,TEXT("A deep hole is outside the tunnel profile"),Hazards,0);return;}
  if(Hazards->Scooters<3){Finish(false,TEXT("Fewer than three scooters in the tunnel"),Hazards,0);return;}
  FVector Start=Hazards->FirstDeepHoleApproach,Rest=Start;
  FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(TunnelHazardStart),true,Bike);
  if(PC->GetWorld()->LineTraceSingleByChannel(Hit,Start+FVector(0,0,250),Start-FVector(0,0,420),ECC_Visibility,Q))Rest=Hit.ImpactPoint;
  const FRotator Facing(0,Hazards->BoreAxis.Rotation().Yaw,0);
  Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->Ride->Recovery=0;Bike->Ride->Gear=5;
  Bike->SetActorLocationAndRotation(Rest+FVector(0,0,98),Facing,false,nullptr,ETeleportType::TeleportPhysics);
  Bike->Ride->SetMovementMode(MOVE_Walking);Bike->Ride->bForceNextFloorCheck=true;
  S.Wipeouts=Bike->Ride->Wipeouts;S.Phase=1;S.Clock=0;
  return;
 }
 Key(true);
 if(Bike->Ride->Wipeouts>S.Wipeouts){Finish(true,TEXT("The deep tunnel hole threw the rider off the bike"),Hazards,Bike->Ride->Wipeouts-S.Wipeouts);return;}
 if(FVector::Dist2D(Bike->GetActorLocation(),Hazards->FirstDeepHole)>2600.f){Finish(false,TEXT("Rode past the deep hole without a wipeout"),Hazards,0);return;}
 if(S.Clock>20){Finish(false,TEXT("Traversal timed out"),Hazards,0);return;}
#endif
}
