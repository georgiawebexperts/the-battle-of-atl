#include "BattleRideFX.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
ABattleRideFX::ABattleRideFX(){
 PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("EffectsOrigin"));
 Tracks=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TireTracks"));Tracks->SetupAttachment(RootComponent);
 Spray=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("SplashDrops"));Spray->SetupAttachment(RootComponent);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube")),Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Rubber(TEXT("/Game/BeltLineGlide/Materials/M_Rubber.M_Rubber"));
 Tracks->SetStaticMesh(Cube.Object);Tracks->SetMaterial(0,Rubber.Object);Spray->SetStaticMesh(Sphere.Object);
 for(auto* Part:{Tracks.Get(),Spray.Get()}){Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);Part->SetCastShadow(false);Part->SetCanEverAffectNavigation(false);}
}
void ABattleRideFX::AddSkid(FVector Start,FVector End,FVector Normal){
 const FVector Delta=End-Start;if(Delta.Size()<2||Delta.Size()>150)return;
 FTrack Mark;Mark.Transform=FTransform(FRotationMatrix::MakeFromXZ(Delta,Normal).ToQuat(),(Start+End)*.5f+Normal*.25f,FVector(Delta.Size()/100,.055,.003));Mark.Life=10;
 if(Trail.Num()<160){Trail.Add(Mark);Tracks->AddInstance(Mark.Transform,true);}else {Trail[NextTrack]=Mark;Tracks->UpdateInstanceTransform(NextTrack,Mark.Transform,true,true);NextTrack=(NextTrack+1)%160;}TrackCount++;
}
void ABattleRideFX::Splash(FVector Position){
 if(auto* Material=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_Splash.M_Splash")))Spray->SetMaterial(0,Material);
 SplashCount++;SplashAge=0;Drops.Empty();Spray->ClearInstances();FRandomStream Random(SplashCount*911);
 for(int I=0;I<48;I++){
  const float Angle=I*2*PI/48;FDrop D;D.Position=Position+FVector(FMath::Cos(Angle),FMath::Sin(Angle),0)*Random.FRandRange(10,55);D.Velocity=FVector(FMath::Cos(Angle)*Random.FRandRange(100,350),FMath::Sin(Angle)*Random.FRandRange(100,350),Random.FRandRange(230,570));D.Life=Random.FRandRange(.7f,1.4f);D.Scale=Random.FRandRange(.035f,.11f);Drops.Add(D);Spray->AddInstance(FTransform(FQuat::Identity,D.Position,FVector(D.Scale)),true);
 }ActiveDrops=Drops.Num();
}
void ABattleRideFX::Tick(float Dt){
 Super::Tick(Dt);bool Dirty=false;
 for(int I=0;I<Trail.Num();I++){auto& M=Trail[I];if(M.Life<=0)continue;M.Life=FMath::Max(0.f,M.Life-Dt);if(M.Life>2)continue;FTransform T=M.Transform;FVector Scale=T.GetScale3D();Scale.Y*=FMath::Clamp(M.Life/2,0.f,1.f);T.SetScale3D(Scale);Tracks->UpdateInstanceTransform(I,T,true,false);Dirty=true;}
 if(Dirty)Tracks->MarkRenderStateDirty();ActiveDrops=0;
 for(int I=0;I<Drops.Num();I++){auto& D=Drops[I];if(D.Life<=0)continue;D.Life=FMath::Max(0.f,D.Life-Dt);D.Velocity.Z-=980*Dt;D.Position+=D.Velocity*Dt;Spray->UpdateInstanceTransform(I,FTransform(FQuat::Identity,D.Position,FVector(D.Scale*FMath::Min(1.f,D.Life*4))),true,false);if(D.Life>0)ActiveDrops++;}
 if(Drops.Num()){Spray->MarkRenderStateDirty();if(ActiveDrops==0){Drops.Empty();Spray->ClearInstances();}}
}
