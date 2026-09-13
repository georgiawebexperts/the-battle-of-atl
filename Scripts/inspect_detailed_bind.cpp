// Read-only comparison of FBX arm bind transforms and skin influence counts.
#include <fbxsdk.h>
#include <iostream>
#include <string>
int main(int argc,char**argv){if(argc!=2)return 2;auto*m=FbxManager::Create();m->SetIOSettings(FbxIOSettings::Create(m,IOSROOT));auto*s=FbxScene::Create(m,"bind");auto*i=FbxImporter::Create(m,"");if(!i->Initialize(argv[1],-1,m->GetIOSettings())||!i->Import(s))return 1;i->Destroy();
for(int n=0;n<s->GetNodeCount();n++){auto*node=s->GetNode(n);std::string name=node->GetName();if(node->GetSkeleton()){auto p=node->EvaluateGlobalTransform().GetT();std::cout<<"BONE "<<name<<" parent="<<(node->GetParent()?node->GetParent()->GetName():"none")<<" xyz="<<p[0]<<","<<p[1]<<","<<p[2]<<"\n";}auto*mesh=node->GetMesh();if(!mesh)continue;
for(int d=0;d<mesh->GetDeformerCount(FbxDeformer::eSkin);d++){auto*skin=(FbxSkin*)mesh->GetDeformer(d,FbxDeformer::eSkin);for(int c=0;c<skin->GetClusterCount();c++){auto*cl=skin->GetCluster(c);std::string name=cl->GetLink()->GetName();if(name.find("_l")==std::string::npos)continue;FbxAMatrix bind;cl->GetTransformLinkMatrix(bind);auto p=bind.GetT();double sum=0;for(int v=0;v<cl->GetControlPointIndicesCount();v++)sum+=cl->GetControlPointWeights()[v];std::cout<<"CLUSTER "<<name<<" count="<<cl->GetControlPointIndicesCount()<<" weight="<<sum<<" bind="<<p[0]<<","<<p[1]<<","<<p[2]<<"\n";}}}m->Destroy();}
