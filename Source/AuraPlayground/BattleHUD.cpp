#include "BattleGunman.h"
#include "BattlePlayerCrash.h"
#include "BattleBike.h"
#include "BattleSpareBikes.h"
#include "BattleMusic.h"
#include "BattleHome.h"
#include "BattleRider.h"
#include "BattleQuest.h"
#include "BattleDrone.h"
#include "BattlePickup.h"
#include "BattleZombie.h"
#include "BattlePolice.h"
#include "BattleKnife.h"
#include "PiedmontPedestrian.h"
#include "BattleCheckpoints.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "Styling/CoreStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "Fonts/FontMeasure.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void ABattleLabHUD::DrawHUD(){
 Super::DrawHUD();if(!Canvas||!GEngine||UGameplayStatics::IsGamePaused(this))return;
 const float W=Canvas->SizeX,H=Canvas->SizeY,S=FMath::Clamp(FMath::Min(W/1920.f,H/1080.f),.75f,1.5f),M=28*S;
 if(!ReadableFont){ReadableFont=NewObject<UFont>(this);ReadableFont->FontCacheType=EFontCacheType::Runtime;}
 const FLinearColor Ink(.012,.018,.025,1),Peach(1,.74,.42),Muted(.96,.97,1);
 FBox2D TextPanel(FVector2D::ZeroVector,FVector2D(W,H));
 const auto Measure=FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
 auto Panel=[&](float X,float Y,float Width,float Height){TextPanel=FBox2D(FVector2D(X,Y),FVector2D(X+Width,Y+Height));DrawRect(Ink,X,Y,Width,Height);};
 auto DrawFitted=[&](const FString& T,float X,float Y,float Size,FLinearColor C,bool Centered){
  const bool InPanel=Y>=TextPanel.Min.Y&&Y<TextPanel.Max.Y&&X>=TextPanel.Min.X&&X<=TextPanel.Max.X;
  const float Available=FMath::Max(1.f,InPanel?(Centered?2.f*FMath::Min(X-TextPanel.Min.X,TextPanel.Max.X-X)-24*S:TextPanel.Max.X-X-12*S):W-X-M);
  auto Font=FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(Size*S));
  while(Font.Size>1&&Measure->Measure(T,Font).X>Available)--Font.Size;
  FCanvasTextItem Item(FVector2D(X,Y),FText::FromString(T),Font,C);Item.Font=ReadableFont;Item.bCentreX=Centered;Canvas->DrawItem(Item);
 };
 auto Text=[&](const FString& T,float X,float Y,float Size,FLinearColor C=FLinearColor::White){DrawFitted(T,X,Y,Size,C,false);};
 auto Center=[&](const FString& T,float Y,float Size,FLinearColor C=FLinearColor::White){DrawFitted(T,W*.5f,Y,Size,C,true);};
 auto* Park=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));
 auto* Bike=Cast<ABattleBike>(GetOwningPawn());auto* Person=Cast<ABattleRider>(GetOwningPawn());auto* Owner=Bike?Bike:Person?Person->ParkedBike.Get():nullptr;
 if(!Owner)return;
 const bool GettingUp=Owner->bCrashActive&&IsValid(Owner->PlayerCrash)&&Owner->PlayerCrash->GetRecoveryPose()!=nullptr;
 const bool HasPhone=Park&&Park->Quest&&Park->Quest->bCollected&&!Park->bTutorialActive;
 const float ObjectiveHeight=HasPhone?144.f:106.f;
 Panel(M,M,470*S,ObjectiveHeight*S);DrawRect(Peach,M,M,5*S,ObjectiveHeight*S);
 Text(Park&&Park->bTutorialActive?TEXT("FREE RIDE • MIDTOWN"):Park&&Park->Quest&&Park->Quest->bCollected?TEXT("MEET MORGAN"):TEXT("FIND MY LOST PHONE"),M+20*S,M+15*S,25,Peach);
 FString Objective=Park&&Park->Quest?Park->Quest->SearchDirection(GetOwningPawn()->GetActorLocation()):TEXT("Search the park");
 if(Park&&Park->Quest&&Park->Quest->bCollected)Objective=Park->Quest->NextCheckpoint<2?FString(BattleCheckpoints::Anchors[Park->Quest->NextCheckpoint].Name):TEXT("Through Krog Street Tunnel");
 if(Park&&Park->Quest&&Park->Quest->bCollected&&Park->Quest->NextCheckpoint>=2)for(TActorIterator<ABattleHome> It(GetWorld());It;++It)if(It->bTunnelExited){Objective=TEXT("98 Estoria • patio on the right");break;}
 if(Park&&Park->bTutorialActive)Objective=TEXT("Ride to the 14th Street gateway");
 Text(Objective,M+20*S,M+60*S,23,Muted);
 if(HasPhone){
  FString Hint=Park->Quest->RoutePoints.Num()>1?TEXT("Follow the gold route on your watch"):TEXT("Watch route is recalculating...");
  for(TActorIterator<ABattleHome> It(GetWorld());It;++It)if(It->bTunnelExited){Hint=TEXT("Enter the patio to meet Morgan");break;}
  Text(Hint,M+20*S,M+104*S,18,Peach);
 }
 if(Park){
  Panel(W*.5f-125*S,M,250*S,106*S);
  const int Seconds=FMath::CeilToInt(Park->TimeRemaining);
  Center(Park->bTutorialActive?TEXT("UNTIMED"):FString::Printf(TEXT("%02d:%02d"),Seconds/60,Seconds%60),M+6*S,Park->bTutorialActive?34:50,Seconds<60?FLinearColor(1,.2,.2):FLinearColor::White);
  Center(Park->bTutorialActive?TEXT("Clock starts at gate"):(Person||Owner->bCrashActive)?FString::Printf(TEXT("%s %.2fx"),Person&&Person->bSwimming?TEXT("SWIMMING"):TEXT("ON FOOT"),Park->FootTimeMultiplier):Park->DifficultyName.ToString(),M+66*S,(Park->bTutorialActive||Person||Owner->bCrashActive)?18:22,(Person||Owner->bCrashActive)?Peach:Muted);
  if(Park->TimeNoticeRemaining>0){Panel(W*.5f-230*S,M+116*S,460*S,46*S);Center(FString::Printf(TEXT("%+.0fs  %s"),Park->LastTimeDelta,*Park->TimeNotice),M+121*S,23,Park->LastTimeDelta>0?FLinearColor(.3,1,.65):FLinearColor(1,.35,.3));}
  if(Park->Quest)Park->Quest->DrawRadar(this,Canvas);
  if(!Park->bTutorialActive&&!Park->bRunEnded&&(Park->StartCountdown>0||Park->RunElapsed<2.5f)){
   const float Y=M+170*S;Panel(W*.5f-220*S,Y,440*S,92*S);
   Center(Park->StartCountdown>0?FString::Printf(TEXT("RIDE STARTED  •  %d"),FMath::CeilToInt(Park->StartCountdown)):TEXT("GO! TIMER RUNNING"),Y+10*S,28,Peach);
   Center(Park->StartCountdown>0?TEXT("Keep riding—controls stay active"):TEXT("Find your phone. Reach Morgan."),Y+52*S,19,Muted);
  }
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
 if(Owner->StunRemaining>0){Text(Owner->StunLabel,W-M-280*S,M+12*S,28,Peach);Text(FString::Printf(TEXT("Recovering  %.1fs"),Owner->StunRemaining),W-M-280*S,M+55*S,20,Muted);}
 else if(Owner->bCrashActive){Text(TEXT("KNOCKED OFF"),W-M-280*S,M+12*S,28,Peach);Text(GettingUp?TEXT("Getting back up"):TEXT("Recovering"),W-M-280*S,M+65*S,23,Muted);}
 else if(Bike){Text(FString::Printf(TEXT("%.0f MPH"),(Bike->Ride->Speed+Bike->Ride->ReverseSpeed)*.0223694f),W-M-280*S,M+12*S,38);Text((Bike->Ride->ReverseSpeed>1?FString::Printf(TEXT("REVERSE  |  GEAR %d"),Bike->Ride->Gear):FString::Printf(TEXT("GEAR %d / 5"),Bike->Ride->Gear)),W-M-280*S,M+65*S,23,Muted);}
 else if(Person->bSwimming){Text(TEXT("SWIMMING"),W-M-280*S,M+12*S,28,Peach);Text(TEXT("Bike stays at the bank"),W-M-280*S,M+55*S,20,Muted);}
 else{Text(Person->ReloadRemaining>0?TEXT("RELOADING"):Person->bWeaponDrawn?BattleWeapons::Name(Person->CurrentWeapon):TEXT("HANDS FREE"),W-M-280*S,M+12*S,28,Peach);if(Person->bWeaponDrawn){Text(FString::Printf(TEXT("%d  LOADED"),Person->Ammo),W-M-280*S,M+46*S,24);Text(FString::Printf(TEXT("%s  SPARE"),*Person->ReserveLabel()),W-M-280*S,M+78*S,18,Muted);}else Text(TEXT("G  DRAW WEAPON"),W-M-280*S,M+55*S,20);}
 if(auto* Rules=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this))){if(Rules->Trouble>.1f||Rules->PeopleHit>0){const float DangerY=M+(ObjectiveHeight+12)*S;Panel(M,DangerY,420*S,44*S);Text(FString::Printf(TEXT("DANGER %.0f  |  PEOPLE %d/3%s"),Rules->Trouble,Rules->PeopleHit,Rules->bPoliceAlert?TEXT("  POLICE"):TEXT("")),M+12*S,DangerY+10*S,18,Peach);}}
 Panel(W-M-300*S,M+118*S,300*S,44*S);Text(FString::Printf(TEXT("%sHORN  %d / 5"),Bike?TEXT("H  "):TEXT(""),Owner->HornUses),W-M-280*S,M+126*S,22,Owner->HornUses>0?Muted:Peach);
 if(Bike){Panel(W-M-300*S,M+174*S,300*S,44*S);Text(Bike->Ride->bRealHandling?TEXT("P  MODE: REALISTIC"):TEXT("P  MODE: ARCADE"),W-M-280*S,M+185*S,18,Muted);}
 const float Bottom=H-M-156*S;
 Panel(M,Bottom,310*S,156*S);
 Text(FString::Printf(TEXT("HEALTH  %.0f"),Owner->RiderHealth),M+18*S,Bottom+10*S,22);
 DrawRect(FLinearColor(.15,.19,.2),M+18*S,Bottom+50*S,274*S,18*S);DrawRect(Owner->RiderHealth<25?FLinearColor(1,.15,.1):FLinearColor(.3,.9,.62),M+18*S,Bottom+50*S,FMath::Clamp(Owner->RiderHealth/100.f,0.f,1.f)*274*S,18*S);
 Text(Owner->Ride->BoostRemaining>0?FString::Printf(TEXT("SPEED BOOST  %.1fs"),Owner->Ride->BoostRemaining):FString::Printf(TEXT("BOOST  %.0f%%"),Owner->Nitro),M+18*S,Bottom+80*S,20,Muted);
 DrawRect(FLinearColor(.15,.19,.2),M+18*S,Bottom+119*S,274*S,12*S);DrawRect(FLinearColor(.15,.75,1),M+18*S,Bottom+119*S,Owner->Nitro/100.f*274*S,12*S);
 FString Prompt=Bike&&Bike->Ride->Speed<20?TEXT("E DISMOUNT  |  S/DOWN BACK UP"):TEXT("E  GET OFF THE BIKE");
 const float MusicY=M+(Bike?230.f:174.f)*S;Panel(W-M-300*S,MusicY,300*S,68*S);Text(BattleMusic::Enabled()?FString::Printf(TEXT("M  MUSIC: SONG %d / 2"),BattleMusic::Selection()):TEXT("M  MUSIC: OFF"),W-M-280*S,MusicY+6*S,18,Muted);Text(TEXT("Cycle: Off > 1 > 2 > Off"),W-M-280*S,MusicY+36*S,16,Muted);
 FString Help=TEXT("Q/R gears   J jump   SHIFT boost   H horn");
 if(Person){const bool Near=FVector::Dist(Person->GetActorLocation(),Owner->GetActorLocation())<240;Prompt=BattleSpareBikes::Nearest(Person)?TEXT("E  RIDE THIS BIKE"):Near?TEXT("E  GET ON THE BIKE"):TEXT("FIND A BIKE  /  BLUE WATCH MARKERS");Help=Person->bWeaponDrawn?TEXT("G holster  CLICK fire  R reload  F melee"):TEXT("G draw  SHIFT run  SPACE jump  C crouch");}
 if(Person&&Person->bSwimming){Prompt=TEXT("EXPLORE THE LAKE");Help=TEXT("WASD / arrows swim   E remount by bike");}
 if(Owner->bCrashActive){Prompt=GettingUp?TEXT("GETTING BACK UP"):TEXT("KNOCKED OFF YOUR BIKE");Help=TEXT("Recovery is automatic — clock keeps running");}
 if(Owner->StunRemaining>0){Prompt=Owner->StunLabel;Help=FString::Printf(TEXT("Control returns in %.1fs — clock keeps running"),Owner->StunRemaining);}
 if(Person&&(Person->bSwimming||Person->bWeaponDrawn)&&Owner->StunRemaining<=0&&!Owner->bCrashActive){
  if(Person->bWeaponDrawn&&!Person->bSwimming)Help=TEXT("G holster   CLICK fire   R reload   F melee");
  Panel(W*.5f-300*S,H-M-50*S,600*S,50*S);Center(Help,H-M-38*S,21,Muted);
 }else{
  Panel(W*.5f-300*S,H-M-100*S,600*S,100*S);
  Center(Prompt,H-M-88*S,29,Peach);Center(Help,H-M-43*S,21,Muted);
 }
 if(Bike){Panel(M,Bottom-82*S,310*S,74*S);Text(FString::Printf(TEXT("PISTOL  %d LOADED"),Bike->PistolAmmo),M+18*S,Bottom-74*S,23,Peach);Text(FString::Printf(TEXT("%d SPARE"),Bike->Inventory[0].Reserve),M+18*S,Bottom-41*S,19,Muted);}
 const float CX=W*.5f,CY=H*.5f;FLinearColor Aim=FLinearColor::White;const ABattleDrone* AimedDrone=nullptr;
 if(!Owner->bCrashActive&&!(Person&&Person->bSwimming)){
 if(auto* PC=GetOwningPlayerController()){
  FVector Eye;FRotator View;PC->GetPlayerViewPoint(Eye,View);FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(HUDAim),true,GetOwningPawn());Q.AddIgnoredActor(Owner);
  if(GetWorld()->LineTraceSingleByChannel(Hit,Eye,Eye+View.Vector()*14000,ECC_Visibility,Q)){
   auto* Target=Cast<APiedmontExplorer>(Hit.GetActor());
   if(Target&&!Target->bDead){Aim=FLinearColor(1,.3,.22);const FString Name=Target->ActorHasTag(TEXT("BattlePolice"))?TEXT("POLICE -60s"):Target->IsA<ABattleZombie>()?TEXT("ZOMBIE +10s"):Target->IsA<ABattleKnife>()?TEXT("KNIFE ATTACKER"):Target->IsA<ABattleGunman>()?TEXT("GUNMAN"):TEXT("PERSON -10s");Panel(CX-115*S,CY+32*S,230*S,38*S);Center(Name,CY+36*S,22,Aim);}
   else if(auto* Drone=Cast<ABattleDrone>(Hit.GetActor());Drone&&Drone->Health>0&&!Drone->IsActorBeingDestroyed()){Aim=FLinearColor(1,.3,.22);AimedDrone=Drone;}
  }
 }
 DrawLine(CX-15*S,CY,CX-5*S,CY,Aim,2*S);DrawLine(CX+5*S,CY,CX+15*S,CY,Aim,2*S);DrawLine(CX,CY-15*S,CX,CY-5*S,Aim,2*S);DrawLine(CX,CY+5*S,CX,CY+15*S,Aim,2*S);
 if((Person?Person->HitFeedback:Bike->HitFeedback)>0){for(int SX:{-1,1})for(int SY:{-1,1})DrawLine(CX+SX*8*S,CY+SY*8*S,CX+SX*19*S,CY+SY*19*S,Peach,3*S);}
 if(Owner->ShotNoticeRemaining>0){Panel(CX-130*S,CY+78*S,260*S,34*S);Center(Owner->ShotNotice,CY+81*S,22,Peach);}
 }
 FString Notice;
 if(Owner->RespawnRemaining>0)Notice=TEXT("RECOVERING AT CHECKPOINT  |  -10 SECONDS");
 else if(Owner->bCrashActive||Owner->StunRemaining>0)Notice=TEXT("");

 else if(Bike&&Bike->Ride->Recovery>0)Notice=FString::Printf(TEXT("RECOVERING  %.1f"),Bike->Ride->Recovery);
 else if(Owner->HornNoticeRemaining>0)Notice=Owner->HornNotice;
 else if(Owner->PickupNoticeRemaining>0)Notice=FString::Printf(TEXT("+%.0f HEALTH"),Owner->LastHealAmount);
 if(!Owner->bCrashActive&&Notice.IsEmpty())for(TActorIterator<ABattleKnife> It(GetWorld());It;++It)if(!It->bDead&&!It->bEscaped&&FVector::Dist2D(It->GetActorLocation(),GetOwningPawn()->GetActorLocation())<1400){Notice=It->bWindingUp?TEXT("KNIFE STRIKE — MOVE!"):It->Stabs>0?TEXT("KNIFE CHASE — RUN, DEFEND OR REMOUNT"):TEXT("KNIFE ATTACKER — RUN OR G TO DRAW");break;}
 if(auto* PC=GetOwningPlayerController()){
  FVector Eye;FRotator View;PC->GetPlayerViewPoint(Eye,View);
  const ABattleGunman* Active=nullptr;float Nearest=3000;
  for(TActorIterator<ABattleGunman> It(GetWorld());It;++It)if(!It->bDead&&(It->bWarning||It->ShotAlertRemaining>0)){const float Distance=FVector::Dist2D(Eye,It->GetActorLocation());if(Distance<Nearest){Nearest=Distance;Active=*It;}}
  if(Active){
   const float Angle=FMath::DegreesToRadians(FMath::FindDeltaAngleDegrees(View.Yaw,(Active->GetActorLocation()-Eye).Rotation().Yaw));
   const FVector2D Direction(FMath::Sin(Angle),-FMath::Cos(Angle)),Across(-Direction.Y,Direction.X),Centre(CX,CY);
   const FVector2D Tip=Centre+Direction*(H*.34f),Base=Centre+Direction*(H*.34f-21*S);
   const float Urgency=1.f-FMath::Clamp(Active->WindupRemaining/1.8f,0.f,1.f);
   const FLinearColor Color=Active->bWarning?FMath::Lerp(Peach,FLinearColor(1,.12,.08),Urgency):FLinearColor(1,.12,.08);
   const float Bearing=FMath::RadiansToDegrees(Angle);
   const TCHAR* BearingLabel=FMath::Abs(Bearing)>135?TEXT("BEHIND"):Bearing>45?TEXT("RIGHT"):Bearing< -45?TEXT("LEFT"):TEXT("AHEAD");
   for(int Side:{-1,1}){const FVector2D Wing=Base+Across*Side*13*S;DrawLine(Tip.X,Tip.Y,Wing.X,Wing.Y,Color,5*S);}
   const FVector Head=Active->GetActorLocation()+FVector(0,0,110);FVector2D Label;
   FHitResult Sight;FCollisionQueryParams Query(SCENE_QUERY_STAT(GunmanLabel),false,GetOwningPawn());Query.AddIgnoredActor(Owner);
   const bool Clear=!GetWorld()->LineTraceSingleByChannel(Sight,Eye,Head,ECC_Visibility,Query)||Sight.GetActor()==Active;
   if(Clear&&PC->ProjectWorldLocationToScreen(Head,Label)&&Label.X>65*S&&Label.X<W-65*S&&Label.Y>125*S&&Label.Y<H-170*S){
    Label.X=FMath::Clamp(Label.X,130*S,W-130*S);
    Panel(Label.X-130*S,Label.Y-34*S,260*S,37*S);
    Text(Active->bWarning?FString::Printf(TEXT("GUNMAN  |  %.1fs"),FMath::Max(.1f,Active->WindupRemaining)):TEXT("GUNMAN  |  FIRING"),Label.X-119*S,Label.Y-31*S,21,Color);
    if(Active->bWarning){DrawRect(FLinearColor(.15,.19,.2),Label.X-119*S,Label.Y-4*S,238*S,3*S);DrawRect(Color,Label.X-119*S,Label.Y-4*S,238*S*Urgency,3*S);}
   }
   // Imminent gunfire takes priority over incidental pickup/horn notices.
   if(Owner->RespawnRemaining<=0&&!Owner->bCrashActive)Notice=Active->bWarning?FString::Printf(TEXT("GUNMAN %s  |  %.1fs  |  MOVE TO COVER"),BearingLabel,FMath::Max(.1f,Active->WindupRemaining)):FString::Printf(TEXT("GUNFIRE %s  |  FIND COVER"),BearingLabel);
  }
 }
 if(Notice.IsEmpty()&&!Owner->bCrashActive)for(TActorIterator<ABattlePolice> It(GetWorld());It;++It)if(!It->bDead&&It->bWarning){
  Notice=FString::Printf(TEXT("APD TASER  |  %.1fs  |  TAKE COVER"),FMath::Max(0.f,It->WarningRemaining));
  FVector2D Label;
  if(GetOwningPlayerController()->ProjectWorldLocationToScreen(It->GetActorLocation()+FVector(0,0,115),Label)&&Label.X>130*S&&Label.X<W-130*S&&Label.Y>180*S&&Label.Y<H*.60f){
   Panel(Label.X-120*S,Label.Y-38*S,240*S,42*S);
   Text(TEXT("STOP! APD"),Label.X-108*S,Label.Y-34*S,21,Peach);
   DrawRect(FLinearColor(.15,.19,.2),Label.X-108*S,Label.Y-5*S,216*S,4*S);
   DrawRect(Peach,Label.X-108*S,Label.Y-5*S,216*S*FMath::Clamp(It->WarningRemaining/2.f,0.f,1.f),4*S);
  }
  break;
 }
 for(TActorIterator<ABattleDrone> It(GetWorld());It;++It)if(!It->bSpent){
  const FVector Location=It->GetActorLocation();const float Distance=FVector::Dist(GetOwningPawn()->GetActorLocation(),Location)/100.f;
  if(Distance>65.f)continue;
  if(Notice.IsEmpty())Notice=It->bWarning?FString::Printf(TEXT("DRONE  |  %.0fm  |  SHOOT OR EVADE"),Distance):TEXT("DRONE DIVING  |  KEEP MOVING");
  auto* PC=GetOwningPlayerController();FVector Eye;FRotator View;PC->GetPlayerViewPoint(Eye,View);FVector2D Screen;
  FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(DroneHUD),false,GetOwningPawn());
  const bool Visible=!GetWorld()->LineTraceSingleByChannel(Hit,Eye,Location,ECC_Visibility,Q)||Hit.GetActor()==*It;
  if(Visible&&PC->ProjectWorldLocationToScreen(Location,Screen)&&Screen.X>105*S&&Screen.X<W-105*S&&Screen.Y>145*S&&Screen.Y<H-170*S){
   Panel(Screen.X-100*S,Screen.Y-48*S,200*S,32*S);
   Text(FString::Printf(TEXT("DRONE  %.0fm"),Distance),Screen.X-91*S,Screen.Y-46*S,20,AimedDrone==*It?Aim:Peach);
   DrawLine(Screen.X-12*S,Screen.Y-12*S,Screen.X+12*S,Screen.Y-12*S,Peach,2*S);
   DrawLine(Screen.X-12*S,Screen.Y+12*S,Screen.X+12*S,Screen.Y+12*S,Peach,2*S);
  }
 }
 // A single nearby supply label helps players spot ammunition without filling the view.
 if(Notice.IsEmpty()&&!Owner->bCrashActive){
  auto* PC=GetOwningPlayerController();FVector Eye;FRotator View;PC->GetPlayerViewPoint(Eye,View);
  float Best=3000.f;FVector2D Label;bool Found=false;
  for(TActorIterator<ABattleWeaponCrate> It(GetWorld());It;++It){
   if(It->WeaponSlot!=0||It->bConsumed)continue;
   const FVector Point=It->GetActorLocation()+FVector(0,0,40);const float Distance=FVector::Dist(GetOwningPawn()->GetActorLocation(),It->GetActorLocation());
   if(Distance>=Best)continue;FVector2D Candidate;FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(AmmoHUD),false,GetOwningPawn());Q.AddIgnoredActor(*It);
   if(GetWorld()->LineTraceSingleByChannel(Hit,Eye,Point,ECC_Visibility,Q)||!PC->ProjectWorldLocationToScreen(Point,Candidate))continue;
   if(Candidate.X<140*S||Candidate.X>W-140*S||Candidate.Y<170*S||Candidate.Y>H-180*S)continue;
   Best=Distance;Label=Candidate;Found=true;
  }
  if(Found){Panel(Label.X-130*S,Label.Y-32*S,260*S,34*S);Text(FString::Printf(TEXT("AMMO +17  |  %.0fm"),Best/100.f),Label.X-118*S,Label.Y-28*S,20,FLinearColor(.45f,.9f,1));}
 }
 if(!Notice.IsEmpty()){Panel(CX-360*S,H*.69f,720*S,52*S);Center(Notice,H*.69f+9*S,27,Peach);}
 if(auto* Viewer=GetOwningPawn()){
  const ABattleZombie* Speaking=nullptr;float Best=2500;
  for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)if(It->SubtitleRemaining>0){const float D=FVector::Dist2D(Viewer->GetActorLocation(),It->GetActorLocation());if(D<Best){Best=D;Speaking=*It;}}
  if(Speaking){Panel(CX-440*S,H-M-166*S,880*S,48*S);Center(Speaking->CharacterName()+TEXT(": ")+Speaking->Subtitle,H-M-159*S,24);}
 }
}
