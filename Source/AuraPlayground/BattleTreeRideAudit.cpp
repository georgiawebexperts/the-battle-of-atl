#include "BattleBike.h"
#include "BattleZombie.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
void TickBattleTreeRideAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FTree{FVector Point;float Radius;FString Mesh;};
 struct FState{TWeakObjectPtr<UWorld> World;TArray<FTree> Trees;int Tree=0,Leg=0,Contacts=0,Completed=0;float Clock=0;bool Started=false,Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;
 auto Key=[&](bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::W,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Finish=[&](bool Pass,const TCHAR* Why){Key(false);S.Done=true;UE_LOG(LogTemp,Display,TEXT("TreeRideAudit: {\"passed\":%s,\"reason\":\"%s\",\"completed\":%d,\"tree\":%d,\"leg\":%d}"),Pass?TEXT("true"):TEXT("false"),Why,S.Completed,S.Tree,S.Leg);PC->ConsoleCommand(TEXT("quit"));};
 auto* Bike=Cast<ABattleBike>(PC->GetPawn());if(!Bike){Finish(false,TEXT("Missing bike"));return;}auto* Move=Bike->Ride.Get();
 auto Floor=[&](FVector Point,FVector& Ground){FHitResult Hit;FCollisionQueryParams Q;Q.bTraceComplex=true;Q.AddIgnoredActor(Bike);if(!PC->GetWorld()->LineTraceSingleByChannel(Hit,Point+FVector(0,0,2000),Point-FVector(0,0,2000),ECC_WorldStatic,Q)||!Hit.GetActor()||Hit.GetActor()->ActorHasTag(TEXT("RideTree")))return false;Ground=Hit.ImpactPoint;return true;};
 if(!S.Started){
  if(auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(PC)))if(Mode->Enemies)Mode->Enemies->bFreezeSpawns=true;
  for(TActorIterator<ABattleZombie> I(PC->GetWorld());I;++I)I->Destroy();for(TActorIterator<APiedmontPedestrian> I(PC->GetWorld());I;++I)I->Destroy();
  TArray<FTree> All;
  for(TActorIterator<AActor> It(PC->GetWorld());It;++It)if(It->ActorHasTag(TEXT("RideTree"))){TArray<UInstancedStaticMeshComponent*> Components;It->GetComponents(Components);for(auto* C:Components){UStaticMesh* Mesh=C->GetStaticMesh();if(!Mesh||!Mesh->GetBodySetup()||Mesh->GetBodySetup()->AggGeom.SphylElems.Num()!=1)continue;const auto& Capsule=Mesh->GetBodySetup()->AggGeom.SphylElems[0];for(int I=0;I<C->GetInstanceCount();I++){FTransform T;C->GetInstanceTransform(I,T,true);All.Add({T.TransformPosition(Capsule.Center),Capsule.Radius*T.GetScale3D().GetMax(),Mesh->GetName()});}}}
  TSet<FString> Chosen;
  for(const auto& Candidate:All){if(Chosen.Contains(Candidate.Mesh))continue;bool Clear=true;
   for(const auto& Other:All){if(&Candidate==&Other)continue;const FVector D=Other.Point-Candidate.Point;if(D.X>-950&&D.X<500&&FMath::Abs(D.Y)<Candidate.Radius+Other.Radius+180){Clear=false;break;}}
   FVector A,B;if(!Clear||!Floor(Candidate.Point+FVector(-800,Candidate.Radius+100,0),A)||!Floor(Candidate.Point+FVector(350,Candidate.Radius+100,0),B)||FMath::Abs(A.Z-B.Z)>120)continue;
   // Verify both approaches against real world geometry, not just other tree centres.
   for(int Leg=0;Leg<2&&Clear;Leg++){
    const float Offset=Leg==0?Candidate.Radius+100:0;const float End=Leg==0?350:-Candidate.Radius-80;FVector Previous;
    if(!Floor(Candidate.Point+FVector(-800,Offset,0),Previous)){Clear=false;break;}
    for(float X=-750;X<=End+1;X+=50){FVector Next;if(!Floor(Candidate.Point+FVector(X,Offset,0),Next)||FMath::Abs(Next.Z-Previous.Z)>40){Clear=false;break;}
     FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(Bike);
     if(PC->GetWorld()->SweepSingleByChannel(Hit,Previous+FVector(0,0,98),Next+FVector(0,0,98),FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,90),Q)&&Hit.ImpactNormal.Z<.6f){Clear=false;break;}Previous=Next;
    }
   }
   if(!Clear)continue;
   S.Trees.Add(Candidate);Chosen.Add(Candidate.Mesh);if(S.Trees.Num()==3)break;
  }
  if(S.Trees.Num()!=3){Finish(false,TEXT("Could not find three clear representative approaches"));return;}S.Started=true;
 }
 const auto& Tree=S.Trees[S.Tree];
 if(S.Clock==0){FVector Ground;const FVector Start=Tree.Point+FVector(-800,S.Leg==0?Tree.Radius+100:0,0);if(!Floor(Start,Ground)){Finish(false,TEXT("Missing approach floor"));return;}Key(false);Move->StopMovementImmediately();Move->Speed=0;Move->Recovery=0;Move->Gear=3;Bike->SetActorLocationAndRotation(Ground+FVector(0,0,98),FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);Move->SetMovementMode(MOVE_Walking);Move->bForceNextFloorCheck=true;Bike->RiderHealth=100;Bike->DamageGrace=0;Bike->HurtCooldown=100;S.Contacts=Move->TreeContacts;S.Clock=.001f;return;}
 S.Clock+=Dt;if(S.Clock<.6f)return;Key(true);
 if(S.Clock>15){UE_LOG(LogTemp,Display,TEXT("TreeRide state: mesh=%s position=%s target=%s speed=%.1f contacts=%d"),*Tree.Mesh,*Bike->GetActorLocation().ToString(),*Tree.Point.ToString(),Move->Speed,Move->TreeContacts-S.Contacts);Finish(false,TEXT("Tree approach timed out"));return;}
 if(S.Leg==0&&Move->TreeContacts!=S.Contacts){Finish(false,TEXT("Near pass hit a trunk"));return;}
 bool Complete=S.Leg==0?Bike->GetActorLocation().X>Tree.Point.X+300:Move->TreeContacts>S.Contacts&&Bike->bCrashActive&&Bike->RiderHealth<100;
 if(Complete){Key(false);if(S.Leg==1){Bike->ClearPhysicalCrash();Bike->bParked=false;Bike->Rider->SetVisibility(true,true);Move->Recovery=0;}++S.Completed;S.Clock=0;if(S.Leg==0)S.Leg=1;else{S.Leg=0;++S.Tree;}if(S.Tree==3)Finish(true,TEXT("Three authored tree types: keyboard near passes clear and direct hits knock the rider off with harm"));}
#endif
}
