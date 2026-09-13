#include "BattleSpareBikes.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleZombie.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
namespace BattleSpareBikes {
void TickAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 static int Phase=0;static float Clock=0;static FTransform Original;static FVector Destination;static ABattleBike* Bike=nullptr;static int Count=0,Reserve=0,Loaded=0,Horns=0;static float Health=0;
 if(Phase==99||PC->GetWorld()->GetTimeSeconds()<5)return;Clock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("SpareBikeAudit: {\"passed\":%s,\"phase\":%d,\"bikes\":%d,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Phase,Count,Reason);Phase=99;PC->ConsoleCommand(TEXT("quit"));};
#define BCHECK(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Approach=[&](ABattleRider* P,FVector Position){
  FCollisionQueryParams Q(SCENE_QUERY_STAT(SpareBikeAuditApproach),false,P);Q.AddIgnoredActor(Bike);
  for(FVector D:{FVector(1,0,0),FVector(-1,0,0),FVector(0,1,0),FVector(0,-1,0)}){FVector C=Position+D*145;FHitResult G;if(!PC->GetWorld()->LineTraceSingleByChannel(G,C+FVector(0,0,200),C-FVector(0,0,400),ECC_Visibility,Q)||G.ImpactNormal.Z<.8f)continue;C=G.ImpactPoint+FVector(0,0,91);if(PC->GetWorld()->OverlapBlockingTestByChannel(C,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,88),Q))continue;P->SetActorLocation(C,false,nullptr,ETeleportType::TeleportPhysics);return true;}return false;
 };
 auto E=[&](){for(bool Down:{true,false})PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::E,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 if(Phase==0){
  Bike=Cast<ABattleBike>(PC->GetPawn());BCHECK(Bike,"No player bike");TArray<FVector> Positions;Locations(PC,Positions);Count=Positions.Num();BCHECK(Count>=4,"Insufficient spare bikes in actual world");Destination=Positions[0];Original=Bike->GetActorTransform();Bike->Inventory[0].Reserve=34;Bike->HornUses=3;Bike->RiderHealth=83;Bike->HurtCooldown=30; // Isolate remount preservation from passive health regeneration.
  if(auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(PC)))if(Mode->Enemies)Mode->Enemies->bFreezeSpawns=true;
  for(TActorIterator<ABattleZombie> It(PC->GetWorld());It;++It)It->Destroy();
  BCHECK(Bike->Dismount(),"Initial dismount failed");auto* P=Cast<ABattleRider>(PC->GetPawn());BCHECK(P&&Approach(P,Destination),"No walkable approach to spare bike");Phase=1;Clock=0;
 }else if(Phase==1&&Clock>.4f){
  static bool Captured=false;FString ReviewDir;FParse::Value(FCommandLine::Get(),TEXT("BattleSpareBikeReviewDir="),ReviewDir);
  if(!ReviewDir.IsEmpty()&&!Captured){auto* Camera=PC->GetWorld()->SpawnActor<ACameraActor>();const FVector Eye=Destination+FVector(240,-330,150);Camera->SetActorLocationAndRotation(Eye,(Destination+FVector(0,0,10)-Eye).Rotation());PC->SetViewTarget(Camera);FScreenshotRequest::RequestScreenshot(ReviewDir/TEXT("spare-bike.png"),true,false);Captured=true;Clock=0;return;}
  auto* P=Cast<ABattleRider>(PC->GetPawn());BCHECK(P&&Nearest(P),"No nearby spare interaction");P->bSwimming=true;const bool Rejected=!P->MountBike();P->bSwimming=false;BCHECK(Rejected,"Swimming allowed mounting");Reserve=Bike->Inventory[0].Reserve;Loaded=P->Ammo;Horns=Bike->HornUses;Health=Bike->RiderHealth;E();Phase=2;Clock=0;
 }else if(Phase==2&&Clock>.3f){
  BCHECK(PC->GetPawn()==Bike&&!Bike->bParked&&FVector::Dist2D(Bike->GetActorLocation(),Destination)<30,"E did not switch to spare bike");UE_LOG(LogTemp,Display,TEXT("SpareState: reserve=%d/%d loaded=%d/%d horns=%d/%d health=%.4f/%.4f"),Bike->Inventory[0].Reserve,Reserve,Bike->PistolAmmo,Loaded,Bike->HornUses,Horns,Bike->RiderHealth,Health);BCHECK(Bike->Inventory[0].Reserve==Reserve&&Bike->PistolAmmo==Loaded&&Bike->HornUses==Horns&&FMath::IsNearlyEqual(Bike->RiderHealth,Health),"Switch lost carried state");TArray<FVector> Positions;Locations(PC,Positions);BCHECK(Positions.ContainsByPredicate([&](FVector P){return P.Equals(Original.GetLocation(),1.f);}),"Abandoned bike disappeared");
  BCHECK(Bike->Dismount(),"Second dismount failed");auto* P=Cast<ABattleRider>(PC->GetPawn());BCHECK(P&&Approach(P,Original.GetLocation()),"Could not approach abandoned bike");Phase=3;Clock=0;
 }else if(Phase==3&&Clock>.3f){E();Phase=4;Clock=0;}
 else if(Phase==4&&Clock>.3f){BCHECK(PC->GetPawn()==Bike&&FVector::Dist2D(Bike->GetActorLocation(),Original.GetLocation())<30&&Bike->Inventory[0].Reserve>=Reserve,"Could not ride abandoned bike again with supplies");Finish(true,TEXT("Actual spare stations, E interaction, carried ammo/health/horns, swimming guard and riding the abandoned bike again pass"));}
#undef BCHECK
#endif
}
}
