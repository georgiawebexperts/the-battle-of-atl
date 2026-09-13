#include "BattleBike.h"
#include "BattleRider.h"
#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"

bool ABattleBike::EnterLake(const FVector& Impact,const FVector& Bank){
 auto* PC=Cast<APlayerController>(GetController());if(!PC||bParked||RiderHealth<=0)return false;
 APiedmontWaterHazard* Lake=nullptr;
 for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->ContainsBike(Impact)){Lake=*It;break;}
 if(!Lake)return false;
 FCollisionQueryParams Query(SCENE_QUERY_STAT(BattleLakeEntry),false,this);
 for(TActorIterator<AActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("RideWater")))Query.AddIgnoredActor(*It);
 FVector Park=Bank;bool BankFound=false;
 auto SafeBank=[&](FVector Point,FVector& Result){
  FVector Probe=Point;Probe.Z=Lake->GetActorLocation().Z+1;if(Lake->ContainsBike(Probe))return false;
  FHitResult Ground;if(!GetWorld()->LineTraceSingleByChannel(Ground,Point+FVector(0,0,1000),Point-FVector(0,0,2000),ECC_Visibility,Query)||Ground.ImpactNormal.Z<.65f||Ground.ImpactPoint.Z<Lake->GetActorLocation().Z-5)return false;
  Result=Ground.ImpactPoint+FVector(0,0,98);
  return !GetWorld()->OverlapBlockingTestByChannel(Result,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,96),Query);
 };
 if(Ride->bHasDryLocation&&FVector::Dist2D(Bank,Impact)<600)BankFound=SafeBank(Bank,Park);
 if(!BankFound){
  float Best=MAX_flt;
  for(const auto* Ring:{&Lake->Polygon,&Lake->IslandPolygon})for(int I=0;I<Ring->Num();I++){
   const FVector A=Lake->GetActorTransform().TransformPosition((*Ring)[I]),B=Lake->GetActorTransform().TransformPosition((*Ring)[(I+1)%Ring->Num()]);
   const FVector Edge=B-A;const float T=FMath::Clamp(FVector::DotProduct(Impact-A,Edge)/FMath::Max(1.,Edge.SizeSquared()),0.,1.);const FVector Base=A+Edge*T;const FVector Along=Edge.GetSafeNormal2D(),Normal(-Along.Y,Along.X,0);
   for(float Offset:{200.f,-200.f,400.f,-400.f,800.f,-800.f}){FVector Candidate;if(SafeBank(Base+Normal*Offset,Candidate)){const float Distance=FVector::DistSquared2D(Candidate,Impact);if(Distance<Best){Best=Distance;Park=Candidate;BankFound=true;}}}
  }
 }
 if(!BankFound)return false;
 FVector Entry;bool Found=false;
 const FVector Forward=GetActorForwardVector();
 for(float Distance:{120.f,240.f,360.f,0.f}){
  Entry=Impact+Forward*Distance;Entry.Z=Lake->GetActorLocation().Z+35;
  if(Lake->ContainsBike(Entry)&&!GetWorld()->OverlapBlockingTestByChannel(Entry,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,35),Query)){Found=true;break;}
 }
 if(!Found)for(float Radius:{120.f,360.f,720.f}){for(int I=0;I<16;I++){const float Angle=I*PI/8;Entry=Impact+FVector(FMath::Cos(Angle)*Radius,FMath::Sin(Angle)*Radius,0);Entry.Z=Lake->GetActorLocation().Z+35;if(Lake->ContainsBike(Entry)&&!GetWorld()->OverlapBlockingTestByChannel(Entry,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,35),Query)){Found=true;break;}}if(Found)break;}
 if(!Found)return false;
 FActorSpawnParameters Spawn;Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
 auto* Person=GetWorld()->SpawnActor<ABattleRider>(Entry,GetActorRotation(),Spawn);if(!Person)return false;
 Person->BeginSurfaceSwimming(Lake->GetActorLocation().Z);
 RideImpact(1.8f,true);
 SetActorLocation(Park,false,nullptr,ETeleportType::TeleportPhysics);
 Ride->BoostRemaining=0;Ride->Speed=Ride->Pedal=Ride->Steer=Ride->Brake=0;Ride->StopMovementImmediately();Ride->DisableMovement();
 bParked=true;Visual->SetRelativeRotation(FRotator::ZeroRotator);Rider->SetVisibility(false,true);ReloadTimer=0;LeanAngle=0;Ride->SmoothedSteer=0;
 Person->ParkedBike=this;Person->Health=RiderHealth;Person->RestoreLoadout();Person->GetCapsuleComponent()->IgnoreActorWhenMoving(this,true);
 PC->Possess(Person);PC->SetControlRotation(GetActorRotation());
#if !UE_BUILD_SHIPPING
 for(float Offset:{-60.f,0.f,60.f}){
  const FVector Point=Park+GetActorForwardVector()*Offset;FHitResult Simple,Complex;
  Query.bTraceComplex=false;const bool S=GetWorld()->LineTraceSingleByChannel(Simple,Point+FVector(0,0,200),Point-FVector(0,0,400),ECC_Visibility,Query);
  Query.bTraceComplex=true;const bool C=GetWorld()->LineTraceSingleByChannel(Complex,Point+FVector(0,0,200),Point-FVector(0,0,400),ECC_Visibility,Query);
  UE_LOG(LogTemp,Display,TEXT("BattleBankSupport: offset=%.0f simple=%d z=%.2f actor=%s complex=%d z=%.2f actor=%s tire_z=%.2f"),Offset,S,Simple.ImpactPoint.Z,Simple.GetActor()?*Simple.GetActor()->GetName():TEXT("none"),C,Complex.ImpactPoint.Z,Complex.GetActor()?*Complex.GetActor()->GetName():TEXT("none"),Park.Z-96);
 }
#endif
 UE_LOG(LogTemp,Display,TEXT("BattleLakeEntry: swimmer=%s bank=%s"),*Entry.ToString(),*Park.ToString());
 return true;
}
