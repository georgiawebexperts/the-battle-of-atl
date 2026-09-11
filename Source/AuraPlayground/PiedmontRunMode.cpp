#include "PiedmontBike.h"
#include "PiedmontExplorer.h"
#include "PiedmontThreat.h"
#include "PiedmontPathSpline.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
void APiedmontRideMode::StartPlay(){Super::StartPlay();}
void APiedmontRideMode::Tick(float Dt){
 Super::Tick(Dt);auto* PC=UGameplayStatics::GetPlayerController(this,0);if(!PC)return;
 if(!bAnchored){
  RunBike=Cast<APiedmontBike>(PC->GetPawn());if(!RunBike)return;
  StartLocation=RunBike->GetActorLocation();StartRotation=RunBike->GetActorRotation();bAnchored=true;RestartRun();
 }
 if(bRunEnded){if(PC->WasInputKeyJustPressed(EKeys::Enter))RestartRun();return;}
 if(StartCountdown>0){StartCountdown=FMath::Max(0.f,StartCountdown-Dt);return;}
 TimeRemaining=FMath::Max(0.f,TimeRemaining-Dt);if(TimeRemaining<=0){EndRun(TEXT("Time expired"));return;}
 EncounterCountdown-=Dt;if(EncounterCountdown<=0){EncounterCountdown=300;ConsiderEncounter();}
 if(bObjectiveReady&&!bItemCollected&&PC->GetPawn()&&FVector::DistSquared(PC->GetPawn()->GetActorLocation(),ItemLocation)<FMath::Square(130.f)){
  FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(PiedmontPickup),false,PC->GetPawn());Q.AddIgnoredActor(ItemActor);
  if(!GetWorld()->LineTraceSingleByChannel(Hit,PC->GetPawn()->GetActorLocation(),ItemLocation,ECC_Visibility,Q)){
   bItemCollected=true;if(ItemActor)ItemActor->SetActorHiddenInGame(true);
  }
 }
}
void APiedmontRideMode::ChooseItem(){
 if(ItemActor){ItemActor->Destroy();ItemActor=nullptr;}
 TArray<FVector> Candidates;bool HasPaths=false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(PiedmontItemPlacement),false,RunBike);
 for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It){
  HasPaths=true;if(It->bBridge)continue;
  for(int I=0;I<It->Centerline->GetNumberOfSplinePoints();I+=3){
   const FVector P=It->Centerline->GetLocationAtSplinePoint(I,ESplineCoordinateSpace::World);
   if(FVector::DistSquared2D(P,StartLocation)<FMath::Square(40000.f)||FVector::DistSquared2D(P,PreviousItem)<FMath::Square(300.f))continue;
   FHitResult Hit;if(GetWorld()->LineTraceSingleByChannel(Hit,P+FVector(0,0,100),P-FVector(0,0,100),ECC_Visibility,Q)&&Hit.GetActor()&&Hit.GetActor()->ActorHasTag(TEXT("RidePath")))Candidates.Add(Hit.ImpactPoint+FVector(0,0,45));
  }
 }
 // Lab-only nearby target; real-world placement never silently falls back to this.
 if(!HasPaths)Candidates.Add(StartLocation+FVector(2200+RunNumber%3*400,1200,0));
 bObjectiveReady=!Candidates.IsEmpty();CompassSector=-1;if(!bObjectiveReady)return;
 ItemLocation=Candidates[FMath::RandRange(0,Candidates.Num()-1)];PreviousItem=ItemLocation;
 ItemActor=GetWorld()->SpawnActor<AStaticMeshActor>(ItemLocation,FRotator::ZeroRotator);
 if(ItemActor){
  auto* Mesh=ItemActor->GetStaticMeshComponent();Mesh->SetMobility(EComponentMobility::Movable);Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere.Sphere")));Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);ItemActor->SetActorScale3D(FVector(.55f));
  Mesh->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/PiedmontRide/Materials/M_Safety.M_Safety")));
 }
}
void APiedmontRideMode::EndRun(const FString& Reason){
 if(bRunEnded)return;bRunEnded=true;FailureReason=Reason;bItemCollected=false;
 auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(RunBike){RunBike->Ride->Pedal=RunBike->Ride->Steer=0;RunBike->Ride->Speed=0;RunBike->Ride->SetComponentTickEnabled(false);}
 if(PC)if(auto* Person=Cast<APiedmontExplorer>(PC->GetPawn())){Person->bDead=true;Person->bWeaponDrawn=false;Person->GetCharacterMovement()->StopMovementImmediately();Person->GetCharacterMovement()->DisableMovement();}
}
void APiedmontRideMode::RestartRun(){
 auto* PC=UGameplayStatics::GetPlayerController(this,0);if(!PC||!IsValid(RunBike))return;
 for(TActorIterator<APiedmontThreat> It(GetWorld());It;++It)It->Destroy();ActiveThreat=nullptr;EncounterCountdown=300;
 if(auto* Person=Cast<APiedmontExplorer>(PC->GetPawn())){PC->UnPossess();Person->Destroy();}
 RunBike->Ride->Gear=1;RunBike->Ride->Cadence=0;RunBike->KnifeHits=0;RunBike->Explorer=nullptr;RunBike->bDismounted=false;RunBike->Ride->Recovery=0;RunBike->Ride->LastCrash.Empty();RunBike->Ride->Speed=0;RunBike->Ride->Pedal=RunBike->Ride->Steer=RunBike->Ride->Brake=0;RunBike->Ride->Crashes=0;RunBike->Ride->TopSpeed=0;
 RunBike->Ride->SafeLocation=StartLocation;RunBike->Ride->SafeRotation=StartRotation;RunBike->Ride->Respawn();RunBike->Ride->SetComponentTickEnabled(true);RunBike->Rider->SetVisibility(!RunBike->bFirstPerson);PC->Possess(RunBike);PC->SetControlRotation(StartRotation);
 bRunEnded=false;bItemCollected=false;FailureReason.Empty();TimeRemaining=600;StartCountdown=3;RunNumber++;ChooseItem();
}
FString APiedmontRideMode::ItemDirection(){
 if(!bObjectiveReady)return TEXT("OBJECTIVE NOT CONFIGURED");if(bItemCollected)return TEXT("ITEM COLLECTED — ROUTE TO KROG PENDING");
 auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);if(!Pawn)return TEXT("");
 const FVector D=ItemLocation-Pawn->GetActorLocation();float Angle=FMath::RadiansToDegrees(FMath::Atan2(D.X,D.Y));if(Angle<0)Angle+=360;
 if(CompassSector<0||FMath::Abs(FMath::FindDeltaAngleDegrees(CompassSector*90.f,Angle))>55.f)CompassSector=FMath::FloorToInt((Angle+45)/90)%4;
 static const TCHAR* Names[]={TEXT("NORTH"),TEXT("EAST"),TEXT("SOUTH"),TEXT("WEST")};return FString(TEXT("SEARCH "))+Names[CompassSector];
}

APiedmontThreat* APiedmontRideMode::SpawnThreatForValidation(bool Gunman,FVector Location){
#if WITH_EDITOR
 if(GetWorld()->WorldType!=EWorldType::PIE||bRunEnded||(IsValid(ActiveThreat)&&!ActiveThreat->bDead))return nullptr;
 FActorSpawnParameters Params;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
 ActiveThreat=GetWorld()->SpawnActor<APiedmontThreat>(Location,FRotator::ZeroRotator,Params);if(ActiveThreat)ActiveThreat->bGunman=Gunman;return ActiveThreat;
#else
 return nullptr;
#endif
}
void APiedmontRideMode::ConsiderEncounter(){
 if(bRunEnded||(IsValid(ActiveThreat)&&!ActiveThreat->bDead)||FMath::FRand()>.35f)return;
 auto* PC=UGameplayStatics::GetPlayerController(this,0);if(!PC||!PC->GetPawn())return;
 FVector Eye;FRotator View;PC->GetPlayerViewPoint(Eye,View);TArray<FVector> Candidates;
 for(TActorIterator<APiedmontPathSpline> It(GetWorld());It;++It){
  if(It->bBridge)continue;
  for(int I=0;I<It->Centerline->GetNumberOfSplinePoints();I+=5){
   const FVector P=It->Centerline->GetLocationAtSplinePoint(I,ESplineCoordinateSpace::World)+FVector(0,0,92);
   const float Distance=FVector::Dist2D(P,PC->GetPawn()->GetActorLocation());
   if(Distance<2000||Distance>3500||FVector::DotProduct((P-Eye).GetSafeNormal(),View.Vector())>.3f)continue;
   Candidates.Add(P);
  }
 }
 if(Candidates.IsEmpty())return;
 FActorSpawnParameters Params;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
 ActiveThreat=GetWorld()->SpawnActor<APiedmontThreat>(Candidates[FMath::RandRange(0,Candidates.Num()-1)],FRotator::ZeroRotator,Params);
 if(ActiveThreat)ActiveThreat->bGunman=FMath::FRand()<.35f;
}
