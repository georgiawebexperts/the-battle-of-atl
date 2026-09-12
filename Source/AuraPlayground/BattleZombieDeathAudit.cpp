#include "Rendering/SkinWeightVertexBuffer.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Engine/SkeletalMesh.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "BattleZombie.h"
#include "BattleParkRegion.h"
#include "LandscapeProxy.h"
#include "BattleBike.h"
#include "Camera/CameraActor.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/DamageEvents.h"
#include "UnrealClient.h"
#include "Misc/CommandLine.h"
#include "HAL/PlatformMisc.h"
void TickBattleZombieDeathAudit(ABattleEnemyDirector* D,float Dt){
#if !UE_BUILD_SHIPPING
 auto* W=D->GetWorld();if(W->GetTimeSeconds()<5)return;
 static int Phase=0;static float Clock=0,HeadZ=0,SlopeDegrees=0,MinClearance=9999,MinSkin=9999,MaxSpeed=0;static int BelowSkin=0,SkinSamples=0;static ABattleZombie* Gun=nullptr;static bool Falling=false;
 auto End=[&](bool Pass,const TCHAR* Why){UE_LOG(LogTemp,Display,TEXT("ZombieDeathAudit: {\"passed\":%s,\"reason\":\"%s\",\"slope_degrees\":%.2f,\"minimum_joint_clearance_cm\":%.2f,\"minimum_skin_clearance_cm\":%.2f,\"below_skin_vertices\":%d,\"skin_samples\":%d,\"max_body_speed_cm_s\":%.2f}"),Pass?TEXT("true"):TEXT("false"),Why,SlopeDegrees,MinClearance,MinSkin,BelowSkin,SkinSamples,MaxSpeed);FPlatformMisc::RequestExit(false);};
#define CHECK_DEATH(C,R) if(!(C)){End(false,TEXT(R));Phase=9;return;}
 FString Dir;FParse::Value(FCommandLine::Get(),TEXT("GunmanReviewDir="),Dir);Clock+=Dt;
 if(Phase==0){
  for(TActorIterator<APiedmontPedestrian> It(W);It;++It)It->Destroy();
  auto* B=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(W,0));CHECK_DEATH(B,"No bike");B->Ride->StopMovementImmediately();
  auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(W));M->bTutorialActive=true;
  FVector Spot=B->GetActorLocation()+FVector(500,0,0);
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleDeathSlope"))){
   bool Found=false;float BestNormal=1;FCollisionQueryParams Q;Q.AddIgnoredActor(B);
   for(int X=600;X<=3600;X+=200)for(int Y=-2000;Y<=2000;Y+=200){
    const FVector P=B->GetActorLocation()+FVector(X,Y,0);if(!BattleParkRegion::Contains(P))continue;FHitResult Hit;
    if(W->LineTraceSingleByChannel(Hit,P+FVector(0,0,2000),P-FVector(0,0,2000),ECC_WorldStatic,Q)&&Hit.GetActor()->IsA<ALandscapeProxy>()&&Hit.ImpactNormal.Z>.85f&&Hit.ImpactNormal.Z<BestNormal){BestNormal=Hit.ImpactNormal.Z;Spot=Hit.ImpactPoint+FVector(0,0,92);Found=true;}
   }
   SlopeDegrees=FMath::RadiansToDegrees(FMath::Acos(BestNormal));CHECK_DEATH(Found&&SlopeDegrees>8,"No suitable park slope found");
  }
  const FTransform Spawn(FRotator(0,180,0),Spot);Gun=W->SpawnActorDeferred<ABattleZombie>(ABattleZombie::StaticClass(),Spawn);if(Gun){Gun->VisualStyle=FParse::Param(FCommandLine::Get(),TEXT("BattlePunkReview"))?1:0;Gun->WeaponDropChance=0;Gun->FinishSpawning(Spawn);}CHECK_DEATH(Gun,"No zombie");
  const FVector Eye=Spot+FVector(-380,-420,220);auto* Cam=W->SpawnActor<ACameraActor>(Eye,(Spot-Eye).Rotation());UGameplayStatics::GetPlayerController(W,0)->SetViewTarget(Cam);Phase=1;Clock=0;
 }else if(Phase==1&&Clock>1.5f){
  HeadZ=Gun->Body->GetSocketLocation(TEXT("Head")).Z;UGameplayStatics::ApplyDamage(Gun,100,nullptr,UGameplayStatics::GetPlayerPawn(W,0),UDamageType::StaticClass());
  CHECK_DEATH(Gun->DeathShoeBodies==2,"Shoe collision did not weld to shins");
  CHECK_DEATH(Gun->bDead&&Gun->DeathPhysics&&Gun->DeathPhysics->IsSimulatingPhysics(TEXT("Chest")),"Death did not enter physics");Phase=2;Clock=0;
 }else if(Phase==2){
  if(Clock>.3f&&!Falling&&!Dir.IsEmpty()){FScreenshotRequest::RequestScreenshot(Dir/TEXT("falling.png"),false,false);Falling=true;}
  if(Clock>3.8f){
   auto* Hip=Gun->DeathPhysics->GetBodyInstance(TEXT("Chest"));CHECK_DEATH(Hip,"No physical chest");
   CHECK_DEATH(FVector::Dist(Gun->Body->GetSocketLocation(TEXT("Chest")),Hip->GetUnrealWorldTransform().GetLocation())<10,"Rendered pose does not follow physics");
   CHECK_DEATH(Gun->Body->GetSocketLocation(TEXT("Head")).Z<HeadZ-60,"Attacker remained upright");
   CHECK_DEATH(!Gun->bTelegraphing,"Dead zombie kept attack warning");
   FCollisionQueryParams GroundQuery;GroundQuery.AddIgnoredActor(Gun);
   for(const FName Bone:{FName(TEXT("Head")),FName(TEXT("Chest")),FName(TEXT("Wrist_L")),FName(TEXT("Wrist_R")),FName(TEXT("LowerLeg_L")),FName(TEXT("LowerLeg_R")),FName(TEXT("Foot_L")),FName(TEXT("Foot_R"))}){
    const FVector Point=Gun->Body->GetSocketLocation(Bone);FHitResult Floor;
    if(W->LineTraceSingleByChannel(Floor,Point+FVector(0,0,100),Point-FVector(0,0,400),ECC_WorldStatic,GroundQuery))MinClearance=FMath::Min(MinClearance,float(Point.Z-Floor.ImpactPoint.Z));
   }
   for(const auto& Setup:Gun->DeathPhysics->GetPhysicsAsset()->SkeletalBodySetups)MaxSpeed=FMath::Max(MaxSpeed,float(Gun->DeathPhysics->GetPhysicsLinearVelocity(Setup->BoneName).Size()));
   const auto& LOD=Gun->DeathPhysics->GetSkeletalMeshAsset()->GetResourceForRendering()->LODRenderData[0];auto* Weights=Gun->Body->GetSkinWeightBuffer(0);
   TMap<FName,int32> Penetrating;
   for(uint32 I=0;Weights&&I<LOD.GetNumVertices();I++){
    const FVector Point=Gun->Body->GetComponentTransform().TransformPosition(FVector(USkinnedMeshComponent::GetSkinnedVertexPosition(Gun->Body,I,LOD,*Weights)));FHitResult Floor;
    if(W->LineTraceSingleByChannel(Floor,Point+FVector(0,0,100),Point-FVector(0,0,400),ECC_WorldStatic,GroundQuery)){SkinSamples++;const float Clearance=Point.Z-Floor.ImpactPoint.Z;MinSkin=FMath::Min(MinSkin,Clearance);if(Clearance<-1){BelowSkin++;
     for(const auto& Section:LOD.RenderSections)if(I>=Section.BaseVertexIndex&&I<Section.BaseVertexIndex+Section.NumVertices){uint32 Influence=0;uint16 Weight=0;
      for(uint32 J=0;J<Weights->GetMaxBoneInfluences();J++)if(Weights->GetBoneWeight(I,J)>Weight){Weight=Weights->GetBoneWeight(I,J);Influence=J;}
      const int32 Local=Weights->GetBoneIndex(I,Influence);if(Section.BoneMap.IsValidIndex(Local))Penetrating.FindOrAdd(Gun->DeathPhysics->GetSkeletalMeshAsset()->GetRefSkeleton().GetBoneName(Section.BoneMap[Local]))++;break;
     }
    }}
   }
   for(const auto& Row:Penetrating)UE_LOG(LogTemp,Display,TEXT("ZombieSkinBone: %s vertices=%d"),*Row.Key.ToString(),Row.Value);
   if(!Dir.IsEmpty())FScreenshotRequest::RequestScreenshot(Dir/TEXT("settled.png"),false,false);Phase=3;Clock=0;
  }
 }else if(Phase==3&&Clock>.3f){CHECK_DEATH(MinClearance<9999&&MinClearance>=-15,"Body joint sank beneath terrain");CHECK_DEATH(SkinSamples>0&&MinSkin>=-3,"Visible skin penetrates terrain");CHECK_DEATH(MaxSpeed<40,"Body still moving too fast at end of fall");End(true,TEXT("Fatal damage starts physics, visible chest follows body, head falls and dead zombie attack warning clears"));Phase=9;}
#undef CHECK_DEATH
#endif
}
