#include "GlideGame.h"
#include "Components/InputComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"

AGlideBike::AGlideBike(){
 PrimaryActorTick.bCanEverTick=true;
 auto* Box=CreateDefaultSubobject<UBoxComponent>(TEXT("BikeCollision"));RootComponent=Box;
 Box->SetBoxExtent(FVector(75,30,65));Box->SetCollisionProfileName(TEXT("Pawn"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Wheel(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Paint(TEXT("/Game/BeltLineGlide/Materials/M_Bike.M_Bike"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Rubber(TEXT("/Game/BeltLineGlide/Materials/M_Rubber.M_Rubber"));
 auto Part=[&](const TCHAR* Name,FVector Loc,FVector Scale,bool Tire){
  auto* M=CreateDefaultSubobject<UStaticMeshComponent>(Name);M->SetupAttachment(RootComponent);M->SetStaticMesh(Tire?Wheel.Object:Cube.Object);
  M->SetRelativeLocation(Loc);M->SetRelativeScale3D(Scale);M->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  M->SetMaterial(0,Tire?Rubber.Object:Paint.Object);if(Tire)M->SetRelativeRotation(FRotator(0,0,90));
 };
 Part(TEXT("Frame"),FVector(0,0,-5),FVector(1.1,.14,.18),false);
 Part(TEXT("Seat"),FVector(-25,0,25),FVector(.32,.26,.10),false);
 Part(TEXT("Stem"),FVector(45,0,15),FVector(.10,.10,.50),false);
 Part(TEXT("Handlebar"),FVector(45,0,40),FVector(.10,.65,.10),false);
 Part(TEXT("RearWheel"),FVector(-55,0,-40),FVector(.65,.65,.12),true);
 Part(TEXT("FrontWheel"),FVector(55,0,-40),FVector(.65,.65,.12),true);
 auto* Arm=CreateDefaultSubobject<USpringArmComponent>(TEXT("ChaseArm"));Arm->SetupAttachment(RootComponent);Arm->TargetArmLength=650;Arm->SetRelativeLocation(FVector(0,0,180));Arm->SetRelativeRotation(FRotator(-18,0,0));Arm->bDoCollisionTest=false;
 auto* Cam=CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));Cam->SetupAttachment(Arm);Cam->FieldOfView=85;
}
void AGlideBike::BeginPlay(){
 Super::BeginPlay();Start=GetActorLocation();Start.Z=90;StartRotation=GetActorRotation();StartRotation.Pitch=StartRotation.Roll=0;SetActorLocationAndRotation(Start,StartRotation);
 int32 I=0;for(TActorIterator<AActor> It(GetWorld());It;++It){
  const FString N=It->GetClass()->GetName();bool Scooter=N.Contains(TEXT("BP_ScooterRider"));
  if(Scooter||N.Contains(TEXT("BP_Pedestrian"))){
   It->SetActorTickEnabled(false);FVector P=It->GetActorLocation();P.Z=90;It->SetActorLocation(P);It->SetActorRotation(FRotator::ZeroRotator);
   Traffic.Add({*It,P,float(I++)*.7f,Scooter});
  }
 }
}
void AGlideBike::SetupPlayerInputComponent(UInputComponent* I){
 Super::SetupPlayerInputComponent(I);
 I->BindAxis(TEXT("GlideThrottle"),this,&AGlideBike::ThrottleInput);I->BindAxis(TEXT("GlideTurn"),this,&AGlideBike::TurnInput);
 I->BindAction(TEXT("GlideBrake"),IE_Pressed,this,&AGlideBike::BrakeOn);I->BindAction(TEXT("GlideBrake"),IE_Released,this,&AGlideBike::BrakeOff);I->BindAction(TEXT("GlideReset"),IE_Pressed,this,&AGlideBike::ResetRide);
}
void AGlideBike::ResetRide(){SetActorLocationAndRotation(Start,StartRotation,false,nullptr,ETeleportType::TeleportPhysics);Speed=0;Cooldown=2;}
void AGlideBike::Tick(float Dt){
 Super::Tick(Dt);Dt=FMath::Min(Dt,.05f);Elapsed+=Dt;Cooldown=FMath::Max(0.f,Cooldown-Dt);
 const float Target=Braking?0.f:Throttle*1000.f;Speed=FMath::FInterpConstantTo(Speed,Target,Dt,Braking?3500.f:1200.f);
 AddActorWorldRotation(FRotator(0,Turn*95.f*Dt,0));const FVector Before=GetActorLocation();
 FVector Next=Before+GetActorForwardVector()*Speed*Dt;Next.Z=90;
 SetActorLocation(Next,true);const float Travel=FVector::Dist2D(Before,GetActorLocation());Distance+=Travel/100.f;Score=FMath::Max(0,int32(Distance)-Hits*50);
 for(auto& T:Traffic){if(!T.Actor.IsValid())continue;AActor* A=T.Actor.Get();float Rate=T.Scooter?1.0f:.4f;
  const float Phase=Elapsed*Rate+T.Phase;FVector P=T.Origin+FVector(FMath::Sin(Phase)*400,FMath::Sin(Phase*.5f)*120,0);P.Z=90;A->SetActorLocation(P);A->SetActorRotation(FRotator(0,FMath::Cos(Phase)>0?0:180,0));
  if(Cooldown<=0&&FVector::DistSquared2D(GetActorLocation(),P)<FMath::Square(125.f)){++Hits;ResetRide();}
 }
}
void AGlideHUD::DrawHUD(){
 Super::DrawHUD();if(!Canvas)return;auto* B=Cast<AGlideBike>(GetOwningPawn());if(!B)return;
 DrawRect(FLinearColor(0.02,0.04,0.05,.85),18,18,560,128);
 DrawText(TEXT("BELTLINE GLIDE  |  Piedmont Park prototype"),FColor::White,32,28,nullptr,1.2f);
 DrawText(TEXT("WASD or ARROWS: ride/steer   SPACE: brake   R: reset"),FColor::White,32,57);
 DrawText(FString::Printf(TEXT("Speed %.0f mph    Score %d    Collisions %d"),FMath::Abs(B->Speed)*.0223694f,B->Score,B->Hits),FColor(94,220,235),32,81,nullptr,1.1f);
 DrawText(TEXT("Avoid pedestrians and scooter riders. A collision resets the bike."),FColor::White,32,110);
 DrawText(TEXT("Map: OpenStreetMap contributors (ODbL). Flat, approximate lake circuit."),FColor::White,20,Canvas->SizeY-28);
}
AGlideGameMode::AGlideGameMode(){DefaultPawnClass=AGlideBike::StaticClass();HUDClass=AGlideHUD::StaticClass();}
