#include "BattleMusic.h"
#include "GameFramework/PlayerController.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Misc/ConfigCacheIni.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
namespace BattleMusic {
namespace { const FName MusicTag(TEXT("BattleThemeMusic")); }
int Selection(){int Value=-1;GConfig->GetInt(TEXT("BattleAudio"),TEXT("MusicSelection"),Value,GGameUserSettingsIni);if(Value<0){bool Legacy=true;GConfig->GetBool(TEXT("BattleAudio"),TEXT("MusicEnabled"),Legacy,GGameUserSettingsIni);Value=Legacy?1:0;}return FMath::Clamp(Value,0,2);}
bool Enabled(){return Selection()>0;}
namespace {
void Apply(APlayerController* PC){
 auto* Audio=PC?PC->FindComponentByTag<UAudioComponent>(MusicTag):nullptr;if(!Audio)return;
 Audio->Stop();const int Track=Selection();if(Track==0)return;
 auto* Sound=LoadObject<USoundWave>(nullptr,Track==1?TEXT("/Game/BattleForTheA/Audio/S_BattleATLTheme.S_BattleATLTheme"):TEXT("/Game/BattleForTheA/Audio/S_BattleATLTheme2.S_BattleATLTheme2"));
 if(Sound){Audio->SetSound(Sound);Audio->Play();}
}
}
void Initialize(APlayerController* PC){
 if(!PC||PC->FindComponentByTag<UAudioComponent>(MusicTag))return;
 auto* Audio=NewObject<UAudioComponent>(PC,TEXT("BattleThemeMusic"));PC->AddInstanceComponent(Audio);Audio->ComponentTags.Add(MusicTag);
 Audio->bAutoActivate=false;Audio->bAutoDestroy=false;Audio->bIsUISound=true;Audio->bAllowSpatialization=false;Audio->SetVolumeMultiplier(.28f);Audio->RegisterComponent();Apply(PC);
}
void Toggle(APlayerController* PC){
 const int Track=(Selection()+1)%3;GConfig->SetInt(TEXT("BattleAudio"),TEXT("MusicSelection"),Track,GGameUserSettingsIni);GConfig->SetBool(TEXT("BattleAudio"),TEXT("MusicEnabled"),Track>0,GGameUserSettingsIni);GConfig->Flush(false,GGameUserSettingsIni);
 Initialize(PC);Apply(PC);UE_LOG(LogTemp,Display,TEXT("BattleMusic: track=%d (0=off)"),Track);
}
void TickAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 static int Phase=0,Original=0;static float Clock=0;static bool Pass=true;Clock+=Dt;if(Clock<1.f||PC->GetWorld()->GetTimeSeconds()<5||Phase>3)return;Clock=0;
 auto Key=[&](){for(bool Down:{true,false})PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::M,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto* Audio=PC->FindComponentByTag<UAudioComponent>(MusicTag);if(!Audio){UE_LOG(LogTemp,Display,TEXT("BattleMusicAudit: {\"passed\":false,\"missing_audio\":true}"));PC->ConsoleCommand(TEXT("quit"));Phase=4;return;}
 UE_LOG(LogTemp,Display,TEXT("BattleMusicState: phase=%d selection=%d state=%d sound=%s"),Phase,Selection(),int(Audio->GetPlayState()),*GetNameSafe(Audio->Sound));
 if(Phase==0){Original=Selection();for(int I=0;I<3&&Selection()!=0;I++)Toggle(PC);Key();}
 else if(Phase==1||Phase==2){auto* Wave=Cast<USoundWave>(Audio->Sound);Pass=Pass&&Selection()==Phase&&Audio->GetPlayState()==EAudioComponentPlayState::Playing&&Wave&&Wave->bLooping&&Wave->Duration>30&&Wave->GetName()==(Phase==1?TEXT("S_BattleATLTheme"):TEXT("S_BattleATLTheme2"));Key();}
 else if(Phase==3){Pass=Pass&&Selection()==0&&Audio->GetPlayState()==EAudioComponentPlayState::Stopped;for(int I=0;I<3&&Selection()!=Original;I++)Toggle(PC);Pass=Pass&&Selection()==Original;UE_LOG(LogTemp,Display,TEXT("BattleMusicAudit: {\"passed\":%s,\"tracks\":2,\"cycle\":\"off-song1-song2-off\",\"preference_restored\":%s}"),Pass?TEXT("true"):TEXT("false"),Selection()==Original?TEXT("true"):TEXT("false"));PC->ConsoleCommand(TEXT("quit"));}
 Phase++;
#endif
}

}
