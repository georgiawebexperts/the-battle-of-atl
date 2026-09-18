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
#include "BattleKrogCrash.h"
#include "BattlePolice.h"
#include "BattleKnife.h"
#include "PiedmontPedestrian.h"
#include "BattleCheckpoints.h"
#include "BattleBuild.h"
#include "BattleMacController.h"
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
 const bool FullHUD=Park&&Park->bTutorialHelp;
 const float BuildX=W*.5f+(FullHUD?145.f:105.f)*S,BuildW=(FullHUD?132.f:106.f)*S,BuildH=(FullHUD?38.f:30.f)*S;
 Panel(BuildX,M,BuildW,BuildH);DrawRect(Peach,BuildX,M,BuildW,BuildH);Text(BattleBuild::Label,BuildX+(FullHUD?13.f:10.f)*S,M+(FullHUD?8.f:5.f)*S,FullHUD?19:15,Ink);
 const bool GettingUp=Owner->bCrashActive&&IsValid(Owner->PlayerCrash)&&Owner->PlayerCrash->GetRecoveryPose()!=nullptr;
 const bool HasPhone=Park&&Park->Quest&&Park->Quest->bCollected&&!Park->bTutorialActive;
 const float ObjectiveHeight=FullHUD?(HasPhone?144.f:106.f):56.f;
 Panel(M,M,(FullHUD?470.f:330.f)*S,ObjectiveHeight*S);DrawRect(Peach,M,M,5*S,ObjectiveHeight*S);
 Text(Park&&Park->bTutorialActive?TEXT("FREE RIDE • MIDTOWN"):Park&&Park->Quest&&Park->Quest->bCollected?TEXT("MEET MORGAN"):TEXT("FIND MY LOST PHONE"),M+20*S,M+(FullHUD?15.f:5.f)*S,FullHUD?25:17,Peach);
 FString Objective=Park&&Park->Quest?Park->Quest->SearchDirection(GetOwningPawn()->GetActorLocation()):TEXT("Search the park");
 if(Park&&Park->Quest&&Park->Quest->bCollected)Objective=Park->Quest->NextCheckpoint<2?FString(BattleCheckpoints::Anchors[Park->Quest->NextCheckpoint].Name):TEXT("Through Krog Street Tunnel");
 if(Park&&Park->Quest&&Park->Quest->bCollected&&Park->Quest->NextCheckpoint>=2)for(TActorIterator<ABattleHome> It(GetWorld());It;++It)if(It->bTunnelExited){Objective=TEXT("98 Estoria • patio on the right");break;}
 if(Park&&Park->bTutorialActive)Objective=TEXT("Ride to the 14th Street gateway");
 Text(Objective,M+20*S,M+(FullHUD?60.f:30.f)*S,FullHUD?23:14,Muted);
 if(HasPhone&&FullHUD){
  FString Hint=Park->Quest->RoutePoints.Num()>1?TEXT("Follow the gold route on your watch"):TEXT("Watch route is recalculating...");
  for(TActorIterator<ABattleHome> It(GetWorld());It;++It)if(It->bTunnelExited){Hint=TEXT("Enter the patio to meet Morgan");break;}
  Text(Hint,M+20*S,M+104*S,18,Peach);
 }
 if(Park){
  Panel(W*.5f-(FullHUD?125.f:75.f)*S,M,(FullHUD?250.f:150.f)*S,(FullHUD?106.f:54.f)*S);
  const int Seconds=FMath::CeilToInt(Park->TimeRemaining);
  Center(Park->bTutorialActive?TEXT("UNTIMED"):FString::Printf(TEXT("%02d:%02d"),Seconds/60,Seconds%60),M+(FullHUD?6.f:2.f)*S,Park->bTutorialActive?(FullHUD?34:21):(FullHUD?50:29),Seconds<60?FLinearColor(1,.2,.2):FLinearColor::White);
  if(FullHUD)Center(Park->bTutorialActive?TEXT("Clock starts at gate"):(Person||Owner->bCrashActive)?FString::Printf(TEXT("%s %.2fx"),Person&&Person->bSwimming?TEXT("SWIMMING"):TEXT("ON FOOT"),Park->FootTimeMultiplier):Park->DifficultyName.ToString(),M+66*S,(Park->bTutorialActive||Person||Owner->bCrashActive)?18:22,(Person||Owner->bCrashActive)?Peach:Muted);
  if(Park->TimeNoticeRemaining>0){Panel(W*.5f-230*S,M+116*S,460*S,46*S);Center(FString::Printf(TEXT("%+.0fs  %s"),Park->LastTimeDelta,*Park->TimeNotice),M+121*S,23,Park->LastTimeDelta>0?FLinearColor(.3,1,.65):FLinearColor(1,.35,.3));}
  if(Park->Quest&&!Owner->bCrashActive)Park->Quest->DrawRadar(this,Canvas);
  if(!Park->bTutorialActive&&!Park->bRunEnded&&(Park->StartCountdown>0||Park->RunElapsed<2.5f)){
   const float Y=M+170*S;Panel(W*.5f-220*S,Y,440*S,92*S);
   Center(Park->StartCountdown>0?FString::Printf(TEXT("RIDE STARTED  •  %d"),FMath::CeilToInt(Park->StartCountdown)):TEXT("GO! TIMER RUNNING"),Y+10*S,28,Peach);
   Center(Park->StartCountdown>0?TEXT("Keep riding—controls stay active"):TEXT("Find your phone. Reach Morgan."),Y+52*S,19,Muted);
  }
 }
 if(Park&&Park->bTutorialActive&&FullHUD){
  const float Y=M+177*S;Panel(M,Y,470*S,240*S);
  Text(TEXT("PRACTICE BEFORE YOU START"),M+15*S,Y+12*S,20,Peach);
  Text(FString(Park->PracticeDistance>500?TEXT("✓ "):TEXT("• "))+(Person?TEXT("WASD / arrows: walk   SHIFT: run"):TEXT("WASD / arrows: pedal and steer")),M+15*S,Y+49*S,20,Muted);
  Text(FString(Park->bPracticeBraked?TEXT("✓ "):TEXT("• "))+(Person?TEXT("SPACE: jump   C / CTRL: crouch"):TEXT("Space: brake   Q / R: change gear")),M+15*S,Y+83*S,20,Muted);
  Text(FString(Park->bPracticeDismounted?TEXT("✓ "):TEXT("• "))+TEXT("E: get off / back on the bike"),M+15*S,Y+117*S,20,Muted);
  Text(FString(Park->bPracticeHorn?TEXT("✓ "):TEXT("• "))+(Person?TEXT("G: draw / holster   1–5: weapon"):TEXT("H: horn   J: jump while moving")),M+15*S,Y+151*S,20,Muted);
  Text(TEXT("Enter gate to start. F1: hide help"),M+15*S,Y+197*S,18,Peach);
 }
 if(Park&&Park->ExpansionNoticeRemaining>0){Panel(W*.5f-340*S,H*.48f,680*S,110*S);Center(TEXT("MIDTOWN EXPANSION COMING SOON"),H*.48f+12*S,26,Peach);Center(TEXT("The Battle of ATL • Follow the route to Piedmont Park"),H*.48f+59*S,20);}
 const float StatusW=(FullHUD?300.f:220.f)*S,StatusH=(FullHUD?106.f:66.f)*S,StatusX=W-M-StatusW,StatusTextX=StatusX+(FullHUD?20.f:14.f)*S;
 Panel(StatusX,M,StatusW,StatusH);
 if(Owner->StunRemaining>0){Text(Owner->StunLabel,StatusTextX,M+8*S,FullHUD?28:20,Peach);Text(FString::Printf(TEXT("Recovering  %.1fs"),Owner->StunRemaining),StatusTextX,M+(FullHUD?55.f:36.f)*S,FullHUD?20:15,Muted);}
 else if(Owner->bCrashActive){Text(TEXT("KNOCKED OFF"),W-M-280*S,M+12*S,28,Peach);Text(GettingUp?TEXT("Getting back up"):TEXT("Recovering"),W-M-280*S,M+65*S,23,Muted);}
 else if(Bike){Text(FString::Printf(TEXT("%.0f MPH"),(Bike->Ride->Speed+Bike->Ride->ReverseSpeed)*.0223694f),StatusTextX,M+(FullHUD?12.f:6.f)*S,FullHUD?38:27);Text((Bike->Ride->ReverseSpeed>1?FString::Printf(TEXT("REVERSE  |  GEAR %d"),Bike->Ride->Gear):FString::Printf(TEXT("GEAR %d / 5"),Bike->Ride->Gear)),StatusTextX,M+(FullHUD?65.f:38.f)*S,FullHUD?23:16,Muted);}
 else if(Person->bSwimming){Text(TEXT("SWIMMING"),W-M-280*S,M+12*S,28,Peach);Text(TEXT("Bike stays at the bank"),W-M-280*S,M+55*S,20,Muted);}
 else{Text(Person->ReloadRemaining>0?TEXT("RELOADING"):Person->bWeaponDrawn?BattleWeapons::Name(Person->CurrentWeapon):TEXT("HANDS FREE"),W-M-280*S,M+12*S,28,Peach);if(Person->bWeaponDrawn){Text(FString::Printf(TEXT("%d  LOADED"),Person->Ammo),W-M-280*S,M+46*S,24);Text(FString::Printf(TEXT("%s  SPARE"),*Person->ReserveLabel()),W-M-280*S,M+78*S,18,Muted);}else Text(TEXT("G  DRAW WEAPON"),W-M-280*S,M+55*S,20);}
 // Wanted panel: why the player is wanted, for how much longer, and whether
 // APD has actually been called. Rent-a-cops watch; APD only answers an alert.
 if(auto* Rules=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this))){if(Rules->Trouble>.1f||Rules->PeopleHit>0){const float DangerY=M+(ObjectiveHeight+12)*S;Panel(M,DangerY,560*S,44*S);Text(FString::Printf(TEXT("WANTED %.0fs  |  %s%s"),Rules->WantedSeconds,Rules->WantedReason.IsEmpty()?TEXT("HEAT"):*Rules->WantedReason,Rules->bPoliceAlert?TEXT("  |  APD RESPONDING"):TEXT("  |  RENT-A-COPS WATCHING")),M+12*S,DangerY+10*S,18,Peach);}}
 if(FullHUD&&!Owner->bCrashActive&&!(Person&&Person->bSwimming)){
  Panel(W-M-300*S,M+118*S,300*S,44*S);Text(FString::Printf(TEXT("%sHORN  %d / 5"),Bike?TEXT("H  "):TEXT(""),Owner->HornUses),W-M-280*S,M+126*S,22,Owner->HornUses>0?Muted:Peach);
 }
 const float Bottom=H-M-(FullHUD?156.f:74.f)*S;
 if(FullHUD||Owner->RiderHealth<100||Owner->Ride->BoostRemaining>0){
  const float HealthPanelH=FullHUD?156.f:74.f;Panel(M,Bottom,310*S,HealthPanelH*S);
  Text(FString::Printf(TEXT("HEALTH  %.0f"),Owner->RiderHealth),M+18*S,Bottom+10*S,FullHUD?22:18);
  DrawRect(FLinearColor(.15,.19,.2),M+18*S,Bottom+(FullHUD?50.f:42.f)*S,274*S,(FullHUD?18.f:12.f)*S);DrawRect(Owner->RiderHealth<25?FLinearColor(1,.15,.1):FLinearColor(.3,.9,.62),M+18*S,Bottom+(FullHUD?50.f:42.f)*S,FMath::Clamp(Owner->RiderHealth/100.f,0.f,1.f)*274*S,(FullHUD?18.f:12.f)*S);
  if(FullHUD){Text(Owner->Ride->BoostRemaining>0?FString::Printf(TEXT("SPEED BOOST  %.1fs"),Owner->Ride->BoostRemaining):FString::Printf(TEXT("BOOST  %.0f%%"),Owner->Nitro),M+18*S,Bottom+80*S,20,Muted);DrawRect(FLinearColor(.15,.19,.2),M+18*S,Bottom+119*S,274*S,12*S);DrawRect(FLinearColor(.15,.75,1),M+18*S,Bottom+119*S,Owner->Nitro/100.f*274*S,12*S);}
  if(FullHUD){Text(Owner->Ride->BoostRemaining>0?FString::Printf(TEXT("SPEED BOOST  %.1fs"),Owner->Ride->BoostRemaining):FString::Printf(TEXT("BOOST  %.0f%%   CHARGES %d/3"),Owner->Nitro,FMath::Clamp(Owner->BoostCharges,0,3)),M+18*S,Bottom+80*S,20,Muted);DrawRect(FLinearColor(.15,.19,.2),M+18*S,Bottom+119*S,274*S,12*S);DrawRect(FLinearColor(.15,.75,1),M+18*S,Bottom+119*S,Owner->Nitro/100.f*274*S,12*S);}
 }
 // Keep reverse discoverable even when propulsion speed stays high against a blocker.
 FString Prompt=TEXT("E DISMOUNT  |  S/DOWN BACK UP");
 if(Bike&&Bike->Ride->StuckSeconds>2.5f)Prompt=TEXT("STUCK?  HOLD S / DOWN TO BACK OUT");
 const float MusicY=M+(Bike?230.f:174.f)*S;if(FullHUD&&!Owner->bCrashActive){Panel(W-M-300*S,MusicY,300*S,78*S);Text(BattleMusic::Enabled()?FString::Printf(TEXT("M  MUSIC: SONG %d / 2"),BattleMusic::Selection()):TEXT("M  MUSIC: OFF"),W-M-280*S,MusicY+8*S,18,Muted);Text(TEXT("Cycle: Off > 1 > 2 > Off"),W-M-280*S,MusicY+45*S,16,Muted);}
 FString Help=TEXT("Q/R gears   J jump   SHIFT boost   H horn");
 if(Person){const bool Near=FVector::Dist(Person->GetActorLocation(),Owner->GetActorLocation())<240;Prompt=BattleSpareBikes::Nearest(Person)?TEXT("E  RIDE THIS BIKE"):Near?TEXT("E  GET ON THE BIKE"):TEXT("FIND A BIKE  /  BLUE WATCH MARKERS");Help=Person->bWeaponDrawn?TEXT("G holster  CLICK fire  R reload  F melee"):TEXT("G draw  SHIFT run  SPACE jump  C crouch");}
 if(Person&&Person->bSwimming){Prompt=TEXT("EXPLORE THE LAKE");Help=TEXT("WASD / arrows swim   E remount by bike");}
 if(Owner->bCrashActive){Prompt=GettingUp?TEXT("GETTING BACK UP"):TEXT("KNOCKED OFF YOUR BIKE");Help=TEXT("Recovery is automatic — clock keeps running");}
 if(Owner->StunRemaining>0){Prompt=Owner->StunLabel;Help=FString::Printf(TEXT("Control returns in %.1fs — clock keeps running"),Owner->StunRemaining);}
 if(FullHUD&&Owner->StunRemaining<=0&&!Owner->bCrashActive&&!Person){
  Panel(W*.5f-460*S,H-M-136*S,920*S,136*S);
  Center(TEXT("BIKE  •  W/UP PEDAL  •  S/DOWN BRAKE / REVERSE  •  A/D OR ARROWS STEER"),H-M-124*S,23,Peach);
  Center(TEXT("E dismount  •  Q/R gears  •  J jump  •  SHIFT boost  •  H horn  •  P arcade/realistic"),H-M-80*S,20,Muted);
  Center(TEXT("F1  COMPACT CONTROLS"),H-M-37*S,17,Muted);
 }else if(FullHUD&&Owner->StunRemaining<=0&&!Owner->bCrashActive&&Person){
  Panel(W*.5f-460*S,H-M-136*S,920*S,136*S);
  Center(Person->bSwimming?TEXT("SWIMMING  •  WASD / ARROWS MOVE  •  MOUSE LOOK"):TEXT("ON FOOT  •  WASD / ARROWS MOVE  •  MOUSE LOOK / AIM  •  Z/X TURN  •  T/V LOOK"),H-M-124*S,23,Peach);
  Center(Person->bSwimming?TEXT("E remount near bike  •  SHIFT swim faster"):TEXT("E bike  •  SHIFT run  •  SPACE jump  •  C crouch  •  G gun  •  CLICK fire  •  RMB aim"),H-M-80*S,20,Muted);
  Center(TEXT("F1  COMPACT CONTROLS"),H-M-37*S,17,Muted);
 }else if(Person&&(Person->bSwimming||Person->bWeaponDrawn)&&Owner->StunRemaining<=0&&!Owner->bCrashActive){
  if(Person->bWeaponDrawn&&!Person->bSwimming)Help=TEXT("G holster   CLICK fire   R reload   F melee");
  Panel(W*.5f-300*S,H-M-50*S,600*S,50*S);Center(Help,H-M-38*S,21,Muted);
 }else if(Owner->bCrashActive||Owner->StunRemaining>0||(Person&&FVector::Dist(Person->GetActorLocation(),Owner->GetActorLocation())<240)){
  Panel(W*.5f-300*S,H-M-100*S,600*S,100*S);
  Center(Prompt,H-M-88*S,29,Peach);Center(Help,H-M-43*S,21,Muted);
 }
 if(!FullHUD&&Owner->StunRemaining<=0&&!Owner->bCrashActive&&Park&&Park->RunElapsed<12.f)Text(TEXT("F1  HELP"),W-M-78*S,M+75*S,13,FLinearColor(.75,.78,.82,.8));
 if(FullHUD&&Bike&&!Owner->bCrashActive){Panel(M,Bottom-82*S,310*S,74*S);Text(FString::Printf(TEXT("PISTOL  %d LOADED"),Bike->PistolAmmo),M+18*S,Bottom-74*S,23,Peach);Text(FString::Printf(TEXT("%d SPARE"),Bike->Inventory[0].Reserve),M+18*S,Bottom-39*S,19,Muted);}
 const float CX=W*.5f,CY=H*.5f;FLinearColor Aim=FLinearColor::White;const ABattleDrone* AimedDrone=nullptr;
 if(Person&&Person->bWeaponDrawn&&!Owner->bCrashActive&&!Person->bSwimming){
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
 else if(const auto* Controller=Cast<ABattleMacController>(GetOwningPlayerController());Controller&&Controller->AimNoticeRemaining>0)Notice=Controller->AimNotice;
 if(!Owner->bCrashActive&&Notice.IsEmpty())for(TActorIterator<ABattleKnife> It(GetWorld());It;++It)if(!It->bDead&&!It->bEscaped&&FVector::Dist2D(It->GetActorLocation(),GetOwningPawn()->GetActorLocation())<1400){Notice=It->bWindingUp?TEXT("KNIFE STRIKE — MOVE!"):It->Stabs>0?TEXT("KNIFE CHASE — RUN, DEFEND OR REMOUNT"):TEXT("KNIFE ATTACKER — RUN OR G TO DRAW");break;}
 // The Murder K rent-a-cops heckle in world text above their heads; repeat the
 // coward call on the HUD so it reads even when the officer is behind you.
 if(!Owner->bCrashActive&&Notice.IsEmpty())for(TActorIterator<ABattlePolice> It(GetWorld());It;++It)if(It->bYellingCoward&&It->TauntVisible>0){Notice=TEXT("RENT-A-COP: COWARD!");break;}
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
  const bool Locked=It->WarningRemaining<=ABattlePolice::TaserAimLockSeconds;
  Notice=Locked?FString::Printf(TEXT("TASER AIM LOCKED  |  %.1fs  |  DODGE SIDEWAYS"),FMath::Max(0.f,It->WarningRemaining)):FString::Printf(TEXT("APD TASER  |  %.1fs  |  KEEP MOVING"),FMath::Max(0.f,It->WarningRemaining));
  FVector2D Label;
  if(GetOwningPlayerController()->ProjectWorldLocationToScreen(It->GetActorLocation()+FVector(0,0,115),Label)&&Label.X>130*S&&Label.X<W-130*S&&Label.Y>180*S&&Label.Y<H*.60f){
   Panel(Label.X-120*S,Label.Y-38*S,240*S,42*S);
   Text(TEXT("STOP! APD"),Label.X-108*S,Label.Y-34*S,21,Peach);
   DrawRect(FLinearColor(.15,.19,.2),Label.X-108*S,Label.Y-5*S,216*S,4*S);
   DrawRect(Locked?FLinearColor(.2,.65,1):Peach,Label.X-108*S,Label.Y-5*S,216*S*FMath::Clamp(It->WarningRemaining/ABattlePolice::TaserWarningSeconds,0.f,1.f),4*S);
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
 // Contextual hints: pop up when they matter and fade away again.
 if(auto* Rules=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this)))if(Rules->HintRemaining>0&&!Rules->HintText.IsEmpty()){
  Panel(CX-430*S,H*.24f,860*S,54*S);Center(Rules->HintText,H*.24f+11*S,23,Peach);
 }
 // A pedestrian the rider just ran into shouts at them. This slot is drawn even
 // while the crash overlay is up, or the line would never be seen in realistic
 // handling where a body impact throws the rider off.
 if(auto* Viewer=GetOwningPawn()){
  const APiedmontPedestrian* Cursing=nullptr;float CurseDistance=2600;
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)if(It->CurseRemaining>0&&!It->CurseLine.IsEmpty()){const float D=FVector::Dist2D(Viewer->GetActorLocation(),It->GetActorLocation());if(D<CurseDistance){CurseDistance=D;Cursing=*It;}}
  if(Cursing){Panel(CX-440*S,H-M-166*S,880*S,48*S);Center(FString::Printf(TEXT("PEDESTRIAN: %s"),*Cursing->CurseLine),H-M-159*S,24,FLinearColor(1,.8f,.7f));}
  if(!Owner->bCrashActive&&!Cursing){
  const ABattleZombie* Speaking=nullptr;float Best=2500;
  for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)if(It->SubtitleRemaining>0){const float D=FVector::Dist2D(Viewer->GetActorLocation(),It->GetActorLocation());if(D<Best){Best=D;Speaking=*It;}}
  if(Speaking){Panel(CX-440*S,H-M-166*S,880*S,48*S);Center(Speaking->CharacterName()+TEXT(": ")+Speaking->Subtitle,H-M-159*S,24);}
  const ABattleKrogCrash* Crash=nullptr;
  if(!Speaking)for(TActorIterator<ABattleKrogCrash> It(GetWorld());It;++It)if(It->ShoutRemaining>0){const float D=FVector::Dist2D(Viewer->GetActorLocation(),It->GetActorLocation());if(D<Best){Best=D;Crash=*It;}}
  if(!Speaking&&Crash){Panel(CX-440*S,H-M-166*S,880*S,48*S);Center(FString::Printf(TEXT("BYSTANDER: %s"),*Crash->ShoutText),H-M-159*S,24,FLinearColor(1.f,.72f,.62f));}
  }
 }
}
