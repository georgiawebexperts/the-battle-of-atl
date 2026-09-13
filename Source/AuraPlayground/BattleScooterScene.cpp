#include "BattleScooterScene.h"
#include "PiedmontPedestrian.h"
#include "PiedmontBike.h"
#include "Animation/AnimSequence.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
ABattleScooterScene::ABattleScooterScene(){RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=.1f;}
bool ABattleScooterScene::IsOffscreen() const{
 auto* PC=GetWorld()->GetFirstPlayerController();if(!PC||!PC->GetPawn()||!PC->PlayerCameraManager)return false;
 if(FVector::Dist2D(PC->GetPawn()->GetActorLocation(),GetActorLocation())<3500)return false;
 const FVector ToScene=GetActorLocation()-PC->PlayerCameraManager->GetCameraLocation();
 // Entire conservative 5m scene sphere must be behind the camera, not just its origin.
 return FVector::DotProduct(ToScene,PC->PlayerCameraManager->GetCameraRotation().Vector()) < -500;
}
void ABattleScooterScene::AbortScene(){for(auto P:Participants)if(IsValid(P))P->Destroy();Participants.Empty();Destroy();}
bool ABattleScooterScene::SpawnScene(){
 FCollisionQueryParams Q(SCENE_QUERY_STAT(ScooterSite),false,this);
 const FVector Offsets[]={FVector(0,0,0),FVector(-40,-100,0),FVector(145,90,0)};
 FVector Places[3];
 for(int I=0;I<3;I++){
  const FVector At=GetActorLocation()+Offsets[I];FHitResult Hit;
  if(!GetWorld()->LineTraceSingleByChannel(Hit,At+FVector(0,0,300),At-FVector(0,0,300),ECC_WorldStatic,Q)||Hit.ImpactNormal.Z<.8f)return false;
  Places[I]=Hit.ImpactPoint+FVector(0,0,90);
  if(GetWorld()->OverlapBlockingTestByChannel(Places[I],FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,86),Q))return false;
 }
 for(int I=0;I<3;I++){
  FTransform T(FRotator(0,I==2?240:90,0),Places[I]);
  auto* P=GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),T,this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  if(!P)return false;P->CityAppearanceVariant=I==2?1:0;P->SetActorHiddenInGame(true);UGameplayStatics::FinishSpawningActor(P,T);P->PauseRemaining=3600;Participants.Add(P);
  const FLinearColor Color=I==0?FLinearColor(.25,.45,.8):I==1?FLinearColor(.9,.28,.08):FLinearColor(.25,.7,.4);
  TArray<USkeletalMeshComponent*> Parts;P->GetComponents(Parts);
  for(auto* Part:Parts)if(Part->GetName()==TEXT("CityOutfit0"))for(int Slot=0;Slot<Part->GetNumMaterials();Slot++)if(auto* M=Part->CreateDynamicMaterialInstance(Slot))
   for(const TCHAR* Key:{TEXT("A_CrowdColor_main"),TEXT("B_CrowdColor_main"),TEXT("C_Color_Value"),TEXT("A_CrowdColor_var"),TEXT("B_CrowdColor_Var")})M->SetVectorParameterValue(Key,Color);
 }
 const FTransform Frame(FRotator(0,25,82),GetActorLocation()+FVector(-110,100,20.440836));
 auto Part=[&](const TCHAR* Name,FVector Local,FVector Size,bool Cylinder,FRotator Rotation,const TCHAR* Material){
  auto* C=NewObject<UStaticMeshComponent>(this,Name);AddInstanceComponent(C);C->SetupAttachment(RootComponent);C->SetMobility(EComponentMobility::Movable);
  C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,Cylinder?TEXT("/Engine/BasicShapes/Cylinder"):TEXT("/Engine/BasicShapes/Cube")));
  C->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,Material));C->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);C->SetCollisionProfileName(TEXT("BlockAll"));C->SetCanEverAffectNavigation(false);C->RegisterComponent();C->SetWorldTransform(FTransform(Rotation,Local,Size/100)*Frame);
 };
 const TCHAR* Metal=TEXT("/Game/BattleForTheA/Environment/KrogIncident/M_ScooterMetal"),*Trim=TEXT("/Game/BattleForTheA/Environment/KrogIncident/M_ScooterTrim"),*Rubber=TEXT("/Game/BeltLineGlide/Materials/M_Rubber");
 Part(TEXT("Deck"),FVector::ZeroVector,FVector(110,20,7),false,FRotator::ZeroRotator,Trim);Part(TEXT("GripTape"),FVector(0,0,4),FVector(92,16,1),false,FRotator::ZeroRotator,Rubber);
 Part(TEXT("Stem"),FVector(48,0,50),FVector(5,5,100),true,FRotator::ZeroRotator,Metal);Part(TEXT("StemBand"),FVector(48,0,82),FVector(5.5,5.5,8),true,FRotator::ZeroRotator,Trim);
 Part(TEXT("Handle"),FVector(48,0,100),FVector(4,4,48),true,FRotator(0,0,90),Metal);
 for(int Side:{-1,1}){Part(Side<0?TEXT("LeftGrip"):TEXT("RightGrip"),FVector(48,Side*18,100),FVector(5,5,12),true,FRotator(0,0,90),Rubber);
  Part(Side<0?TEXT("RearWheel"):TEXT("FrontWheel"),FVector(Side*53,0,-6),FVector(24,24,6),true,FRotator(0,0,90),Rubber);
  Part(Side<0?TEXT("RearHub"):TEXT("FrontHub"),FVector(Side*53,0,-6),FVector(8,8,7),true,FRotator(0,0,90),Metal);}
 return true;
}
void ABattleScooterScene::Tick(float Dt){
 Super::Tick(Dt);if(GetWorld()->GetTimeSeconds()<3)return;
 auto* Mode=Cast<APiedmontRideMode>(UGameplayStatics::GetGameMode(this));if(!Mode||Mode->bRunEnded)return;
 if(!bChoiceMade){bChoiceMade=true;bSelected=FMath::FRand()<FMath::Clamp(AppearanceChance,0.f,1.f);if(!bSelected){SetActorTickEnabled(false);return;}}
 if(!bSpawned){if(!IsOffscreen())return;SetActorHiddenInGame(true);if(!SpawnScene()){AbortScene();return;}bSpawned=true;return;}
 if(!bSceneReady){SetupAge+=Dt;if(SetupAge<1||!IsOffscreen())return;
  if(Participants.Num()!=3){AbortScene();return;}for(auto P:Participants)if(!IsValid(P)){AbortScene();return;}
  if(!bPosesSet){auto* Clip=LoadObject<UAnimSequence>(nullptr,TEXT("/Game/BattleRetarget/City/Male/M_ragdoll_getup_stand_B"));
  if(!Clip||!Participants[0]->BeginIncidentPose(Clip,Clip->GetPlayLength()*.08f,3600)||!Participants[1]->BeginIncidentPose(Clip,Clip->GetPlayLength()*.3f,3600)){AbortScene();return;}
  bPosesSet=true;SetupAge=0;return;}
  for(auto P:Participants)P->SetActorHiddenInGame(false);SetActorHiddenInGame(false);bSceneReady=true;return;
 }
 auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);if(Pawn&&FVector::Dist2D(Pawn->GetActorLocation(),GetActorLocation())<1500)bVisitStarted=true;
 if(bVisitStarted){VisitAge+=Dt;if(VisitAge>25&&!bReleased){for(auto P:Participants)if(IsValid(P)){P->ReleaseIncidentPose();P->PauseRemaining=1;}bReleased=true;}}
}
