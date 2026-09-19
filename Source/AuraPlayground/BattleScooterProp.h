#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

/**
 * A stand-up scooter, built from primitives.
 *
 * Why this exists: the prop the game called a scooter was
 * `/Game/BeltLineGlide/BP_ScooterRider`, whose only visible part is an
 * untextured engine cylinder. Every "scooter" in the project — the moving
 * traffic, the crash wreck, the incident scene — is that cylinder, which is why
 * Elliott rode to Krog Street Tunnel and asked where the scooters were. The one
 * place a real scooter existed was `ABattleScooterScene`, authored part by part;
 * this is that geometry, shared, so the crash and the tunnel can use it too.
 *
 * Local space: +X is forward, the deck sits at the anchor's origin and the
 * wheels hang 18 cm below it, so a scooter standing on its wheels wants its
 * anchor 18 cm above the pavement and one lying on its side wants about 11.
 */
inline int32 BuildBattleScooter(AActor* Owner,USceneComponent* Attach,const FVector& Spot,const FRotator& Rotation,int32 ColourVariant=0,bool bCollision=false)
{
 if(!Owner||!Attach)return 0;
 // Names have to be unique per actor, not per scooter: several scooters share
 // one owner and a repeated FName would make the engine rename the parts.
 static int32 Scaffold=0;
 UStaticMesh* Cube=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));
 UStaticMesh* Cylinder=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 UMaterialInterface* Metal=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Environment/KrogIncident/M_ScooterMetal.M_ScooterMetal"));
 UMaterialInterface* Trim=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Environment/KrogIncident/M_ScooterTrim.M_ScooterTrim"));
 // One path, the one that exists. The first attempt used to name
 // /Game/BattleForTheA/Materials/M_Rubber, which is not in the project, so every
 // scooter logged a failed load and fell through to this one.
 UMaterialInterface* Rubber=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BeltLineGlide/Materials/M_Rubber.M_Rubber"));
 if(!Cube||!Cylinder)return 0;
 auto* Anchor=NewObject<USceneComponent>(Owner,*FString::Printf(TEXT("ScooterAnchor%d"),++Scaffold));
 Anchor->SetupAttachment(Attach);
 Anchor->RegisterComponent();
 Anchor->SetWorldLocationAndRotation(Spot,Rotation);
 // Four colours for the deck, so a heap of them is not four identical props.
 static const FLinearColor DeckColours[]={FLinearColor(.72f,.12f,.09f),FLinearColor(.10f,.32f,.62f),FLinearColor(.92f,.62f,.06f),FLinearColor(.16f,.55f,.30f)};
 UMaterialInstanceDynamic* DeckMat=Trim?UMaterialInstanceDynamic::Create(Trim,Owner):nullptr;
 if(DeckMat){
  DeckMat->SetVectorParameterValue(TEXT("Color"),DeckColours[((ColourVariant%4)+4)%4]);
  DeckMat->SetVectorParameterValue(TEXT("BaseColor"),DeckColours[((ColourVariant%4)+4)%4]);
 }
 int32 Parts=0;
 auto Part=[&](const TCHAR* Name,const FVector& Local,const FVector& Size,bool bRound,const FRotator& Rot,UMaterialInterface* Mat){
  auto* C=NewObject<UStaticMeshComponent>(Owner,*FString::Printf(TEXT("Scooter_%s_%d"),Name,++Scaffold));
  C->SetupAttachment(Anchor);
  C->SetStaticMesh(bRound?Cylinder:Cube);
  C->SetRelativeLocation(Local);
  C->SetRelativeRotation(Rot);
  // The primitives are 100 cm; Size is given the way the incident scene gives it.
  C->SetRelativeScale3D(Size/100.f);
  C->SetCollisionEnabled(bCollision?ECollisionEnabled::QueryAndPhysics:ECollisionEnabled::NoCollision);
  if(bCollision)C->SetCollisionProfileName(TEXT("BlockAll"));
  C->SetCanEverAffectNavigation(false);
  C->RegisterComponent();
  if(Mat)C->SetMaterial(0,Mat);
  ++Parts;
 };
 Part(TEXT("Deck"),FVector::ZeroVector,FVector(110,20,7),false,FRotator::ZeroRotator,DeckMat?DeckMat:Trim);
 Part(TEXT("GripTape"),FVector(0,0,4),FVector(92,16,1),false,FRotator::ZeroRotator,Rubber);
 Part(TEXT("Stem"),FVector(48,0,50),FVector(5,5,100),true,FRotator::ZeroRotator,Metal);
 Part(TEXT("StemBand"),FVector(48,0,82),FVector(5.5f,5.5f,8),true,FRotator::ZeroRotator,Trim);
 Part(TEXT("Handle"),FVector(48,0,100),FVector(4,4,48),true,FRotator(0,0,90),Metal);
 for(int32 Side=-1;Side<=1;Side+=2){
  Part(Side<0?TEXT("LeftGrip"):TEXT("RightGrip"),FVector(48,Side*18.f,100),FVector(5,5,12),true,FRotator(0,0,90),Rubber);
  Part(Side<0?TEXT("RearWheel"):TEXT("FrontWheel"),FVector(Side*53.f,0,-6),FVector(24,24,6),true,FRotator(0,0,90),Rubber);
  Part(Side<0?TEXT("RearHub"):TEXT("FrontHub"),FVector(Side*53.f,0,-6),FVector(8,8,7),true,FRotator(0,0,90),Metal);
 }
 return Parts;
}
