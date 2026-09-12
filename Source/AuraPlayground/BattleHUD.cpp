#include "BattleBike.h"
#include "BattleHome.h"
#include "BattleRider.h"
#include "BattleQuest.h"
#include "BattleDrone.h"
#include "BattleZombie.h"
#include "BattlePolice.h"
#include "BattleKnife.h"
#include "PiedmontPedestrian.h"
#include "BattleCheckpoints.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "Styling/CoreStyle.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void ABattleLabHUD::DrawHUD(){
 Super::DrawHUD();if(!Canvas||!GEngine||UGameplayStatics::IsGamePaused(this))return;
 const float W=Canvas->SizeX,H=Canvas->SizeY,S=FMath::Clamp(FMath::Min(W/1920.f,H/1080.f),.75f,1.5f),M=28*S;
 if(!ReadableFont){ReadableFont=NewObject<UFont>(this);ReadableFont->FontCacheType=EFontCacheType::Runtime;}
 const FLinearColor Ink(.018,.024,.032,.93),Peach(1,.64,.32),Muted(.77,.82,.87);
 auto Panel=[&](float X,float Y,float Width,float Height){DrawRect(Ink,X,Y,Width,Height);};
 auto Text=[&](const FString& T,float X,float Y,float Size,FLinearColor C=FLinearColor::White){FCanvasTextItem Item(FVector2D(X,Y),FText::FromString(T),FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(Size*S)),C);Item.Font=ReadableFont;Canvas->DrawItem(Item);};
 auto Center=[&](const FString& T,float Y,float Size,FLinearColor C=FLinearColor::White){FCanvasTextItem Item(FVector2D(W*.5f,Y),FText::FromString(T),FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(Size*S)),C);Item.Font=ReadableFont;Item.bCentreX=true;Canvas->DrawItem(Item);};
 auto* Park=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));
 auto* Bike=Cast<ABattleBike>(GetOwningPawn());auto* Person=Cast<ABattleRider>(GetOwningPawn());auto* Owner=Bike?Bike:Person?Person->ParkedBike.Get():nullptr;
 if(!Owner)return;
 Panel(M,M,470*S,106*S);DrawRect(Peach,M,M,5*S,106*S);
 Text(Park&&Park->bTutorialActive?TEXT("FREE RIDE • MIDTOWN"):Park&&Park->Quest&&Park->Quest->bCollected?TEXT("MEET MORGAN"):TEXT("FIND MY LOST PHONE"),M+20*S,M+15*S,25,Peach);
 FString Objective=Park&&Park->Quest?Park->Quest->SearchDirection(GetOwningPawn()->GetActorLocation()):TEXT("Search the park");
 if(Park&&Park->Quest&&Park->Quest->bCollected)Objective=Park->Quest->NextCheckpoint<2?FString(BattleCheckpoints::Anchors[Park->Quest->NextCheckpoint].Name):TEXT("Through Krog Street Tunnel");
 if(Park&&Park->Quest&&Park->Quest->bCollected&&Park->Quest->NextCheckpoint>=2)for(TActorIterator<ABattleHome> It(GetWorld());It;++It)if(It->bTunnelExited){Objective=TEXT("98 Estoria • patio on the right");break;}
 if(Park&&Park->bTutorialActive)Objective=TEXT("Ride to the 14th Street gateway");
 Text(Objective,M+20*S,M+60*S,23,Muted);
 if(Park){
  Panel(W*.5f-125*S,M,250*S,106*S);
  const int Seconds=FMath::CeilToInt(Park->TimeRemaining);
  Center(Park->bTutorialActive?TEXT("UNTIMED"):FString::Printf(TEXT("%02d:%02d"),Seconds/60,Seconds%60),M+6*S,Park->bTutorialActive?34:50,Seconds<60?FLinearColor(1,.2,.2):FLinearColor::White);
  Center(Park->bTutorialActive?TEXT("Clock starts at gate"):Person?FString::Printf(TEXT("ON FOOT %.2fx"),Park->FootTimeMultiplier):Park->DifficultyName.ToString(),M+66*S,(Park->bTutorialActive||Person)?18:22,Person?Peach:Muted);
  if(Park->TimeNoticeRemaining>0){Panel(W*.5f-230*S,M+116*S,460*S,46*S);Center(FString::Printf(TEXT("%+.0fs  %s"),Park->LastTimeDelta,*Park->TimeNotice),M+121*S,23,Park->LastTimeDelta>0?FLinearColor(.3,1,.65):FLinearColor(1,.35,.3));}
  if(Park->Quest)Park->Quest->DrawRadar(this,Canvas);
  if(Park->StartCountdown>0){Panel(W*.5f-100*S,H*.38f,200*S,120*S);Center(FString::FromInt(FMath::CeilToInt(Park->StartCountdown)),H*.38f+12*S,82,Peach);}
 }
 if(Park&&Park->bTutorialActive&&Park->bTutorialHelp){
  const float Y=M+177*S;Panel(M,Y,470*S,240*S);
  Text(TEXT("PRACTICE BEFORE YOU START"),M+15*S,Y+12*S,20,Peach);
  Text(FString(Park->PracticeDistance>500?TEXT("✓ "):TEXT("• "))+(Person?TEXT("WASD / arrows: walk   SHIFT: run"):TEXT("WASD / arrows: pedal and steer")),M+15*S,Y+49*S,20,Muted);
  Text(FString(Park->bPracticeBraked?TEXT("✓ "):TEXT("• "))+(Person?TEXT("SPACE: jump   C / CTRL: crouch"):TEXT("Space: brake   Q / R: change gear")),M+15*S,Y+83*S,20,Muted);
  Text(FString(Park->bPracticeDismounted?TEXT("✓ "):TEXT("• "))+TEXT("E: get off / back on the bike"),M+15*S,Y+117*S,20,Muted);
  Text(FString(Park->bPracticeHorn?TEXT("✓ "):TEXT("• "))+(Person?TEXT("G: draw / holster   1–5: weapon"):TEXT("H: horn   J: jump while moving")),M+15*S,Y+151*S,20,Muted);
  Text(TEXT("Enter gate to start. F1: hide help"),M+15*S,Y+197*S,18,Peach);
 }
 if(Park&&Park->ExpansionNoticeRemaining>0){Panel(W*.5f-340*S,H*.48f,680*S,110*S);Center(TEXT("MIDTOWN EXPANSION COMING SOON"),H*.48f+12*S,26,Peach);Center(TEXT("The Battle of ATL • Follow the route to Piedmont Park"),H*.48f+59*S,20);}
 Panel(W-M-300*S,M,300*S,106*S);
 if(Bike){Text(FString::Printf(TEXT("%.0f MPH"),Bike->Ride->Speed*.0223694f),W-M-280*S,M+12*S,38);Text(FString::Printf(TEXT("GEAR %d / 5"),Bike->Ride->Gear),W-M-280*S,M+65*S,23,Muted);}
 else{Text(Person->bWeaponDrawn?BattleWeapons::Name(Person->CurrentWeapon):TEXT("HANDS FREE"),W-M-280*S,M+12*S,28,Peach);Text(Person->bWeaponDrawn?FString::Printf(TEXT("%d  /  %s"),Person->Ammo,*Person->ReserveLabel()):TEXT("G  DRAW WEAPON"),W-M-280*S,M+55*S,Person->bWeaponDrawn?30:20);}
 if(auto* Rules=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this))){if(Rules->Trouble>.1f||Rules->PeopleHit>0){Panel(M,M+118*S,420*S,44*S);Text(FString::Printf(TEXT("DANGER %.0f  |  PEOPLE %d/3%s"),Rules->Trouble,Rules->PeopleHit,Rules->bPoliceAlert?TEXT("  POLICE"):TEXT("")),M+12*S,M+128*S,18,Peach);}}
 Panel(W-M-300*S,M+118*S,300*S,44*S);Text(FString::Printf(TEXT("%sHORN  %d / 5"),Bike?TEXT("H  "):TEXT(""),Owner->HornUses),W-M-280*S,M+126*S,22,Owner->HornUses>0?Muted:Peach);
 const float Bottom=H-M-156*S;
 Panel(M,Bottom,310*S,156*S);
 Text(FString::Printf(TEXT("HEALTH  %.0f"),Owner->RiderHealth),M+18*S,Bottom+10*S,22);
 DrawRect(FLinearColor(.15,.19,.2),M+18*S,Bottom+50*S,274*S,18*S);DrawRect(Owner->RiderHealth<25?FLinearColor(1,.15,.1):FLinearColor(.3,.9,.62),M+18*S,Bottom+50*S,FMath::Clamp(Owner->RiderHealth/100.f,0.f,1.f)*274*S,18*S);
 Text(FString::Printf(TEXT("BOOST  %.0f%%"),Owner->Nitro),M+18*S,Bottom+80*S,20,Muted);
 DrawRect(FLinearColor(.15,.19,.2),M+18*S,Bottom+119*S,274*S,12*S);DrawRect(FLinearColor(.15,.75,1),M+18*S,Bottom+119*S,Owner->Nitro/100.f*274*S,12*S);
 FString Prompt=TEXT("E  GET OFF THE BIKE");
 FString Help=TEXT("W/UP pedal   S/DOWN brake   J jump");
 if(Person){const bool Near=FVector::Dist(Person->GetActorLocation(),Owner->GetActorLocation())<240;Prompt=Near?TEXT("E  GET ON THE BIKE"):TEXT("RETURN TO YOUR BIKE TO RIDE");Help=Person->bWeaponDrawn?TEXT("G holster  CLICK fire  R reload  F melee"):TEXT("G draw  SHIFT run  SPACE jump  C crouch");}
 Panel(W*.5f-300*S,H-M-100*S,600*S,100*S);
 Center(Prompt,H-M-88*S,29,Peach);Center(Help,H-M-43*S,21,Muted);
 if(Bike){Panel(M,Bottom-58*S,310*S,50*S);Text(FString::Printf(TEXT("PISTOL  %d / %d"),Bike->PistolAmmo,Bike->Inventory[0].Reserve),M+20*S,Bottom-48*S,27,Peach);}
 const float CX=W*.5f,CY=H*.5f;FLinearColor Aim=FLinearColor::White;
 if(auto* PC=GetOwningPlayerController()){
  FVector Eye;FRotator View;PC->GetPlayerViewPoint(Eye,View);FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(HUDAim),true,GetOwningPawn());Q.AddIgnoredActor(Owner);
  if(GetWorld()->LineTraceSingleByChannel(Hit,Eye,Eye+View.Vector()*14000,ECC_Visibility,Q)){
   auto* Target=Cast<APiedmontExplorer>(Hit.GetActor());
   if(Target&&!Target->bDead){Aim=FLinearColor(1,.3,.22);const FString Name=Target->ActorHasTag(TEXT("BattlePolice"))?TEXT("POLICE -60s"):Target->IsA<ABattleZombie>()?TEXT("ZOMBIE +10s"):Target->IsA<ABattleKnife>()?TEXT("KNIFE ATTACKER"):TEXT("PERSON -10s");Panel(CX-115*S,CY+32*S,230*S,38*S);Center(Name,CY+36*S,22,Aim);}
  }
 }
 DrawLine(CX-15*S,CY,CX-5*S,CY,Aim,2*S);DrawLine(CX+5*S,CY,CX+15*S,CY,Aim,2*S);DrawLine(CX,CY-15*S,CX,CY-5*S,Aim,2*S);DrawLine(CX,CY+5*S,CX,CY+15*S,Aim,2*S);
 if((Person?Person->HitFeedback:Bike->HitFeedback)>0){for(int SX:{-1,1})for(int SY:{-1,1})DrawLine(CX+SX*8*S,CY+SY*8*S,CX+SX*19*S,CY+SY*19*S,Peach,3*S);}
 FString Notice;
 if(Owner->RespawnRemaining>0)Notice=TEXT("RECOVERING AT CHECKPOINT  |  -10 SECONDS");
 else if(Owner->StunRemaining>0)Notice=FString::Printf(TEXT("%s  %.1fs  |  GET UP, THEN E TO REMOUNT"),*Owner->StunLabel,Owner->StunRemaining);
 else if(Bike&&Bike->Ride->Recovery>0)Notice=FString::Printf(TEXT("RECOVERING  %.1f"),Bike->Ride->Recovery);
 else if(Person&&Person->ReloadRemaining>0)Notice=TEXT("RELOADING");
 else if(Owner->HornNoticeRemaining>0)Notice=Owner->HornNotice;
 else if(Owner->PickupNoticeRemaining>0)Notice=FString::Printf(TEXT("+%.0f HEALTH"),Owner->LastHealAmount);
 if(Notice.IsEmpty())for(TActorIterator<ABattleKnife> It(GetWorld());It;++It)if(!It->bDead&&!It->bEscaped&&FVector::Dist2D(It->GetActorLocation(),GetOwningPawn()->GetActorLocation())<1400){Notice=It->bWindingUp?TEXT("KNIFE STRIKE — MOVE!"):It->Stabs>0?TEXT("KNIFE CHASE — RUN, DEFEND OR REMOUNT"):TEXT("KNIFE ATTACKER — RUN OR G TO DRAW");break;}
 if(Notice.IsEmpty())for(TActorIterator<ABattlePolice> It(GetWorld());It;++It)if(It->bWarning){Notice=TEXT("POLICE TASER — MOVE TO COVER!");break;}
 if(Notice.IsEmpty())for(TActorIterator<ABattleDrone> It(GetWorld());It;++It)if(It->bWarning){Notice=TEXT("DRONE SWOOP — KEEP MOVING!");break;}
 if(!Notice.IsEmpty()){Panel(CX-360*S,H*.69f,720*S,52*S);Center(Notice,H*.69f+9*S,27,Peach);}
 if(auto* Viewer=GetOwningPawn()){
  const ABattleZombie* Speaking=nullptr;float Best=2500;
  for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)if(It->SubtitleRemaining>0){const float D=FVector::Dist2D(Viewer->GetActorLocation(),It->GetActorLocation());if(D<Best){Best=D;Speaking=*It;}}
  if(Speaking){Panel(CX-440*S,H-M-166*S,880*S,48*S);Center(Speaking->CharacterName()+TEXT(": ")+Speaking->Subtitle,H-M-159*S,24);}
 }
}
