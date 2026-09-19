#include "BattleBike.h"
#include "BattleDance.h"
#include "BattlePicnic.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
#include "Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
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
 *   -BattlePropProbeOrigin=-2000,6000,0
 *                               probe from a fixed spot instead of the rider, so
 *                               a sighting photographed from somewhere else can
 *                               be named without riding back to it
 *
 * Every mesh part prints its material, because an untextured pale slab in a
 * screenshot is usually a component whose material failed to load and fell back
 * to the engine default - which is a name, if the probe prints the material.
 * An instanced component prints each instance's transform for the same reason:
 * "Mesh x12" with no positions is exactly the blind spot that left a floating
 * panel unnamed for two days.
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
 FVector Origin=Pawn->GetActorLocation();
 FString OriginArg;
 if(FParse::Value(FCommandLine::Get(),TEXT("BattlePropProbeOrigin="),OriginArg)){
  TArray<FString> Parts;OriginArg.ParseIntoArray(Parts,TEXT(","),true);
  if(Parts.Num()>=3)Origin=FVector(FCString::Atof(*Parts[0]),FCString::Atof(*Parts[1]),FCString::Atof(*Parts[2]));
 }
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
    if(Instanced->GetStaticMesh()){
     const UMaterialInterface* Mat=Instanced->GetMaterial(0);
     FString Where;
     const int32 Shown=FMath::Min(Instanced->GetInstanceCount(),8);
     for(int32 I=0;I<Shown;++I){FTransform T;if(Instanced->GetInstanceTransform(I,T,true))Where+=FString::Printf(TEXT("%s%s"),I?TEXT(" "):TEXT(""),*T.GetLocation().ToCompactString());}
     Meshes.Add(FString::Printf(TEXT("%s x%d mat=%s at %s%s"),*Instanced->GetStaticMesh()->GetName(),Instanced->GetInstanceCount(),Mat?*Mat->GetName():TEXT("NONE"),Where.IsEmpty()?TEXT("?"):*Where,Instanced->GetInstanceCount()>Shown?TEXT("..."):TEXT("")));
    }
   }else if(auto* Static=Cast<UStaticMeshComponent>(Component)){
    if(Static->GetStaticMesh()){
     const UMaterialInterface* Mat=Static->GetMaterial(0);
     Meshes.Add(FString::Printf(TEXT("%s@%s mat=%s%s"),*Static->GetStaticMesh()->GetName(),
      *Static->GetComponentLocation().ToCompactString(),Mat?*Mat->GetName():TEXT("NONE"),Static->IsVisible()?TEXT(""):TEXT("(hidden)")));
    }
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

/**
 * Names a single unexplained sighting instead of a whole area.
 *
 * The prop probe above reports an instanced mesh as "MeshName xN" with no
 * position, so a stray panel that lives as one instance inside an instanced
 * component cannot be found by reading it. This probe anchors on the PARK DJ
 * disc (falling back to the nearest picnic group, then the rider), walks every
 * mesh part and every instanced instance one at a time, and reports each part
 * with its material and the height of its underside above the ground beneath
 * it. A part that is flat, wide and floating is flagged with stars.
 *
 *   -BattlePanelProbe          search 30 m around the DJ
 *   -BattlePanelProbe=8000     search 80 m instead
 */
void TickBattlePanelProbe(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 static bool Done=false;
 static float Clock=0;
 if(Done||PC->GetWorld()->GetTimeSeconds()<8)return;
 Clock+=Dt;
 if(Clock<1.f)return;
 UWorld* World=PC->GetWorld();
 FVector Anchor=FVector::ZeroVector;
 FString AnchorName=TEXT("none");
 TActorIterator<ABattleDanceCircle> Dance(World);
 if(Dance){Anchor=Dance->GetActorLocation();AnchorName=Dance->GetName();}
 else{TActorIterator<ABattlePicnicGroup> Picnic(World);if(Picnic){Anchor=Picnic->GetActorLocation();AnchorName=Picnic->GetName();}}
 if(AnchorName==TEXT("none")&&PC->GetPawn()){Anchor=PC->GetPawn()->GetActorLocation();AnchorName=TEXT("rider");}
 float Radius=3000.f;
 FParse::Value(FCommandLine::Get(),TEXT("BattlePanelProbe="),Radius);
 // -BattlePanelProbeAll prints every mesh part in the radius, not only the flat
 // and floating ones, so a sighting that matches none of those rules still gets
 // an inventory to read.
 const bool bAll=FParse::Param(FCommandLine::Get(),TEXT("BattlePanelProbeAll"));

 struct FRow{float Distance;bool Flag;FString Text;};
 TArray<FRow> Rows;
 const FCollisionQueryParams Query(SCENE_QUERY_STAT(BattlePanelProbe),false,PC->GetPawn());
 auto Material=[&](UPrimitiveComponent* C){return C->GetMaterial(0)?C->GetMaterial(0)->GetName():FString(TEXT("none"));};
 // One part at a time: its world transform, its size, and how far its underside
 // sits above whatever is directly below it.
 auto Report=[&](AActor* Actor,UPrimitiveComponent* C,UStaticMesh* Mesh,const FTransform& X,const FString& Part){
  if(!Mesh||!C->IsVisible())return;
  const FVector Extent=Mesh->GetBounds().BoxExtent*X.GetScale3D().GetAbs();
  const FVector Centre=X.GetLocation();
  const float Distance=FVector::Dist(Anchor,Centre);
  if(Distance>Radius)return;
  const float Flat=FMath::Min3(Extent.X,Extent.Y,Extent.Z);
  const float Wide=FMath::Max3(Extent.X,Extent.Y,Extent.Z);
  FHitResult Hit;
  const bool bGround=World->LineTraceSingleByChannel(Hit,Centre+FVector(0,0,40),Centre-FVector(0,0,6000),ECC_Visibility,Query);
  const float Underside=Centre.Z-Extent.Z;
  const float Height=bGround?Underside-Hit.ImpactPoint.Z:-9999.f;
  const bool bPanel=Flat<30.f&&Wide>90.f;
  const bool bFloat=bGround&&Height>60.f;
  const FString Mat=Material(C);
  if(!bAll&&!bPanel&&!bFloat)return;
  const FString Text=FString::Printf(TEXT("%.0fcm %s | %s | %s.%s | mesh=%s | mat=%s | size=%.0fx%.0fx%.0f | z=%.1f | over_ground=%.1f %s"),
   Distance,*Actor->GetClass()->GetName(),*Actor->GetName(),*C->GetName(),*Part,*Mesh->GetName(),*Mat,
   Extent.X*2,Extent.Y*2,Extent.Z*2,Centre.Z,Height,(bPanel&&bFloat)?TEXT("*** FLAT WIDE AND FLOATING"):(bPanel?TEXT("(flat and wide, grounded)"):(bFloat?TEXT("(floating, not flat)"):TEXT("(listed)"))));
  Rows.Add({Distance,bPanel&&bFloat,Text});
 };
 for(TActorIterator<AActor> It(World);It;++It){
  AActor* Actor=*It;
  if(Actor==PC->GetPawn()||Actor->IsA(ALandscape::StaticClass()))continue;
  // Sky and light plumbing is normally noise, but the skyline is a cluster of
  // pale instanced cubes and it was the one pale flat geometry in the world that
  // this probe could never report - which is the shape of the floating panel
  // sighting. -BattlePanelProbeSky includes it.
  if(Actor->GetClass()->GetName().Contains(TEXT("Sky"))&&!FParse::Param(FCommandLine::Get(),TEXT("BattlePanelProbeSky")))continue;
  for(UActorComponent* Component:Actor->GetComponents()){
   if(auto* Instanced=Cast<UInstancedStaticMeshComponent>(Component)){
    UStaticMesh* Mesh=Instanced->GetStaticMesh();
    if(!Mesh)continue;
    for(int32 I=0;I<Instanced->GetInstanceCount();I++){
     FTransform X;
     if(!Instanced->GetInstanceTransform(I,X,true))continue;
     Report(Actor,Instanced,Mesh,X,FString::Printf(TEXT("instance %d of %d"),I,Instanced->GetInstanceCount()));
    }
   }else if(auto* Static=Cast<UStaticMeshComponent>(Component)){
    if(Static->GetStaticMesh())Report(Actor,Static,Static->GetStaticMesh(),Static->GetComponentTransform(),TEXT("component"));
   }
  }
 }
 Rows.Sort([](const FRow& A,const FRow& B){return A.Flag==B.Flag?A.Distance<B.Distance:A.Flag;});
 UE_LOG(LogTemp,Display,TEXT("PanelProbe: anchor=%s at %s radius=%.0f candidates=%d"),*AnchorName,*Anchor.ToString(),Radius,Rows.Num());
 for(const FRow& Row:Rows)UE_LOG(LogTemp,Display,TEXT("PanelProbe: %s"),*Row.Text);
 Done=true;
 UKismetSystemLibrary::QuitGame(PC,PC,EQuitPreference::Quit,false);
#endif
}
