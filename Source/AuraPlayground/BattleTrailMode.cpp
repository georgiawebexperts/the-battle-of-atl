#include "BattleTrailMode.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "PiedmontPathSpline.h"

void UBattleTrailMode::ApplyToWorld(UWorld* World,bool bRealistic){
 if(!World)return;
 for(TActorIterator<AActor> It(World);It;++It){
  AActor* Actor=*It;
  if(Actor->ActorHasTag(TEXT("BattleTrailArcade"))){
   Actor->SetActorHiddenInGame(bRealistic);
   Actor->SetActorEnableCollision(!bRealistic);
  }else if(Actor->ActorHasTag(TEXT("BattleTrailRealistic"))){
   Actor->SetActorHiddenInGame(!bRealistic);
   Actor->SetActorEnableCollision(bRealistic);
  }
 }
 for(TActorIterator<APiedmontPathSpline> It(World);It;++It)
  if(It->ActorHasTag(TEXT("BattleTrailWidth")))It->WidthCm=bRealistic?RealisticWidthCm:ArcadeWidthCm;
}

void UBattleTrailMode::Apply(UObject* WorldContextObject,bool bRealistic){
 if(!GEngine)return;
 ApplyToWorld(GEngine->GetWorldFromContextObject(WorldContextObject,EGetWorldErrorMode::ReturnNull),bRealistic);
}
