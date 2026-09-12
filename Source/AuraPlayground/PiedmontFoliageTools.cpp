#include "PiedmontWorldTools.h"
#if WITH_EDITOR
#include "Editor.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGNode.h"
#include "PCGDataAsset.h"
#include "Data/PCGPointData.h"
#include "Elements/IO/PCGLoadAssetElement.h"
#include "Elements/PCGStaticMeshSpawner.h"
#include "MeshSelectors/PCGMeshSelectorWeighted.h"
#include "Components/SceneComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/Package.h"
#endif

AActor* UPiedmontWorldTools::CreateParkFoliage(const TArray<FTransform>& Stations,UStaticMesh* Mesh){
#if WITH_EDITOR
 UWorld* World=GEditor?GEditor->GetEditorWorldContext().World():nullptr;
 if(!World||!Mesh||Stations.Num()==0)return nullptr;
 const TCHAR* DataPath=TEXT("/Game/BattleForTheA/Environment/Park/DA_TreeStations");
 const TCHAR* GraphPath=TEXT("/Game/BattleForTheA/Environment/Park/PCG_ParkCanopy");
 // A prior generated asset must be reviewed rather than silently overwritten.
 if(LoadObject<UPCGGraph>(nullptr,TEXT("/Game/BattleForTheA/Environment/Park/PCG_ParkCanopy.PCG_ParkCanopy")))return nullptr;
 auto* Data=NewObject<UPCGDataAsset>(CreatePackage(DataPath),TEXT("DA_TreeStations"),RF_Public|RF_Standalone);
 auto* Points=NewObject<UPCGPointData>(Data,TEXT("GeographicTreeStations"),RF_Public);
 auto& Values=Points->GetMutablePoints();Values.Reserve(Stations.Num());
 for(int32 Index=0;Index<Stations.Num();++Index){FPCGPoint& Point=Values.AddDefaulted_GetRef();Point.Transform=Stations[Index];Point.Density=1;Point.Seed=Index+2701;}
 FPCGTaggedData& Tagged=Data->Data.TaggedData.AddDefaulted_GetRef();Tagged.Data=Points;Tagged.Pin=PCGPinConstants::DefaultOutputLabel;
 Data->Name=TEXT("Piedmont mapped and authored tree stations");Data->MarkPackageDirty();
 auto* Graph=NewObject<UPCGGraph>(CreatePackage(GraphPath),TEXT("PCG_ParkCanopy"),RF_Public|RF_Standalone);
 UPCGLoadDataAssetSettings* Load=nullptr;auto* LoadNode=Graph->AddNodeOfType(Load);
 Load->Asset=Data;Load->bSynchronousLoad=true;Load->UpdateFromData();LoadNode->UpdateAfterSettingsChangeDuringCreation();
 UPCGStaticMeshSpawnerSettings* Spawn=nullptr;auto* SpawnNode=Graph->AddNodeOfType(Spawn);
 Spawn->SetMeshSelectorType(UPCGMeshSelectorWeighted::StaticClass());
 auto* Selector=CastChecked<UPCGMeshSelectorWeighted>(Spawn->MeshSelectorParameters);
 FPCGMeshSelectorWeightedEntry Entry(Mesh,1);
 // Canopy is visual: collision must be separately authored for trunks, never
 // one enormous convex hull that blocks paths underneath branches.
 Entry.Descriptor.BodyInstance.SetCollisionProfileName(TEXT("NoCollision"));
 Entry.Descriptor.InstanceStartCullDistance=28000;Entry.Descriptor.InstanceEndCullDistance=35000;
 Selector->MeshEntries.Add(Entry);
 Graph->AddEdge(LoadNode,PCGPinConstants::DefaultOutputLabel,SpawnNode,PCGPinConstants::DefaultInputLabel);Graph->MarkPackageDirty();
 auto* Actor=World->SpawnActor<AActor>();Actor->SetActorLabel(TEXT("Piedmont PCG canopy - interim Epic tree"));Actor->Tags.Add(TEXT("BattleParkCanopy"));
 FBox Bounds(ForceInit);for(const auto& Station:Stations)Bounds+=Station.GetLocation();Bounds=Bounds.ExpandBy(2000);
 auto* Root=NewObject<UBoxComponent>(Actor,TEXT("FoliageBounds"));Actor->SetRootComponent(Root);Actor->AddInstanceComponent(Root);Root->SetBoxExtent(Bounds.GetExtent());Root->SetRelativeLocation(Bounds.GetCenter());Root->SetCollisionEnabled(ECollisionEnabled::NoCollision);Root->SetCanEverAffectNavigation(false);Root->SetHiddenInGame(true);Root->RegisterComponent();
 auto* Component=NewObject<UPCGComponent>(Actor,TEXT("ParkCanopyPCG"));Actor->AddInstanceComponent(Component);Component->RegisterComponent();
 Component->GenerationTrigger=EPCGComponentGenerationTrigger::GenerateOnDemand;Component->SetGraph(Graph);Component->GenerateLocal(true);
 return Actor;
#else
 return nullptr;
#endif
}

// Independent review actor; never overwrites the saved geographic PCG assets.
AActor* UPiedmontWorldTools::CreateFoliageReviewInstances(const TArray<FTransform>& Stations,UStaticMesh* Mesh){
#if WITH_EDITOR
 auto* World=GEditor?GEditor->GetEditorWorldContext().World():nullptr;
 if(!World||!Mesh||Stations.IsEmpty())return nullptr;
 auto* Actor=World->SpawnActor<AActor>();Actor->SetActorLabel(TEXT("Detailed canopy review"));Actor->Tags.Add(TEXT("DetailedCanopyReview"));
 auto* Instances=NewObject<UHierarchicalInstancedStaticMeshComponent>(Actor,TEXT("TreeInstances"));Actor->SetRootComponent(Instances);Actor->AddInstanceComponent(Instances);
 Instances->SetStaticMesh(Mesh);Instances->SetCollisionEnabled(ECollisionEnabled::NoCollision);Instances->SetCanEverAffectNavigation(false);Instances->SetCullDistances(28000,35000);Instances->RegisterComponent();
 for(const auto& Transform:Stations)Instances->AddInstance(Transform,true);
 return Actor;
#else
 return nullptr;
#endif
}
