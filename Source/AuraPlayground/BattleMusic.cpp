#include "BattleMusic.h"
#include "GameFramework/PlayerController.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Misc/ConfigCacheIni.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
namespace BattleMusic {
namespace { const FName MusicTag(TEXT("BattleThemeMusic")); }
bool Enabled(){bool Value=true;GConfig->GetBool(TEXT("BattleAudio"),TEXT("MusicEnabled"),Value,GGameUserSettingsIni);return Value;}
void Initialize(APlayerController* PC){
 if(!PC||PC->FindComponentByTag<UAudioComponent>(MusicTag))return;
 auto* Sound=LoadObject<USoundWave>(nullptr,TEXT("/Game/BattleForTheA/Audio/S_BattleATLTheme.S_BattleATLTheme"));if(!Sound)return;
 auto* Audio=NewObject<UAudioComponent>(PC,TEXT("BattleThemeMusic"));PC->AddInstanceComponent(Audio);Audio->ComponentTags.Add(MusicTag);
 Audio->bAutoActivate=false;Audio->bAutoDestroy=false;Audio->bIsUISound=true;Audio->bAllowSpatialization=false;Audio->SetSound(Sound);Audio->SetVolumeMultiplier(.28f);Audio->RegisterComponent();
 if(Enabled())Audio->Play();
}
void Toggle(APlayerController* PC){
 const bool Value=!Enabled();GConfig->SetBool(TEXT("BattleAudio"),TEXT("MusicEnabled"),Value,GGameUserSettingsIni);GConfig->Flush(false,GGameUserSettingsIni);
 Initialize(PC);if(auto* Audio=PC?PC->FindComponentByTag<UAudioComponent>(MusicTag):nullptr){Audio->SetPaused(!Value);if(Value&&!Audio->IsPlaying())Audio->Play();}
 UE_LOG(LogTemp,Display,TEXT("BattleMusic: enabled=%d"),Value);
}
void TickAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 static int Phase=0;static float Clock=0;static bool Original=true,Pass=true;Clock+=Dt;if(Clock<1.f||PC->GetWorld()->GetTimeSeconds()<5||Phase>3)return;Clock=0;
 auto Key=[&](){for(bool Down:{true,false})PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::M,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto* Audio=PC->FindComponentByTag<UAudioComponent>(MusicTag);if(!Audio){UE_LOG(LogTemp,Display,TEXT("BattleMusicAudit: {\"passed\":false,\"missing_audio\":true}"));PC->ConsoleCommand(TEXT("quit"));Phase=4;return;}
 UE_LOG(LogTemp,Display,TEXT("BattleMusicState: phase=%d state=%d enabled=%d raw_duration=%.2f"),Phase,int(Audio->GetPlayState()),Enabled(),Audio->Sound?Audio->Sound->Duration:0.f);
 if(Phase==0){Original=Enabled();if(!Original)Key();}
 else if(Phase==1){Pass=Pass&&Audio->GetPlayState()==EAudioComponentPlayState::Playing&&Cast<USoundWave>(Audio->Sound)&&Cast<USoundWave>(Audio->Sound)->bLooping&&FMath::IsNearlyEqual(Cast<USoundWave>(Audio->Sound)->Duration,188.f,1.f);Key();}
 else if(Phase==2){Pass=Pass&&!Enabled()&&Audio->GetPlayState()==EAudioComponentPlayState::Paused;Key();}
 else if(Phase==3){Pass=Pass&&Enabled()&&Audio->GetPlayState()==EAudioComponentPlayState::Playing;if(!Original)Key();Pass=Pass&&Enabled()==Original;UE_LOG(LogTemp,Display,TEXT("BattleMusicAudit: {\"passed\":%s,\"duration_seconds\":188,\"preference_restored\":%s}"),Pass?TEXT("true"):TEXT("false"),Enabled()==Original?TEXT("true"):TEXT("false"));PC->ConsoleCommand(TEXT("quit"));}
 Phase++;
#endif
}

}
