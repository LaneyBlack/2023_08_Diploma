#include "MyGameUserSettings.h"

UMyGameUserSettings::UMyGameUserSettings(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer),
      MasterVolume(1.0f),
      MusicVolume(1.0f),
      SFXVolume(1.0f),
      UIVolume(1.0f),
      CameraSensitivity(1.0f),
      GraphicsQuality(EGraphicsQuality::High)
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

void UMyGameUserSettings::SetCameraSensitivity(float Sensitivity)
{
    CameraSensitivity = FMath::Clamp(Sensitivity, 0.0f, 1.0f);
}

float UMyGameUserSettings::GetCameraSensitivity() const
{
    return CameraSensitivity;
}

void UMyGameUserSettings::SetGraphicsQuality(EGraphicsQuality Quality)
{
    GraphicsQuality = Quality;
}

EGraphicsQuality UMyGameUserSettings::GetGraphicsQuality() const
{
    return GraphicsQuality;
}

int32 UMyGameUserSettings::GetGraphicsQualityIndex() const
{
    return static_cast<int32>(GraphicsQuality);
}

void UMyGameUserSettings::SetGraphicsQualityByIndex(int32 Index)
{
    if (Index >= 0 && Index < static_cast<int32>(EGraphicsQuality::Cinematic) + 1)
    {
        GraphicsQuality = static_cast<EGraphicsQuality>(Index);
    }
}

UMyGameUserSettings* UMyGameUserSettings::GetMyGameUserSettings()
{
    return Cast<UMyGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}