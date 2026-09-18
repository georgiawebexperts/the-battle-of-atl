#include "BattleBike.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
#include "Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

/**
 * Names every prop around the rider. Elliott keeps reporting a sighting ("the
 * logs") that the editor cannot explain, because the park furniture, bins and
 * pickups are spawned at runtime. This dumps what is actually there, nearest
 * first, so a sighting can be turned into a class name instead of a guess.
 *
 *   -BattlePropProbe            list actors within 120 m of the rider
 *   -BattlePropProbe=6000       use a 60 m radius instead
 */
void TickBattlePropProbe(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 static bool Done=false;
 static float Clock=0;
 if(Done||PC->GetWorld()->GetTimeSeconds()<6)return;
 Clock+=Dt;
 if(Clock<1.f)return;
 APawn* Pawn=PC->GetPawn();
 if(!Pawn)return;
 float Radius=12000.f;
 FParse::Value(FCommandLine::Get(),TEXT("BattlePropProbe="),Radius);
 const FVector Origin=Pawn->GetActorLocation();
 struct FRow{float Distance;FString Class;FString Label;FString Mesh;FVector Location;bool bHidden;};
 TArray<FRow> Rows;
 for(TActorIterator<AActor> It(PC->GetWorld());It;++It){
  AActor* Actor=*It;
  if(Actor==Pawn||Actor->IsA(ALandscape::StaticClass()))continue;
  const float Distance=FVector::Dist(Origin,Actor->GetActorLocation());
  if(Distance>Radius)continue;
  // Name every mesh part: a stray wheel or a misplaced frame shows up as one
  // part, not as the first mesh on the actor.
  TArray<FString> Meshes;
  for(auto* Component:Actor->GetComponents()){
   if(auto* Instanced=Cast<UInstancedStaticMeshComponent>(Component)){
    if(Instanced->GetStaticMesh())Meshes.Add(FString::Printf(TEXT("%s x%d"),*Instanced->GetStaticMesh()->GetName(),Instanced->GetInstanceCount()));
   }else if(auto* Static=Cast<UStaticMeshComponent>(Component)){
    if(Static->GetStaticMesh())Meshes.Add(FString::Printf(TEXT("%s@%s%s"),*Static->GetStaticMesh()->GetName(),
     *Static->GetComponentLocation().ToString(),Static->IsVisible()?TEXT(""):TEXT("(hidden)")));
   }
  }
  TArray<FString> TagNames;
  for(const FName& Tag:Actor->Tags)TagNames.Add(Tag.ToString());
  // Landscape, sky and other scene plumbing is not a prop.
  const FString Class=Actor->GetClass()->GetName();
  if(Class.Contains(TEXT("Sky"))||Class.Contains(TEXT("Light"))||Class.Contains(TEXT("Fog"))||Class.Contains(TEXT("Cloud"))||Class.Contains(TEXT("PostProcess")))continue;
  // GetActorLabel only exists in the editor; the packaged game uses the name.
#if WITH_EDITOR
  const FString Label=Actor->GetActorLabel();
#else
  const FString Label=Actor->GetName();
#endif
  Rows.Add({Distance,Class,Label,
   FString::Printf(TEXT("[%s] %s"),*FString::Join(TagNames,TEXT(",")),*FString::Join(Meshes,TEXT(" | "))),
   Actor->GetActorLocation(),Actor->IsHidden()});
 }
 Rows.Sort([](const FRow& A,const FRow& B){return A.Distance<B.Distance;});
 UE_LOG(LogTemp,Display,TEXT("PropProbe: origin=%s radius=%.0f actors=%d"),*Origin.ToString(),Radius,Rows.Num());
 for(const FRow& Row:Rows){
  UE_LOG(LogTemp,Display,TEXT("PropProbe: %.0fcm %s | %s | %s | %s | hidden=%d"),
   Row.Distance,*Row.Class,*Row.Label,*Row.Mesh,*Row.Location.ToString(),Row.bHidden?1:0);
 }
 Done=true;
 UKismetSystemLibrary::QuitGame(PC,PC,EQuitPreference::Quit,false);
#endif
}
