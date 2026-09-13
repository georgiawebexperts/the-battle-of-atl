// Extract City Sample arm geometry while retaining skinning, normals, materials and UVs.
#include <fbxsdk.h>
#include <vector>
#include <string>
#include <iostream>
bool Arm(FbxNode* Node){for(;Node;Node=Node->GetParent()){std::string N=Node->GetName();if(N=="upperarm_l"||N=="upperarm_r")return true;}return false;}
int main(int argc,char** argv){
 if(argc!=3)return 2;auto* M=FbxManager::Create();M->SetIOSettings(FbxIOSettings::Create(M,IOSROOT));auto* S=FbxScene::Create(M,"FPSArms");auto* I=FbxImporter::Create(M,"");if(!I->Initialize(argv[1],-1,M->GetIOSettings())||!I->Import(S))return 3;I->Destroy();
 int Kept=0,Removed=0;
 for(int N=0;N<S->GetNodeCount();++N){auto* Node=S->GetNode(N);auto* Mesh=Node->GetMesh();if(!Mesh)continue;std::vector<double> Weights(Mesh->GetControlPointsCount(),0),Totals(Mesh->GetControlPointsCount(),0);
  for(int D=0;D<Mesh->GetDeformerCount(FbxDeformer::eSkin);D++){auto* Skin=static_cast<FbxSkin*>(Mesh->GetDeformer(D,FbxDeformer::eSkin));for(int C=0;C<Skin->GetClusterCount();C++){auto* Cluster=Skin->GetCluster(C);for(int V=0;V<Cluster->GetControlPointIndicesCount();V++){int Id=Cluster->GetControlPointIndices()[V];double W=Cluster->GetControlPointWeights()[V];Totals[Id]+=W;if(Arm(Cluster->GetLink()))Weights[Id]+=W;}}}
  // FBX influences are not necessarily normalized. Raw thresholds delete palms/fingers.
  for(int V=0;V<Mesh->GetControlPointsCount();V++)if(Totals[V]>0)Weights[V]/=Totals[V];
  auto* Out=FbxMesh::Create(S,(std::string(Node->GetName())+"_Arms").c_str());Out->InitControlPoints(Mesh->GetControlPointsCount());for(int V=0;V<Mesh->GetControlPointsCount();V++)Out->SetControlPointAt(Mesh->GetControlPointAt(V),V);
  auto* Normals=Out->CreateElementNormal();Normals->SetMappingMode(FbxLayerElement::eByPolygonVertex);Normals->SetReferenceMode(FbxLayerElement::eDirect);
  auto* Materials=Out->CreateElementMaterial();Materials->SetMappingMode(FbxLayerElement::eByPolygon);Materials->SetReferenceMode(FbxLayerElement::eIndexToDirect);auto* OriginalMaterials=Mesh->GetElementMaterial();
  FbxStringList UVNames;Mesh->GetUVSetNames(UVNames);std::vector<FbxGeometryElementUV*> UVSets;
  for(int U=0;U<UVNames.GetCount();U++){auto* UV=Out->CreateElementUV(UVNames[U]);UV->SetMappingMode(FbxLayerElement::eByPolygonVertex);UV->SetReferenceMode(FbxLayerElement::eDirect);UVSets.push_back(UV);}
  auto* SourceColors=Mesh->GetElementVertexColor();FbxGeometryElementVertexColor* Colors=nullptr;
  std::cerr<<"uv_sets="<<UVNames.GetCount()<<" vertex_colors="<<(SourceColors?SourceColors->GetDirectArray().GetCount():0)<<"\n";
  if(SourceColors){Colors=Out->CreateElementVertexColor();Colors->SetMappingMode(FbxLayerElement::eByPolygonVertex);Colors->SetReferenceMode(FbxLayerElement::eDirect);}
  int LocalKept=0;
  for(int P=0;P<Mesh->GetPolygonCount();P++){bool Keep=true;for(int V=0;V<Mesh->GetPolygonSize(P);V++)if(Weights[Mesh->GetPolygonVertex(P,V)]<.5)Keep=false;if(!Keep){Removed++;continue;}
   int Material=0;if(OriginalMaterials)Material=OriginalMaterials->GetIndexArray().GetAt(OriginalMaterials->GetMappingMode()==FbxLayerElement::eAllSame?0:P);
   Out->BeginPolygon(Material);for(int V=0;V<Mesh->GetPolygonSize(P);V++){Out->AddPolygon(Mesh->GetPolygonVertex(P,V));FbxVector4 Normal;Mesh->GetPolygonVertexNormal(P,V,Normal);Normals->GetDirectArray().Add(Normal);
   if(Colors){int C=SourceColors->GetMappingMode()==FbxLayerElement::eByControlPoint?Mesh->GetPolygonVertex(P,V):SourceColors->GetMappingMode()==FbxLayerElement::eAllSame?0:Mesh->GetPolygonVertexIndex(P)+V;if(SourceColors->GetReferenceMode()!=FbxLayerElement::eDirect)C=SourceColors->GetIndexArray().GetAt(C);Colors->GetDirectArray().Add(SourceColors->GetDirectArray().GetAt(C));}for(int U=0;U<UVNames.GetCount();U++){FbxVector2 UV;bool Unmapped=false;if(!Mesh->GetPolygonVertexUV(P,V,UVNames[U],UV,Unmapped)||Unmapped)return 6;UVSets[U]->GetDirectArray().Add(UV);}}Out->EndPolygon();Kept++;LocalKept++;
  }
  for(int D=Mesh->GetDeformerCount()-1;D>=0;D--){auto* Def=Mesh->GetDeformer(D);Mesh->RemoveDeformer(D);Out->AddDeformer(Def);}
  Node->SetNodeAttribute(LocalKept?Out:nullptr);

 }
 std::cerr<<"kept="<<Kept<<" removed="<<Removed<<"\n";if(Kept<100)return 4;auto* E=FbxExporter::Create(M,"");if(!E->Initialize(argv[2],-1,M->GetIOSettings())||!E->Export(S))return 5;
 std::cout<<"{\"retained_arm_polygons\":"<<Kept<<",\"removed_body_polygons\":"<<Removed<<"}\n";E->Destroy();M->Destroy();return 0;
}
