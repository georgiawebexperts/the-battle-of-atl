#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/KismetSystemLibrary.h"

// Opt-in test: keep the actual pedestrian capsule, movement component and AI path
// following; suppress autonomous destination selection so it cannot change the fixture.
void TickBattleGrassLinkAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<APiedmontPedestrian> Person;int Phase=0;float Time=0,Travel=0;FVector Last;int Samples=0,Grounded=0;bool Done=false;};
 static FState S;if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}
 if(S.Done||PC->GetWorld()->GetTimeSeconds()<6)return;
 S.Time+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){S.Done=true;UE_LOG(LogTemp,Display,TEXT("BattleGrassLinkAudit: {\"passed\":%s,\"reason\":\"%s\",\"legs\":%d,\"travel_cm\":%.2f,\"samples\":%d,\"grounded\":%d}"),Pass?TEXT("true"):TEXT("false"),Reason,S.Phase-1,S.Travel,S.Samples,S.Grounded);UKismetSystemLibrary::QuitGame(PC,PC,EQuitPreference::Quit,false);};
 const FVector A(10650.3783,8651.0100,-364.0919),B(10961.6267,8463.4584,-354.6365);
 if(S.Time>20){Finish(false,TEXT("Traversal timed out"));return;}
 if(S.Phase==0){
  for(TActorIterator<APiedmontTrafficDirector> It(PC->GetWorld());It;++It)It->SetActorTickEnabled(false);
  auto* Person=PC->GetWorld()->SpawnActor<APiedmontPedestrian>(A+FVector(0,0,90),FRotator::ZeroRotator);
  if(!Person){Finish(false,TEXT("Pedestrian spawn failed"));return;}
  Person->SetActorTickEnabled(false);Person->GetCharacterMovement()->SetComponentTickEnabled(true);
  Person->GetCharacterMovement()->MaxWalkSpeed=135;
  auto* AI=Cast<AAIController>(Person->GetController());
  if(!AI){Finish(false,TEXT("Missing AI controller"));return;}
  if(AI->MoveToLocation(B,10,false,true,false,false,nullptr,false)==EPathFollowingRequestResult::Failed){Finish(false,TEXT("Forward request failed"));return;}
  S.Person=Person;S.Last=Person->GetActorLocation();S.Phase=1;return;
 }
 auto* Person=S.Person.Get();if(!Person||Person->bDead){Finish(false,TEXT("Pedestrian lost"));return;}
 const FVector Location=Person->GetActorLocation();S.Travel+=FVector::Dist2D(Location,S.Last);S.Last=Location;
 ++S.Samples;if(Person->GetCharacterMovement()->IsMovingOnGround())++S.Grounded;
 const FVector Goal=S.Phase==1?B:A;
 if(FVector::Dist2D(Location,Goal)<18){
  if(S.Phase==1){
   auto* AI=Cast<AAIController>(Person->GetController());
   if(!AI||AI->MoveToLocation(A,10,false,true,false,false,nullptr,false)==EPathFollowingRequestResult::Failed){Finish(false,TEXT("Reverse request failed"));return;}
   S.Phase=2;
  }else{S.Phase=3;Finish(S.Travel>650&&S.Grounded>=S.Samples*.99f,TEXT("Both walking legs completed"));}
 }
#endif
}
