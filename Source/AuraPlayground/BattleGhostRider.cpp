#include "BattleGhostRider.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
ABattleGhostRider::ABattleGhostRider(){
 PrimaryActorTick.bCanEverTick=false;
 SetActorHiddenInGame(true);
 Visual=CreateDefaultSubobject<USceneComponent>(TEXT("GhostLeanAssembly"));Visual->SetupAttachment(RootComponent);Visual->SetRelativeLocation(FVector(0,0,-96));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Glow(TEXT("/Game/BattleForTheA/Materials/M_DiscGlow.M_DiscGlow"));
 auto Part=[&](FString Name,FVector Loc,FVector Scale,bool Round){auto* M=CreateDefaultSubobject<UStaticMeshComponent>(*Name);M->SetupAttachment(Visual);M->SetStaticMesh(Round?Cylinder.Object:Cube.Object);M->SetRelativeLocation(Loc);M->SetRelativeScale3D(Scale);M->SetCollisionEnabled(ECollisionEnabled::NoCollision);M->SetMaterial(0,Glow.Object);return M;};
 auto Tube=[&](FString Name,FVector A,FVector B,float Radius){auto* M=Part(Name,(A+B)*.5,FVector(Radius/50,Radius/50,(B-A).Size()/100),true);M->SetRelativeRotation(FQuat::FindBetweenNormals(FVector::UpVector,(B-A).GetSafeNormal()));return M;};
 const FVector Back(-60,0,35),Front(60,0,35),Crank(-5,0,36),Seat(-23,0,82),Head(43,0,91);
 Tube(TEXT("SeatTube"),Crank,Seat,2.2);Tube(TEXT("TopTube"),Seat,Head,2);Tube(TEXT("DownTube"),Crank,Head,3);
 for(int S:{-1,1}){Tube(FString::Printf(TEXT("Stay%d"),S),Back+FVector(0,S*7,0),Seat,1.4);Tube(FString::Printf(TEXT("ChainStay%d"),S),Back+FVector(0,S*7,0),Crank,1.4);Tube(FString::Printf(TEXT("Fork%d"),S),Head,Front+FVector(0,S*6,0),1.6);}
 Tube(TEXT("Stem"),Head,FVector(40,0,112),2);Tube(TEXT("Handlebar"),FVector(40,-30,112),FVector(40,30,112),1.5);
 Part(TEXT("Saddle"),FVector(-23,0,88),FVector(.29,.19,.055),false);
 FrontWheel=Part(TEXT("FrontWheel"),Front,FVector(.70,.70,.055),true);FrontWheel->SetRelativeRotation(FRotator(0,0,90));
 RearWheel=Part(TEXT("RearWheel"),Back,FVector(.70,.70,.055),true);RearWheel->SetRelativeRotation(FRotator(0,0,90));
 auto* Aura=CreateDefaultSubobject<UPointLightComponent>(TEXT("GhostAura"));Aura->SetupAttachment(RootComponent);Aura->SetRelativeLocation(FVector(0,0,40));Aura->SetLightColor(FLinearColor(.12f,.55f,1.f));Aura->SetIntensity(900);Aura->SetAttenuationRadius(420);Aura->SetCastShadows(false);
}
void ABattleGhostRider::LoadRoute(const TArray<FVector>& InSamples,float InStride){
 if(InSamples.Num()<2||InStride<=0)return;
 Samples=InSamples;Stride=InStride;bLoaded=true;WheelAngle=0;LastPos=Samples[0];SetActorLocation(Samples[0]);SetActorHiddenInGame(false);
}
void ABattleGhostRider::Advance(float Elapsed){
 if(!bLoaded)return;
 const int32 Index=FMath::Clamp(FMath::FloorToInt(Elapsed/Stride),0,Samples.Num()-1);
 const int32 Next=FMath::Min(Index+1,Samples.Num()-1);
 const FVector Pos=FMath::Lerp(Samples[Index],Samples[Next],FMath::Clamp((Elapsed/Stride)-Index,0.f,1.f));
 const FVector Dir=(Samples[Next]-Samples[Index]);
 if(!Dir.IsNearlyZero())SetActorRotation(Dir.Rotation());
 WheelAngle+=FMath::RadiansToDegrees((Pos-LastPos).Size()/35.f);LastPos=Pos;
 FrontWheel->SetRelativeRotation(FRotator(WheelAngle,0,90));RearWheel->SetRelativeRotation(FRotator(WheelAngle,0,90));
 SetActorLocation(Pos);
}