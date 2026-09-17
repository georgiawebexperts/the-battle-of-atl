#include "BattleMacController.h"
#include "HAL/IConsoleManager.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleDetailedRider.h"
#include "BattleZombie.h"
#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"
#include "GameFramework/HUD.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
#include "DrawDebugHelpers.h"
#include "StaticMeshResources.h"
#if WITH_EDITOR
#include "MeshDescription.h"
#endif
void ABattleMacController::TickFootAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(FootStage<0||GetWorld()->GetTimeSeconds()<5)return;
 auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* P=Cast<ABattleRider>(GetPawn());
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleAimPitchAudit"))){
  static int Phase=0;static float Clock=0;static FVector Hands[3],SupportHands[3];static float MaxWeaponHandDistance=0;Clock+=Dt;
  auto Finish=[&](bool Passed){UE_LOG(LogTemp,Display,TEXT("BattleAimPitchAudit: {\"passed\":%s,\"vertical_hand_travel_cm\":%.2f}"),Passed&&MaxWeaponHandDistance<80?TEXT("true"):TEXT("false"),Hands[1].Z-Hands[2].Z);UE_LOG(LogTemp,Display,TEXT("BattleAimSupportTravel: %.2f"),SupportHands[1].Z-SupportHands[2].Z);UE_LOG(LogTemp,Display,TEXT("BattleAimWeaponDistance: %.2f"),MaxWeaponHandDistance);FootStage=-1;ConsoleCommand(TEXT("quit"));};
  if(Phase==0){
   if(auto* CVar=IConsoleManager::Get().FindConsoleVariable(TEXT("r.AsyncPipelineCompile")))UE_LOG(LogTemp,Display,TEXT("BattlePipelinePolicy: async=%d"),CVar->GetInt());
   auto* B=Cast<ABattleBike>(GetPawn());if(!B||!B->Dismount())return;P=Cast<ABattleRider>(GetPawn());
   if(M->Enemies)M->Enemies->bFreezeSpawns=true;
   for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
   for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
   for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)It->Destroy();
   P->SetActorLocation(P->GetActorLocation()+FVector(1200,0,80));
   TInlineComponentArray<UMeshComponent*> ReviewMeshes(P);for(auto* Mesh:ReviewMeshes)Mesh->SetOnlyOwnerSee(false);
   P->ToggleDrawWeapon();int Slot=0;FParse::Value(FCommandLine::Get(),TEXT("BattleAimWeapon="),Slot);if(Slot>0){B->GiveWeapon(Slot,60);P->SelectWeapon(Slot);}SetControlRotation(FRotator(0,0,0));Phase=1;Clock=-1.5f;return;
  }
  if(!P){Finish(false);return;}
  if(Clock>.8f&&Clock<1.2f){
   const float Pitch=Phase==1?0.f:Phase==2?35.f:-35.f;
   const float Error=FMath::Abs(FMath::FindDeltaAngleDegrees((P->CurrentWeapon>0?P->LongGun->GetComponentRotation().Pitch:P->Weapon->GetComponentRotation().Pitch),Pitch));
   if(Error>1.f){Finish(false);return;}
   const FVector Hand=P->Body->GetSocketLocation(TEXT("hand_r"));
   if(P->CurrentWeapon>0)MaxWeaponHandDistance=FMath::Max(MaxWeaponHandDistance,float(FVector::Dist(P->LongGun->GetComponentLocation(),Hand)));
   Hands[Phase-1]=Hand-P->GetActorLocation();
   if(FParse::Param(FCommandLine::Get(),TEXT("BattleGripMarkers"))){
    DrawDebugSphere(GetWorld(),Hand,1.5f,12,FColor::Green,false,2.f,0,1.f);
    const FVector Knuckle=P->Body->GetSocketLocation(TEXT("middle_01_r"));
    DrawDebugSphere(GetWorld(),Knuckle,1.5f,12,FColor::Cyan,false,2.f,0,1.f);
    const FVector GripArt=P->ShotgunMesh->GetComponentTransform().TransformPosition(FVector(-43.75f,0,-2.5f));

    if(auto* Render=P->ShotgunMesh->GetStaticMesh()->GetRenderData()){
     for(int32 LOD=0;LOD<Render->LODResources.Num();LOD++){
      auto& Buffer=Render->LODResources[LOD].VertexBuffers.PositionVertexBuffer;
      UE_LOG(LogTemp,Display,TEXT("GripRenderLOD: lod=%d vertices=%u cpu=%d"),LOD,Buffer.GetNumVertices(),Buffer.GetAllowCPUAccess());
      if(Buffer.GetAllowCPUAccess()){
       FVector Closest;double Distance=MAX_dbl;
       for(uint32 I=0;I<Buffer.GetNumVertices();I++){FVector V(Buffer.VertexPosition(I));double D=FVector::DistSquared(V,FVector(-43.75f,0,-2.5f));if(D<Distance){Distance=D;Closest=V;}}
       UE_LOG(LogTemp,Display,TEXT("GripRenderVertex: lod=%d nearest=%s distance=%.3f"),LOD,*Closest.ToString(),FMath::Sqrt(Distance));
      }
     }
    }
#if WITH_EDITOR
    if(auto* Description=P->ShotgunMesh->GetStaticMesh()->GetMeshDescription(0)){
     auto Positions=Description->GetVertexPositions();FVector Closest;double Distance=MAX_dbl;
     for(auto Vertex:Description->Vertices().GetElementIDs()){
      FVector Position(Positions[Vertex]);const double D=FVector::DistSquared(Position,FVector(-43.75f,0,-2.5f));if(D<Distance){Distance=D;Closest=Position;}
     }
     DrawDebugSphere(GetWorld(),P->ShotgunMesh->GetComponentTransform().TransformPosition(Closest),2.f,12,FColor::Red,false,2.f,0,1.f);
     UE_LOG(LogTemp,Display,TEXT("GripMeshVertex: nearest=%s distance=%.3f"),*Closest.ToString(),FMath::Sqrt(Distance));
    }
#endif

    DrawDebugSphere(GetWorld(),GripArt,1.5f,12,FColor::Magenta,false,2.f,0,1.f);
    UE_LOG(LogTemp,Display,TEXT("GripMarker: phase=%d wrist=%s knuckle=%s art=%s mesh=%s"),Phase,*Hand.ToString(),*Knuckle.ToString(),*GripArt.ToString(),*P->ShotgunMesh->GetComponentTransform().ToString());
   }

   SupportHands[Phase-1]=P->Body->GetSocketLocation(TEXT("hand_l"))-P->GetActorLocation();
   const FVector Focus=P->GetActorLocation()+FVector(0,0,40),Eye=Focus+(FParse::Param(FCommandLine::Get(),TEXT("BattleAimClose"))?FVector(100,FParse::Param(FCommandLine::Get(),TEXT("BattleAimRightSide"))?120:-120,35):FVector(190,-240,65));
   if(auto* Camera=GetWorld()->SpawnActor<ACameraActor>(Eye,(Focus-Eye).Rotation()))SetViewTarget(Camera);
   FString Dir;if(FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Dir))FScreenshotRequest::RequestScreenshot(Dir/FString::Printf(TEXT("aim-%d.png"),Phase),false,false);
   Clock=1.2f;return;
  }
  if(Clock>1.5f){
   if(Phase==3){Finish(FMath::Max(Hands[1].Z-Hands[2].Z,SupportHands[1].Z-SupportHands[2].Z)>25.f);return;}
   Phase++;SetControlRotation(FRotator(Phase==2?35:-35,0,0));Clock=0;
  }
  return;
 }
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleMouseLookAudit"))){
  static int Phase=0;static float Clock=0;static FRotator Initial,InitialBody;static FVector Start;
  static bool OrbitPassed=false,WalkPassed=false,AimPassed=false,KeyboardPassed=false;static FRotator KeyboardStart;Clock+=Dt;
  auto Key=[&](FKey K,bool Down){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
  auto MouseAxis=[&](FKey Axis,float Value){if(!P||!P->InputComponent)return false;bool Bound=false;for(const auto& Binding:P->InputComponent->AxisKeyBindings)Bound|=Binding.AxisKey==Axis;if(Bound){FRotator R=GetControlRotation();if(Axis==EKeys::MouseX)R.Yaw+=Value;else R.Pitch=FMath::ClampAngle(R.Pitch-Value*.85f,-80,80);SetControlRotation(R);}return Bound;};
  if(Phase==0){
   auto* B=Cast<ABattleBike>(GetPawn());if(!B||!B->Dismount())return;
   P=Cast<ABattleRider>(GetPawn());
   if(!P||!P->InputComponent){UE_LOG(LogTemp,Display,TEXT("BattleMouseLookAudit: {\"passed\":false,\"reason\":\"on-foot input component missing\"}"));Phase=5;ConsoleCommand(TEXT("quit"));return;}
   bool FireBound=false,AimBound=false;
   for(const auto& Binding:P->InputComponent->KeyBindings){FireBound|=Binding.Chord.Key==EKeys::LeftMouseButton;AimBound|=Binding.Chord.Key==EKeys::RightMouseButton;}
   if(!FireBound||!AimBound){UE_LOG(LogTemp,Display,TEXT("BattleMouseLookAudit: {\"passed\":false,\"reason\":\"fire or aim button binding missing\"}"));Phase=5;ConsoleCommand(TEXT("quit"));return;}
   Initial=GetControlRotation();InitialBody=GetPawn()->GetActorRotation();Start=GetPawn()->GetActorLocation();Phase=1;Clock=0;return;
  }
  if((Phase==1||Phase==3)&&Clock<.5f){
   if(!MouseAxis(EKeys::MouseX,240.f*Dt)||!MouseAxis(EKeys::MouseY,80.f*Dt)){UE_LOG(LogTemp,Display,TEXT("BattleMouseLookAudit: {\"passed\":false,\"reason\":\"mouse axis binding missing\"}"));Phase=5;ConsoleCommand(TEXT("quit"));}return;
  }
  if(Phase==1&&Clock>.8f){
   FVector Eye;FRotator View;GetPlayerViewPoint(Eye,View);
   UE_LOG(LogTemp,Display,TEXT("BattleMouseLookDiagnostic: control_yaw=%.2f body_yaw=%.2f view_yaw=%.2f initial_yaw=%.2f camera_distance=%.2f"),GetControlRotation().Yaw,P?P->GetActorRotation().Yaw:0,View.Yaw,Initial.Yaw,P?FVector::Dist(Eye,P->GetActorLocation()):0);
   OrbitPassed=P&&FMath::Abs(FMath::FindDeltaAngleDegrees(Initial.Yaw,GetControlRotation().Yaw))>10
    &&FMath::Abs(FMath::FindDeltaAngleDegrees(InitialBody.Yaw,P->GetActorRotation().Yaw))<2
    &&FMath::Abs(FMath::FindDeltaAngleDegrees(View.Yaw,GetControlRotation().Yaw))<2&&FVector::Dist(Eye,P->GetActorLocation())>180;
   Key(EKeys::W,true);Phase=2;Clock=0;
  }else if(Phase==2&&Clock>.8f){
   WalkPassed=P&&FVector::Dist2D(Start,P->GetActorLocation())>150&&FMath::Abs(FMath::FindDeltaAngleDegrees(P->GetActorRotation().Yaw,GetControlRotation().Yaw))<5;
   Key(EKeys::W,false);Key(EKeys::G,true);Key(EKeys::G,false);Key(EKeys::RightMouseButton,true);Phase=3;Clock=0;
  }else if(Phase==3&&Clock>.8f){
   FVector Eye;FRotator View;GetPlayerViewPoint(Eye,View);
   AimPassed=P&&P->bAiming&&P->bWeaponDrawn&&FMath::Abs(FMath::FindDeltaAngleDegrees(P->GetActorRotation().Yaw,GetControlRotation().Yaw))<2&&FVector::Dist(Eye,P->GetActorLocation())>100;
   Key(EKeys::RightMouseButton,false);KeyboardStart=GetControlRotation();Key(EKeys::X,true);Key(EKeys::T,true);Phase=4;Clock=0;
  }else if(Phase==4&&Clock>.45f){
   Key(EKeys::X,false);Key(EKeys::T,false);const FRotator End=GetControlRotation();
   KeyboardPassed=FMath::Abs(FMath::FindDeltaAngleDegrees(KeyboardStart.Yaw,End.Yaw))>20&&FMath::Abs(FMath::FindDeltaAngleDegrees(KeyboardStart.Pitch,End.Pitch))>12;
   UE_LOG(LogTemp,Display,TEXT("BattleMouseLookAudit: {\"passed\":%s,\"free_orbit\":%s,\"camera_relative_walk\":%s,\"shoulder_aim\":%s,\"keyboard_look\":%s}"),OrbitPassed&&WalkPassed&&AimPassed&&KeyboardPassed?TEXT("true"):TEXT("false"),OrbitPassed?TEXT("true"):TEXT("false"),WalkPassed?TEXT("true"):TEXT("false"),AimPassed?TEXT("true"):TEXT("false"),KeyboardPassed?TEXT("true"):TEXT("false"));
   Phase=5;ConsoleCommand(TEXT("quit"));
  }
  return;
 }
 auto End=[&](bool Pass,const TCHAR* Why){FlushPressedKeys();if(FootCeiling.IsValid())FootCeiling->Destroy();FootStage=-1;UE_LOG(LogTemp,Display,TEXT("BattleFootAudit: {\"passed\":%s,\"reason\":\"%s\",\"walk_speed\":%.1f,\"run_speed\":%.1f,\"arm_swing_cm\":%.1f}"),Pass?TEXT("true"):TEXT("false"),Why,FootWalkSpeed,FootRunSpeed,FootArmMin<MAX_flt?FootArmMax-FootArmMin:0);ConsoleCommand(TEXT("quit"));};
#define CHECKFOOT(C,R) if(!(C)){End(false,TEXT(R));return;}
 auto Key=[&](FKey K,bool Down){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Capture=[&](const TCHAR* Name){if(FParse::Param(FCommandLine::Get(),TEXT("BattleFootReview"))){FString Dir;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Dir);FScreenshotRequest::RequestScreenshot(Dir/(FString(Name)+TEXT(".png")),false,false);}};
 auto Magazine=[&]()->UPoseableMeshComponent*{if(!P)return nullptr;TInlineComponentArray<UPoseableMeshComponent*> Parts(P);for(auto* Part:Parts)if(Part->GetFName()==TEXT("DetailedM1911"))return Part;return nullptr;};
 auto Advance=[&](int N){FootStage=N;FootClock=0;};
 FootClock+=Dt;
 if(FootStage==0){
  CHECKFOOT(M&&M->StartCountdown==0,"Missing active game");if(M->Enemies)M->Enemies->bFreezeSpawns=true;
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)It->Destroy();
  auto* B=Cast<ABattleBike>(GetPawn());CHECKFOOT(B&&B->Dismount(),"Cannot dismount");P=Cast<ABattleRider>(GetPawn());CHECKFOOT(P&&!P->bWeaponDrawn&&!P->Weapon->IsVisible()&&!P->Fire(),"Dismount did not leave weapon holstered");
  FootStart=P->GetActorLocation();Key(EKeys::LeftMouseButton,true);Key(EKeys::RightMouseButton,true);Advance(1);return;
 }
 CHECKFOOT(P,"Lost on-foot pawn");
 if(FootStage==1&&FootClock>.3f){CHECKFOOT(P->Ammo==17&&P->ShotsFired==0&&!P->bAiming&&!P->bWeaponDrawn,"Click drew or fired a holstered gun");Capture(TEXT("idle"));Key(EKeys::LeftMouseButton,false);Key(EKeys::RightMouseButton,false);Key(EKeys::G,true);Advance(2);}
 else if(FootStage==2&&FootClock>.08f){Key(EKeys::G,false);CHECKFOOT(P->bWeaponDrawn&&P->DrawRemaining>0&&!P->Fire(),"Draw delay or G binding failed");Advance(3);}
 else if(FootStage==3&&FootClock>.5f){CHECKFOOT(P->Weapon->IsVisible()&&P->Ammo==17,"Drawn weapon missing or fired early");if(FParse::Param(FCommandLine::Get(),TEXT("BattleFootReview")))if(auto* HUD=GetHUD())HUD->bShowHUD=false;Capture(TEXT("drawn"));Key(EKeys::RightMouseButton,true);Advance(13);}
 else if(FootStage==13&&FootClock>.5f){CHECKFOOT(P->bAiming&&P->Ammo==17,"Aiming failed or consumed ammo");Capture(TEXT("aimed"));Key(EKeys::RightMouseButton,false);Advance(12);}
 else if(FootStage==12&&FootClock>.15f){CHECKFOOT(P->Fire()&&P->Ammo==16,"Drawn gun could not fire");CHECKFOOT(P->ParkedBike&&P->ParkedBike->GiveWeapon(0,1),"Reload round fixture failed");Key(EKeys::R,true);Advance(14);}
 else if(FootStage==14&&FootClock>.75f){Key(EKeys::R,false);CHECKFOOT(P->ReloadRemaining>0&&P->Ammo==16&&!P->Fire(),"Reload failed or allowed firing");if(BattleUseDetailedRider()){auto* Gun=Magazine();CHECKFOOT(Gun&&Gun->GetBoneLocationByName(TEXT("Mag"),EBoneSpaces::ComponentSpace).Z<-15,"Pistol magazine did not withdraw");}Capture(TEXT("reload"));Advance(15);}
 else if(FootStage==15&&FootClock>1.1f){CHECKFOOT(P->ReloadRemaining<=0&&P->Ammo==17&&P->ParkedBike->Inventory[0].Reserve==0,"Reload did not conserve rounds");if(BattleUseDetailedRider()){auto* Gun=Magazine();CHECKFOOT(Gun&&FMath::Abs(Gun->GetBoneLocationByName(TEXT("Mag"),EBoneSpaces::ComponentSpace).Z+1.6719f)<.1f,"Pistol magazine did not reinsert");}if(auto* HUD=GetHUD())HUD->bShowHUD=true;CHECKFOOT(P->Fire()&&P->Ammo==16,"Cannot fire after reload");if(FParse::Param(FCommandLine::Get(),TEXT("BattleReloadInterruptAudit")))Advance(16);else{Key(EKeys::G,true);Advance(4);}}
 else if(FootStage==16){CHECKFOOT(P->ParkedBike->GiveWeapon(0,2),"Interruption reserve fixture failed");Key(EKeys::R,true);Advance(17);}
 else if(FootStage==17&&FootClock>.55f){Key(EKeys::R,false);CHECKFOOT(P->ReloadRemaining>0,"Holster interruption never began reload");Key(EKeys::G,true);Advance(18);}
 else if(FootStage==18&&FootClock>.15f){Key(EKeys::G,false);auto* Gun=Magazine();CHECKFOOT(!P->bWeaponDrawn&&P->ReloadRemaining==0&&P->Ammo==16&&P->ParkedBike->Inventory[0].Reserve==2,"Holster consumed rounds or kept reload active");CHECKFOOT(Gun&&!Gun->IsVisible()&&FMath::Abs(Gun->GetBoneLocationByName(TEXT("Mag"),EBoneSpaces::ComponentSpace).Z+1.6719f)<.1f,"Holster left magazine withdrawn or pistol visible");Key(EKeys::G,true);Advance(19);}
 else if(FootStage==19&&FootClock>.5f){Key(EKeys::G,false);CHECKFOOT(P->ParkedBike->GiveWeapon(1,6),"Switch fixture failed");Key(EKeys::R,true);Advance(20);}
 else if(FootStage==20&&FootClock>.55f){Key(EKeys::R,false);CHECKFOOT(P->ReloadRemaining>0,"Switch interruption never began reload");Key(EKeys::Two,true);Advance(21);}
 else if(FootStage==21&&FootClock>.4f){Key(EKeys::Two,false);CHECKFOOT(P->CurrentWeapon==1&&P->ReloadRemaining==0&&P->ParkedBike->Inventory[0].Magazine==16&&P->ParkedBike->Inventory[0].Reserve==2,"Weapon switch changed pistol rounds or kept reload active");CHECKFOOT(Magazine()&&!Magazine()->IsVisible(),"Pistol visible with shotgun");Key(EKeys::One,true);Advance(22);}
 else if(FootStage==22&&FootClock>.5f){Key(EKeys::One,false);CHECKFOOT(P->CurrentWeapon==0&&P->Ammo==16,"Return to pistol lost rounds");Key(EKeys::R,true);Advance(23);}
 else if(FootStage==23&&FootClock>.55f){Key(EKeys::R,false);CHECKFOOT(P->ReloadRemaining>0&&P->MountBike(),"Remount interruption failed");auto* Bike=Cast<ABattleBike>(GetPawn());CHECKFOOT(Bike&&Bike->Dismount(),"Cannot dismount after interrupted reload");Advance(24);}
 else if(FootStage==24&&FootClock>.2f){CHECKFOOT(!P->bWeaponDrawn&&P->Ammo==16&&P->ReloadRemaining==0&&P->ParkedBike->Inventory[0].Reserve==2,"Remount/dismount duplicated rounds or resumed reload");CHECKFOOT(Magazine()&&!Magazine()->IsVisible()&&FMath::Abs(Magazine()->GetBoneLocationByName(TEXT("Mag"),EBoneSpaces::ComponentSpace).Z+1.6719f)<.1f,"New rider pistol state is not reset");UE_LOG(LogTemp,Display,TEXT("DetailedReloadInterruptions: holster/switch/remount passed"));Advance(4);}
 else if(FootStage==4){
  Key(EKeys::G,false);Key(EKeys::W,true);
  CHECKFOOT(!P->bWeaponDrawn&&!P->Fire(),"Holstering did not prevent firing");
  FootArmMin=FMath::Min(FootArmMin,P->FirstPersonArms->GetBoneLocationByName(BattleDetailedBone(TEXT("Hand_R"),BattleUseDetailedRider()),EBoneSpaces::ComponentSpace).Y);FootArmMax=FMath::Max(FootArmMax,P->FirstPersonArms->GetBoneLocationByName(BattleDetailedBone(TEXT("Hand_R"),BattleUseDetailedRider()),EBoneSpaces::ComponentSpace).Y);
  if(FootClock>.45f&&FootCaptures==0){Capture(TEXT("walk-a"));FootCaptures++;}if(FootClock>.85f&&FootCaptures==1){Capture(TEXT("walk-b"));FootCaptures++;}
  if(FootClock>1.3f){FootWalkSpeed=P->GetVelocity().Size2D();CHECKFOOT((FootWalkSpeed>350&&FootWalkSpeed<450&&FVector::Dist2D(P->GetActorLocation(),FootStart)>300)&&FootArmMax-FootArmMin>15,"Walking or empty-hand swing failed");Key(EKeys::LeftShift,true);Advance(5);}
 }else if(FootStage==5&&FootClock>1){FootRunSpeed=P->GetVelocity().Size2D();CHECKFOOT(FootRunSpeed>FootWalkSpeed*1.3f&&!P->bWeaponDrawn,"Sprint did not accelerate hands-free");Capture(TEXT("run"));FootJumpBase=P->GetActorLocation().Z;Key(EKeys::SpaceBar,true);Advance(6);}
 else if(FootStage==6&&FootClock>.3f){CHECKFOOT(P->GetCharacterMovement()->IsFalling()&&P->GetActorLocation().Z>FootJumpBase+30,"Space did not jump");if(BattleUseDetailedRider()){
 const FVector Hip=P->Body->GetBoneLocationByName(TEXT("pelvis"),EBoneSpaces::WorldSpace);
 UE_LOG(LogTemp,Display,TEXT("DetailedFootJump: hip_offset=%s"),*(Hip-P->GetActorLocation()).ToString());
 CHECKFOOT(FVector::Dist(Hip,P->GetActorLocation())<130,"Jump animation displaced body outside player capsule");
 }Capture(TEXT("jump"));Key(EKeys::SpaceBar,false);if(!FParse::Param(FCommandLine::Get(),TEXT("BattleLandingRunReview"))){Key(EKeys::W,false);Key(EKeys::LeftShift,false);}Advance(7);}
 else if(FootStage==7&&FootClock<=1.2f){if(P->GetCharacterMovement()->IsMovingOnGround()){if(FootCaptures==2){Capture(TEXT("land-contact"));FootCaptures++;}else if(FootCaptures==3&&FootClock>.8f){Capture(TEXT("land-settle"));FootCaptures++;}}}
 else if(FootStage==7&&FootClock>1.2f){CHECKFOOT(P->GetCharacterMovement()->IsMovingOnGround(),"Jump did not land");Key(EKeys::W,false);Key(EKeys::LeftShift,false);Key(EKeys::C,true);Advance(8);}
 else if(FootStage==8&&FootClock>.4f){Key(EKeys::C,false);CHECKFOOT(P->bIsCrouched&&P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()<60&&P->Camera->GetRelativeLocation().Z<50,"Crouch did not lower capsule and camera");Capture(TEXT("crouch"));Key(EKeys::LeftShift,true);Key(EKeys::W,true);Advance(9);}
 else if(FootStage==9&&FootClock>.6f){CHECKFOOT(P->GetVelocity().Size2D()<260,"Sprint bypassed crouch speed");Key(EKeys::W,false);Key(EKeys::LeftShift,false);P->GetCharacterMovement()->StopMovementImmediately();
  auto* Ceiling=GetWorld()->SpawnActor<AStaticMeshActor>(P->GetActorLocation()+FVector(0,0,80),FRotator::ZeroRotator);CHECKFOOT(Ceiling,"Ceiling fixture missing");Ceiling->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);Ceiling->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Ceiling->SetActorScale3D(FVector(4,4,.2));Ceiling->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));FootCeiling=Ceiling;FHitResult CeilingHit;FCollisionQueryParams Q(SCENE_QUERY_STAT(CrouchCeiling),false,P);CHECKFOOT(GetWorld()->LineTraceSingleByChannel(CeilingHit,P->GetActorLocation()+FVector(0,0,50),P->GetActorLocation()+FVector(0,0,120),ECC_Visibility,Q)&&CeilingHit.GetActor()==Ceiling,"Ceiling collision fixture invalid");Key(EKeys::C,true);Advance(10);
 }else if(FootStage==10&&FootClock>.3f){Key(EKeys::C,false);CHECKFOOT(P->bIsCrouched,"Stood through low ceiling");FootCeiling->Destroy();Advance(11);}
 else if(FootStage==11&&FootClock>.4f){CHECKFOOT(!P->bIsCrouched,"Did not stand after ceiling cleared");P->SetActorLocation(FootStart,false,nullptr,ETeleportType::TeleportPhysics);CHECKFOOT(P->MountBike(),"Remount failed");auto* B=Cast<ABattleBike>(GetPawn());CHECKFOOT(B&&B->PistolAmmo==16&&B->Dismount(),"Loadout or second dismount failed");P=Cast<ABattleRider>(GetPawn());CHECKFOOT(P&&!P->bWeaponDrawn&&P->Ammo==16,"Second dismount auto-drew or lost ammo");End(true,TEXT("Holstered dismount, G draw/holster, aim, protected firing/reload, walking arms, sprint, jump, crouch clearance and remount pass"));}
#undef CHECKFOOT
#endif
}
