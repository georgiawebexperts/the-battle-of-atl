#include "PiedmontWorldTools.h"
#if WITH_EDITOR
#include "Editor.h"
#include "MeshDescription.h"
#include "PhysicsEngine/BodySetup.h"
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

AActor* UPiedmontWorldTools::CreateParkFoliage(const TArray<FTransform>& Stations,UStaticMesh* Mesh){return CreateParkFoliageAtPath(Stations,Mesh,TEXT("/Game/BattleForTheA/Environment/Park"));}

AActor* UPiedmontWorldTools::CreateParkFoliageAtPath(const TArray<FTransform>& Stations,UStaticMesh* Mesh,const FString& AssetFolder){
#if WITH_EDITOR
 UWorld* World=GEditor?GEditor->GetEditorWorldContext().World():nullptr;
 if(!World||!Mesh||Stations.Num()==0)return nullptr;
 if(!AssetFolder.StartsWith(TEXT("/Game/BattleForTheA/Environment/Park")))return nullptr;
 const FString DataPath=AssetFolder/TEXT("DA_TreeStations");
 const FString GraphPath=AssetFolder/TEXT("PCG_ParkCanopy");
 // A prior generated asset must be reviewed rather than silently overwritten.
 if(LoadObject<UPCGGraph>(nullptr,*(GraphPath+TEXT(".PCG_ParkCanopy"))))return nullptr;
 auto* Data=NewObject<UPCGDataAsset>(CreatePackage(*DataPath),TEXT("DA_TreeStations"),RF_Public|RF_Standalone);
 auto* Points=NewObject<UPCGPointData>(Data,TEXT("GeographicTreeStations"),RF_Public);
 auto& Values=Points->GetMutablePoints();Values.Reserve(Stations.Num());
 for(int32 Index=0;Index<Stations.Num();++Index){FPCGPoint& Point=Values.AddDefaulted_GetRef();Point.Transform=Stations[Index];Point.Density=1;Point.Seed=Index+2701;}
 FPCGTaggedData& Tagged=Data->Data.TaggedData.AddDefaulted_GetRef();Tagged.Data=Points;Tagged.Pin=PCGPinConstants::DefaultOutputLabel;
 Data->Name=TEXT("Piedmont mapped and authored tree stations");Data->MarkPackageDirty();
 auto* Graph=NewObject<UPCGGraph>(CreatePackage(*GraphPath),TEXT("PCG_ParkCanopy"),RF_Public|RF_Standalone);
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
 Instances->BuildTreeIfOutdated(false,true);
 return Actor;
#else
 return nullptr;
#endif
}

float UPiedmontWorldTools::GetTreeLowGeometryRadius(UStaticMesh* Mesh,float LocalHeightAboveBottom){
#if WITH_EDITOR
 if(!Mesh||LocalHeightAboveBottom<=0)return -1;
 const FMeshDescription* Description=Mesh->GetMeshDescription(0);if(!Description)return -1;
 const auto Positions=Description->GetVertexPositions();const auto Bounds=Mesh->GetBounds();
 const float Cut=Bounds.Origin.Z-Bounds.BoxExtent.Z+LocalHeightAboveBottom;float RadiusSquared=0;
 // Include every triangle that enters the height band, not just its low vertices.
 // Including its upper vertices overestimates clearance instead of missing a branch.
 for(const FTriangleID Id:Description->Triangles().GetElementIDs()){
  const auto Vertices=Description->GetTriangleVertices(Id);bool InBand=false;
  for(const auto Vertex:Vertices)InBand|=Positions[Vertex].Z<=Cut;
  if(InBand)for(const auto Vertex:Vertices){const auto P=Positions[Vertex];RadiusSquared=FMath::Max(RadiusSquared,P.X*P.X+P.Y*P.Y);}
 }
 return FMath::Sqrt(RadiusSquared);
#else
 return -1;
#endif
}

bool UPiedmontWorldTools::EnableParkTrunkCollision(AActor* Actor){
#if WITH_EDITOR
 if(!Actor||!Actor->ActorHasTag(TEXT("BattleMixedCanopy")))return false;
 auto* Component=Actor->FindComponentByClass<UPCGComponent>();auto* Graph=Component?Component->GetGraph():nullptr;if(!Graph)return false;
 bool Updated=false;
 for(auto* Node:Graph->GetNodes())if(auto* Settings=Cast<UPCGStaticMeshSpawnerSettings>(Node->GetSettings())){
  auto* Selector=Cast<UPCGMeshSelectorWeighted>(Settings->MeshSelectorParameters);if(!Selector)return false;
  for(auto& Entry:Selector->MeshEntries)Entry.Descriptor.BodyInstance.SetCollisionProfileName(TEXT("BlockAll"));Updated=true;
 }
 if(!Updated)return false;
 TArray<UInstancedStaticMeshComponent*> Instances;Actor->GetComponents(Instances);
 for(auto* Instance:Instances){UStaticMesh* Mesh=Instance->GetStaticMesh();auto* Body=Mesh?Mesh->GetBodySetup():nullptr;
  if(!Body||Body->AggGeom.SphylElems.Num()!=1||Body->AggGeom.GetElementCount()!=1||Body->CollisionTraceFlag!=CTF_UseSimpleAsComplex)return false;
 }
 Graph->MarkPackageDirty();Actor->Tags.AddUnique(TEXT("RideTree"));
 for(auto* Instance:Instances){Instance->SetCollisionProfileName(TEXT("BlockAll"));Instance->RecreatePhysicsState();}
 Actor->MarkPackageDirty();return true;
#else
 return false;
#endif
}

AActor* UPiedmontWorldTools::CreateGroundNavigationTiles(const TArray<FTransform>& Tiles){
#if WITH_EDITOR
 auto* World=GEditor?GEditor->GetEditorWorldContext().World():nullptr;
 if(!World||(World->GetName()!=TEXT("PiedmontScooterReview")&&World->GetName()!=TEXT("PiedmontWorld"))||Tiles.IsEmpty())return nullptr;
 auto* Mesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube"));if(!Mesh)return nullptr;
 auto* Actor=World->SpawnActor<AActor>();Actor->SetActorLabel(TEXT("Scooter grass navigation only"));Actor->Tags.Add(TEXT("RideNavOnly"));Actor->Tags.Add(TEXT("ScooterNavigation"));Actor->SetActorHiddenInGame(true);
 auto* Instances=NewObject<UHierarchicalInstancedStaticMeshComponent>(Actor,TEXT("GroundNavigationTiles"));Actor->SetRootComponent(Instances);Actor->AddInstanceComponent(Instances);
 Instances->SetStaticMesh(Mesh);Instances->SetCollisionEnabled(ECollisionEnabled::NoCollision);Instances->SetCustomNavigableGeometry(EHasCustomNavigableGeometry::EvenIfNotCollidable);Instances->SetCanEverAffectNavigation(true);Instances->SetVisibility(false);Instances->RegisterComponent();
 for(const auto& T:Tiles)Instances->AddInstance(T,true);Instances->BuildTreeIfOutdated(false,true);return Actor;
#else
 return nullptr;
#endif
}
