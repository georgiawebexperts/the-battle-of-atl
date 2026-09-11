#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleZombie.h"
#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
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
void ABattleMacController::TickFootAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(FootStage<0||GetWorld()->GetTimeSeconds()<5)return;
 auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* P=Cast<ABattleRider>(GetPawn());
 auto End=[&](bool Pass,const TCHAR* Why){FlushPressedKeys();if(FootCeiling.IsValid())FootCeiling->Destroy();FootStage=-1;UE_LOG(LogTemp,Display,TEXT("BattleFootAudit: {\"passed\":%s,\"reason\":\"%s\",\"walk_speed\":%.1f,\"run_speed\":%.1f,\"arm_swing_cm\":%.1f}"),Pass?TEXT("true"):TEXT("false"),Why,FootWalkSpeed,FootRunSpeed,FootArmMin<MAX_flt?FootArmMax-FootArmMin:0);ConsoleCommand(TEXT("quit"));};
#define CHECKFOOT(C,R) if(!(C)){End(false,TEXT(R));return;}
 auto Key=[&](FKey K,bool Down){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Capture=[&](const TCHAR* Name){if(FParse::Param(FCommandLine::Get(),TEXT("BattleFootReview"))){FString Dir;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Dir);FScreenshotRequest::RequestScreenshot(Dir/(FString(Name)+TEXT(".png")),false,false);}};
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
 if(FootStage==1&&FootClock>.3f){CHECKFOOT(P->Ammo==10&&P->ShotsFired==0&&!P->bAiming&&!P->bWeaponDrawn,"Click drew or fired a holstered gun");Capture(TEXT("idle"));Key(EKeys::LeftMouseButton,false);Key(EKeys::RightMouseButton,false);Key(EKeys::G,true);Advance(2);}
 else if(FootStage==2&&FootClock>.08f){Key(EKeys::G,false);CHECKFOOT(P->bWeaponDrawn&&P->DrawRemaining>0&&!P->Fire(),"Draw delay or G binding failed");Advance(3);}
 else if(FootStage==3&&FootClock>.5f){CHECKFOOT(P->Weapon->IsVisible()&&P->Fire()&&P->Ammo==9,"Drawn gun could not fire");Capture(TEXT("drawn"));Key(EKeys::G,true);Advance(4);}
 else if(FootStage==4){
  Key(EKeys::G,false);Key(EKeys::W,true);
  CHECKFOOT(!P->bWeaponDrawn&&!P->Fire(),"Holstering did not prevent firing");
  FootArmMin=FMath::Min(FootArmMin,P->FirstPersonArms->GetBoneLocationByName(TEXT("Hand_R"),EBoneSpaces::ComponentSpace).Y);FootArmMax=FMath::Max(FootArmMax,P->FirstPersonArms->GetBoneLocationByName(TEXT("Hand_R"),EBoneSpaces::ComponentSpace).Y);
  if(FootClock>.45f&&FootCaptures==0){Capture(TEXT("walk-a"));FootCaptures++;}if(FootClock>.85f&&FootCaptures==1){Capture(TEXT("walk-b"));FootCaptures++;}
  if(FootClock>1.3f){FootWalkSpeed=P->GetVelocity().Size2D();CHECKFOOT(FootWalkSpeed>450&&FVector::Dist2D(P->GetActorLocation(),FootStart)>300&&FootArmMax-FootArmMin>15,"Walking or empty-hand swing failed");Key(EKeys::LeftShift,true);Advance(5);}
 }else if(FootStage==5&&FootClock>1){FootRunSpeed=P->GetVelocity().Size2D();CHECKFOOT(FootRunSpeed>FootWalkSpeed*1.3f&&!P->bWeaponDrawn,"Sprint did not accelerate hands-free");Capture(TEXT("run"));FootJumpBase=P->GetActorLocation().Z;Key(EKeys::SpaceBar,true);Advance(6);}
 else if(FootStage==6&&FootClock>.3f){CHECKFOOT(P->GetCharacterMovement()->IsFalling()&&P->GetActorLocation().Z>FootJumpBase+30,"Space did not jump");Capture(TEXT("jump"));Key(EKeys::SpaceBar,false);Key(EKeys::W,false);Key(EKeys::LeftShift,false);Advance(7);}
 else if(FootStage==7&&FootClock>1.2f){CHECKFOOT(P->GetCharacterMovement()->IsMovingOnGround(),"Jump did not land");Key(EKeys::C,true);Advance(8);}
 else if(FootStage==8&&FootClock>.4f){Key(EKeys::C,false);CHECKFOOT(P->bIsCrouched&&P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()<60&&P->Camera->GetRelativeLocation().Z<50,"Crouch did not lower capsule and camera");Capture(TEXT("crouch"));Key(EKeys::LeftShift,true);Key(EKeys::W,true);Advance(9);}
 else if(FootStage==9&&FootClock>.6f){CHECKFOOT(P->GetVelocity().Size2D()<260,"Sprint bypassed crouch speed");Key(EKeys::W,false);Key(EKeys::LeftShift,false);P->GetCharacterMovement()->StopMovementImmediately();
  auto* Ceiling=GetWorld()->SpawnActor<AStaticMeshActor>(P->GetActorLocation()+FVector(0,0,80),FRotator::ZeroRotator);CHECKFOOT(Ceiling,"Ceiling fixture missing");Ceiling->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);Ceiling->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Ceiling->SetActorScale3D(FVector(4,4,.2));Ceiling->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));FootCeiling=Ceiling;FHitResult CeilingHit;FCollisionQueryParams Q(SCENE_QUERY_STAT(CrouchCeiling),false,P);CHECKFOOT(GetWorld()->LineTraceSingleByChannel(CeilingHit,P->GetActorLocation()+FVector(0,0,50),P->GetActorLocation()+FVector(0,0,120),ECC_Visibility,Q)&&CeilingHit.GetActor()==Ceiling,"Ceiling collision fixture invalid");Key(EKeys::C,true);Advance(10);
 }else if(FootStage==10&&FootClock>.3f){Key(EKeys::C,false);CHECKFOOT(P->bIsCrouched,"Stood through low ceiling");FootCeiling->Destroy();Advance(11);}
 else if(FootStage==11&&FootClock>.4f){CHECKFOOT(!P->bIsCrouched,"Did not stand after ceiling cleared");P->SetActorLocation(FootStart,false,nullptr,ETeleportType::TeleportPhysics);CHECKFOOT(P->MountBike(),"Remount failed");auto* B=Cast<ABattleBike>(GetPawn());CHECKFOOT(B&&B->PistolAmmo==9&&B->Dismount(),"Loadout or second dismount failed");P=Cast<ABattleRider>(GetPawn());CHECKFOOT(P&&!P->bWeaponDrawn&&P->Ammo==9,"Second dismount auto-drew or lost ammo");End(true,TEXT("Holstered dismount, G draw/holster, protected firing, walking arms, sprint, jump, crouch clearance and remount pass"));}
#undef CHECKFOOT
#endif
}
