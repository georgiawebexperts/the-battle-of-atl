#include "BattleBenchFire.h"
#include "BattleParkFurniture.h"
#include "PiedmontPedestrian.h"
#include "Components/PoseableMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
void TickBattleBenchFireReview(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ABattleBenchFire> Fire;TWeakObjectPtr<APiedmontPedestrian> Visitor;FTransform Bench;float Clock=0;int Phase=0;bool Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 const bool bIgnition=FParse::Param(FCommandLine::Get(),TEXT("BattleBenchIgnitionReview"));
 if(S.Phase==0){
  for(TActorIterator<ABattleParkFurniture> It(PC->GetWorld());It;++It)if(It->Benches.Num()){
   const FTransform T=It->Benches[0];S.Bench=T;
   if(bIgnition||FParse::Param(FCommandLine::Get(),TEXT("BattleBenchReachReview"))){
    const FTransform Spawn((T.TransformVectorNoScale(FVector(0,-1,0))).Rotation(),T.TransformPosition(FVector(0,55,90)));
    auto* Visitor=PC->GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),Spawn,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
    Visitor->CityAppearanceVariant=0;UGameplayStatics::FinishSpawningActor(Visitor,Spawn);Visitor->PauseRemaining=12;S.Visitor=Visitor;
    UE_LOG(LogTemp,Display,TEXT("BenchReachReview: started=%d"),bIgnition?Visitor->BeginBenchIgnition(*It,0):Visitor->BeginBenchReach());
   }else S.Fire=ABattleBenchFire::IgniteBench(*It,0,6.f);
   auto* Camera=PC->GetWorld()->SpawnActor<ACameraActor>();const FVector Look=T.GetLocation()+FVector(0,0,100),Offset=T.TransformVectorNoScale(FVector(230,420,180));
   Camera->GetCameraComponent()->SetFieldOfView(50);Camera->SetActorLocation(Look+Offset);Camera->SetActorRotation((-Offset).Rotation());PC->SetViewTarget(Camera);if(PC->GetHUD())PC->GetHUD()->bShowHUD=false;
   S.Phase=1;S.Clock=0;break;
  }
 }
 const bool bReach=FParse::Param(FCommandLine::Get(),TEXT("BattleBenchReachReview"));
 const float CaptureTimes[]={bIgnition?1.5f:.12f,bIgnition?2.2f:(bReach?2.2f:2.f),5.85f};
 if(S.Phase>=1&&S.Phase<=3&&S.Clock>CaptureTimes[S.Phase-1]){
  if(S.Fire.IsValid())UE_LOG(LogTemp,Display,TEXT("BenchFireEnvelope: sample=%d age=%.3f flame=%.4f smoke=%.4f"),S.Phase,S.Clock,S.Fire->FlameStrength,S.Fire->SmokeStrength);
  if(S.Visitor.IsValid()){const FVector Hand=S.Bench.InverseTransformPosition(S.Visitor->Body->GetSocketLocation(TEXT("hand_r")));UE_LOG(LogTemp,Display,TEXT("BenchReachReview: sample=%d reaching=%d hand_local=%s"),S.Phase,S.Visitor->bBenchReaching,*Hand.ToString());}
  if(bIgnition&&S.Visitor.IsValid())UE_LOG(LogTemp,Display,TEXT("BenchIgnitionReview: sample=%d ignited=%d visitor_local=%s"),S.Phase,S.Visitor->BenchesIgnited,*S.Bench.InverseTransformPosition(S.Visitor->GetActorLocation()).ToString());
  FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);if(Folder.IsEmpty())Folder=FPaths::ProjectSavedDir()/TEXT("BenchFireReview");IFileManager::Get().MakeDirectory(*Folder,true);
  FScreenshotRequest::RequestScreenshot(Folder/FString::Printf(TEXT("fire-%d.png"),S.Phase),false,false);++S.Phase;
 }
 if(S.Clock>6.5f&&S.Phase==4){S.Done=true;UE_LOG(LogTemp,Display,TEXT("BenchFireReview: captured=3 expired=%d"),!S.Fire.IsValid());PC->ConsoleCommand(TEXT("quit"));}
 if(S.Phase==0&&S.Clock>15){S.Done=true;UE_LOG(LogTemp,Error,TEXT("BenchFireReview: no bench available"));PC->ConsoleCommand(TEXT("quit"));}
#endif
}
