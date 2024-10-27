#include "MyGameUserSettings.h"

UMyGameUserSettings::UMyGameUserSettings(const FObjectInitializer& ObjectInitializer) 
    : Super(ObjectInitializer),
      MasterVolume(1.0f),
      MusicVolume(1.0f),
      SFXVolume(1.0f),
      UIVolume(1.0f)
{
}

void UMyGameUserSettings::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

float UMyGameUserSettings::GetMasterVolume() const
{
    return MasterVolume;
}

void UMyGameUserSettings::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

float UMyGameUserSettings::GetMusicVolume() const
{
    return MusicVolume;
}

void UMyGameUserSettings::SetSFXVolume(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

float UMyGameUserSettings::GetSFXVolume() const
{
    return SFXVolume;
}

void UMyGameUserSettings::SetUIVolume(float Volume)
{
    UIVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

float UMyGameUserSettings::GetUIVolume() const
{
    return UIVolume;
}

UMyGameUserSettings* UMyGameUserSettings::GetMyGameUserSettings()
{
    return Cast<UMyGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}
