#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleRoadTrafficDirector.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

void TickBattleConstructionAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;bool Started=false,Done=false;float Time=0;int Sweeps=0;TArray<TWeakObjectPtr<ABattleRoadCar>> Cars;};
 static FState S;if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}
 if(S.Done||PC->GetWorld()->GetTimeSeconds()<6)return;S.Time+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){S.Done=true;UE_LOG(LogTemp,Display,TEXT("BattleConstructionAudit: {\"passed\":%s,\"reason\":\"%s\",\"blocked_capsule_sweeps\":%d,\"cars\":%d}"),Pass?TEXT("true"):TEXT("false"),Reason,S.Sweeps,S.Cars.Num());PC->ConsoleCommand(TEXT("quit"));};
 if(S.Time>60){Finish(false,TEXT("Traffic timed out"));return;}
 if(!S.Started){
  S.Started=true;auto* Bike=Cast<ABattleBike>(PC->GetPawn());if(!Bike){Finish(false,TEXT("No bike"));return;}
  FString Text;TSharedPtr<FJsonObject> Data;
  if(!FFileHelper::LoadFileToString(Text,*(FPaths::ProjectDir()/TEXT("Tests/Results/2026-09-12-krog-construction-placement.json")))||!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text),Data)){Finish(false,TEXT("Placement missing"));return;}
  const auto* Rider=GetDefault<ABattleRider>();const auto* Capsule=Rider->GetCapsuleComponent();
  const FCollisionShape Shapes[]={Bike->Capsule->GetCollisionShape(),Capsule->GetCollisionShape(),FCollisionShape::MakeCapsule(Capsule->GetUnscaledCapsuleRadius(),Rider->GetCharacterMovement()->CrouchedHalfHeight)};
  auto Vec=[](const TArray<TSharedPtr<FJsonValue>>& A){return FVector(A[0]->AsNumber(),A[1]->AsNumber(),A[2]->AsNumber());};
  FCollisionObjectQueryParams Objects;Objects.AddObjectTypesToQuery(ECC_WorldStatic);FCollisionQueryParams Q(SCENE_QUERY_STAT(ConstructionCapsules),false);
  // Isolate the fence surface so a rising verge cannot mask or replace its hit.
  for(TActorIterator<AActor> It(PC->GetWorld());It;++It)if(!It->ActorHasTag(TEXT("KrogConstruction")))Q.AddIgnoredActor(*It);
  for(const auto& Entry:Data->GetArrayField(TEXT("closures"))){
   const auto Row=Entry->AsObject();const FVector D=Vec(Row->GetArrayField(TEXT("outward")));const auto Feet=Row->GetObjectField(TEXT("feet"));
   for(const auto& Pair:Feet->Values){
    const FVector Foot=Vec(Pair.Value->AsArray());
    for(const auto& Shape:Shapes)for(float Lift:{0.f,150.f,300.f}){
     const FVector Center=Foot+FVector(0,0,Shape.GetCapsuleHalfHeight()+2+Lift);FHitResult Hit;
     if(!PC->GetWorld()->SweepSingleByObjectType(Hit,Center-D*100,Center+D*100,FQuat::Identity,Objects,Shape,Q)||!Hit.GetActor()||!Hit.GetActor()->ActorHasTag(TEXT("KrogConstruction"))){UE_LOG(LogTemp,Display,TEXT("ConstructionMiss: offset=%s lift=%.1f center=%s"),*Pair.Key,Lift,*Center.ToString());Finish(false,TEXT("Capsule passed closure"));return;}++S.Sweeps;
    }
   }
  }
  for(TActorIterator<ABattleRoadCar> It(PC->GetWorld());It;++It)if(It->ActorHasTag(TEXT("AmbientRoadCar")))It->Destroy();
  for(TActorIterator<ABattleRoadTrafficDirector> It(PC->GetWorld());It;++It){
   It->SetActorTickEnabled(false);if(!It->ActorHasTag(TEXT("KrogTrafficReview")))continue;
   for(const auto& Lane:It->Lanes){auto* Car=PC->GetWorld()->SpawnActor<ABattleRoadCar>();Car->Route=Lane.Points;Car->Crossings=Lane.Crossings;Car->CruiseSpeed=500;if(!Car->StartRoute()){Finish(false,TEXT("Car start failed"));return;}S.Cars.Add(Car);}
  }
  if(S.Cars.Num()!=2){Finish(false,TEXT("Expected two Krog lanes"));return;}
 }
 bool Complete=true;for(auto Item:S.Cars){auto* Car=Item.Get();if(!Car||!Car->bGrounded){Finish(false,TEXT("Car lost ground"));return;}Complete&=Car->bRouteFinished;}
 if(Complete)Finish(true,TEXT("Capsules blocked through300cm lift; both traffic lanes finished"));
#endif
}
