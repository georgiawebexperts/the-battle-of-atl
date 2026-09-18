#include "BattlePicnic.h"
#include "BattleBike.h"
#include "BattleFrisbee.h"
#include "PiedmontPedestrian.h"
#include "AIController.h"
#include "Camera/CameraActor.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
#include "UObject/ConstructorHelpers.h"

ABattlePicnicGroup::ABattlePicnicGroup(){
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("PicnicRoot"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 Blanket=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PicnicBlanket"));Blanket->SetupAttachment(RootComponent);Blanket->SetStaticMesh(Cube.Object);Blanket->SetRelativeLocation(FVector(0,0,15));Blanket->SetRelativeScale3D(FVector(2.6,1.8,.05));
 PicnicBag=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PicnicBag"));PicnicBag->SetupAttachment(RootComponent);PicnicBag->SetStaticMesh(Cube.Object);PicnicBag->SetRelativeLocation(FVector(-65,35,22));PicnicBag->SetRelativeScale3D(FVector(.28,.18,.22));
 Drink=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PicnicDrink"));Drink->SetupAttachment(RootComponent);Drink->SetStaticMesh(Cylinder.Object);Drink->SetRelativeLocation(FVector(62,-34,15));Drink->SetRelativeScale3D(FVector(.055,.055,.15));
 for(auto* Part:{Blanket.Get(),PicnicBag.Get(),Drink.Get()}){Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);Part->SetCanEverAffectNavigation(false);}
 Tags.Add(TEXT("BattlePicnicGroup"));
}
void ABattlePicnicGroup::BeginPlay(){
 Super::BeginPlay();
 auto* Cloth=LoadObject<UMaterialInterface>(nullptr,GroupVariant%2?TEXT("/Game/BattleForTheA/Materials/M_ColaRed.M_ColaRed"):TEXT("/Game/PiedmontRide/Materials/M_Safety.M_Safety"));
 Blanket->SetMaterial(0,Cloth);PicnicBag->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/PiedmontRide/Materials/M_BridgeWood.M_BridgeWood")));Drink->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_ColaRed.M_ColaRed")));
 // Side by side across the cloth's short axis, both facing the same way, so the
 // two seated pairs of legs run parallel instead of into each other.
 const FVector Offsets[]={FVector(-26,-52,88),FVector(26,52,88)};
 const float ClothTopZ=Blanket->GetComponentLocation().Z+50.f*Blanket->GetComponentScale().Z;
 for(int32 I=0;I<2;I++){
  const FTransform T(FRotator(0,GetActorRotation().Yaw,0),GetActorLocation()+GetActorRotation().RotateVector(Offsets[I]));
  auto* Person=GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),T,this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  if(!Person)continue;Person->bPicnicChiller=true;Person->PicnicPose=(GroupVariant+I)%3;Person->PicnicClothZ=ClothTopZ;Person->bPicnicClothSet=true;Person->CityAppearanceVariant=(GroupVariant*2+I)%8;Person->CityOutfitVariant=(GroupVariant*3+I)%8;Person->FinishSpawning(T);Chillers.Add(Person);
}
 bReady=Chillers.Num()==2;
}
void ABattlePicnicGroup::EndPlay(const EEndPlayReason::Type Reason){for(auto P:Chillers)if(IsValid(P))P->Destroy();Super::EndPlay(Reason);}

ABattlePicnicDirector::ABattlePicnicDirector(){PrimaryActorTick.bCanEverTick=true;}
void ABattlePicnicDirector::BeginPlay(){
 Super::BeginPlay();
#if !UE_BUILD_SHIPPING
 const bool OwnAudit=FParse::Param(FCommandLine::Get(),TEXT("BattlePicnicAudit"));if(FString(FCommandLine::Get()).Contains(TEXT("Audit"))&&!OwnAudit)return;
#endif
 const auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!Mode)return;DesiredGroups=FMath::Clamp(Mode->Difficulty.PicnicGroups,0,10);
 const FVector Sites[]={FVector(5200,5600,0),FVector(6650,7350,0),FVector(-7600,8200,0),FVector(-9800,6800,0),FVector(3100,2250,0),FVector(7600,4700,0),FVector(-5200,9900,0),FVector(1700,7100,0),FVector(-11200,8800,0),FVector(5850,9300,0)};
 for(int32 I=0;I<DesiredGroups;I++){
  FVector Ground;if(!ABattleFrisbeeGroup::Ground(GetWorld(),Sites[I],Ground,true)){PlacementFailures++;continue;}
  const FTransform T(FRotator(0,31.f*I-70.f,0),Ground+FVector(0,0,4));auto* Group=GetWorld()->SpawnActorDeferred<ABattlePicnicGroup>(ABattlePicnicGroup::StaticClass(),T,this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  if(!Group){PlacementFailures++;continue;}Group->GroupVariant=I;Group->FinishSpawning(T);if(Group->bReady)Groups.Add(Group);else{Group->Destroy();PlacementFailures++;}
 }
 UE_LOG(LogTemp,Display,TEXT("BattlePicnic: groups=%d desired=%d chillers=%d failures=%d"),Groups.Num(),DesiredGroups,Groups.Num()*2,PlacementFailures);
}
void ABattlePicnicDirector::Tick(float Dt){
 Super::Tick(Dt);
#if !UE_BUILD_SHIPPING
 if(bAuditDone||!FParse::Param(FCommandLine::Get(),TEXT("BattlePicnicAudit")))return;AuditClock+=Dt;
 if(Groups.Num()!=DesiredGroups){if(AuditClock>3){UE_LOG(LogTemp,Display,TEXT("BattlePicnicAudit: {\"passed\":false,\"groups\":%d,\"desired\":%d,\"failures\":%d}"),Groups.Num(),DesiredGroups,PlacementFailures);bAuditDone=true;if(auto* PC=UGameplayStatics::GetPlayerController(this,0))UKismetSystemLibrary::QuitGame(this,PC,EQuitPreference::Quit,false);}return;}
 FString ReviewDir;const bool Review=FParse::Value(FCommandLine::Get(),TEXT("BattlePicnicReviewDir="),ReviewDir);
 // -BattlePicnicReviewAngle=<degrees> orbits the review camera, so a pose can be
 // judged from the front instead of only from whichever side the default offset
 // happens to give. The default is the angle that framed the original defect.
 if(Review&&!bReviewSetup&&AuditClock>.25f){
  float Orbit=-47.f;FParse::Value(FCommandLine::Get(),TEXT("BattlePicnicReviewAngle="),Orbit);
  const FVector Focus=Groups[0]->GetActorLocation()+FVector(0,0,55),Eye=Focus+FRotator(0,Orbit,0).RotateVector(FVector(0,-615,190));
  if(auto* PC=UGameplayStatics::GetPlayerController(this,0))if(auto* Camera=GetWorld()->SpawnActor<ACameraActor>(Eye,(Focus-Eye).Rotation()))PC->SetViewTarget(Camera);bReviewSetup=true;
 }
 auto* Sample=Groups[0]->Chillers[0].Get();if(!bAuditSampled&&AuditClock>.6f&&Sample){AuditHand=Sample->Body->GetSocketLocation(TEXT("hand_r"));bAuditSampled=true;}
 if(bAuditSampled&&Sample)AuditMotion=FMath::Max(AuditMotion,float(FVector::Dist(AuditHand,Sample->Body->GetSocketLocation(TEXT("hand_r")))));
 if(Review&&!ReviewDir.IsEmpty()&&AuditClock>5.1f&&AuditClock-Dt<=5.1f)FScreenshotRequest::RequestScreenshot(ReviewDir/TEXT("picnic-groups.png"),false,false);
 if(AuditClock<(Review?6.f:2.f))return;
 int32 Chillers=0,Animated=0;for(auto G:Groups)if(G&&G->bReady)for(auto P:G->Chillers){Chillers++;if(P&&P->bNativeCrowdRig&&P->bPicnicChiller&&P->ChillClock>1)Animated++;}
 bool Panic=false;if(Sample){Sample->HearGunfire(Sample->GetActorLocation());Panic=Sample->PanicRemaining>0;}
 const bool Pass=Groups.Num()==DesiredGroups&&Chillers==DesiredGroups*2&&Animated==Chillers&&AuditMotion>2&&Panic;
 UE_LOG(LogTemp,Display,TEXT("BattlePicnicAudit: {\"passed\":%s,\"groups\":%d,\"desired\":%d,\"chillers\":%d,\"animated\":%d,\"hand_travel_cm\":%.2f,\"panic_response\":%s}"),Pass?TEXT("true"):TEXT("false"),Groups.Num(),DesiredGroups,Chillers,Animated,AuditMotion,Panic?TEXT("true"):TEXT("false"));bAuditDone=true;FTimerHandle Quit;GetWorldTimerManager().SetTimer(Quit,[this](){if(auto* PC=UGameplayStatics::GetPlayerController(this,0))UKismetSystemLibrary::QuitGame(this,PC,EQuitPreference::Quit,false);},Review?.8f:.05f,false);
#endif
}
void ABattlePicnicDirector::EndPlay(const EEndPlayReason::Type Reason){for(auto G:Groups)if(IsValid(G))G->Destroy();Super::EndPlay(Reason);}
