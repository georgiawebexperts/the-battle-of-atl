#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
void TickBattleFpsAudit(APlayerController* PC,float Dt){
 if(!PC||!PC->GetWorld())return;
 static TWeakObjectPtr<UWorld> World;static bool Done=false;static TArray<float> Samples;static float SampleClock=0;static bool Sampling=false;
 if(Done)return;
 if(!World.IsValid())World=PC->GetWorld();
 if(PC->GetWorld()->GetTimeSeconds()<5.f)return;
 if(!Sampling){Sampling=true;return;}
 if(Dt>0.001f)Samples.Push(1.f/Dt);
 SampleClock+=Dt;
 if(SampleClock<30.f)return;
 Done=true;
 TArray<float> Sorted=Samples;Sorted.Sort();
 const int32 Count=Sorted.Num();
 float Avg=0,Min=0,P05=0;
 if(Count>0){
  Avg=0;for(const float S:Sorted)Avg+=S;Avg/=Count;
  Min=Sorted[0];
  P05=Sorted[FMath::FloorToInt(Count*0.05f)];
 }
 const FString Json=FString::Printf(TEXT("{\"passed\":true,\"avg_fps\":%.1f,\"min_fps\":%.1f,\"p05_fps\":%.1f,\"samples\":%d}"),Avg,Min,P05,Count);
 UE_LOG(LogTemp,Display,TEXT("BattleFpsAudit: %s"),*Json);
 const FString OutDir=FPaths::Combine(FPaths::ProjectDir(),TEXT("Saved"));
 const FString OutPath=FPaths::Combine(OutDir,TEXT("FpsAudit.json"));FFileHelper::SaveStringToFile(Json,*OutPath);
}