// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "Sound/SoundClass.h"
#include "MyGameUserSettings.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EGraphicsQuality : uint8
{
    Low        UMETA(DisplayName = "Low", Value = 0),
    Medium     UMETA(DisplayName = "Medium", Value = 1),
    High       UMETA(DisplayName = "High", Value = 2),
    Epic       UMETA(DisplayName = "Epic", Value = 3),
    Cinematic  UMETA(DisplayName = "Cinematic", Value = 4),
    Custom UMETA(DisplayName = "Custom", Value = 5)
};

UCLASS()
class THEFALLENSAMURAI_API UMyGameUserSettings : public UGameUserSettings
{
    GENERATED_UCLASS_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintPure)
    float GetMasterVolume() const;

    UFUNCTION(BlueprintCallable)
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintPure)
    float GetMusicVolume() const;

    UFUNCTION(BlueprintCallable)
    void SetSFXVolume(float Volume);

    UFUNCTION(BlueprintPure)
    float GetSFXVolume() const;

    UFUNCTION(BlueprintCallable)
    void SetUIVolume(float Volume);

    UFUNCTION(BlueprintPure)
    float GetUIVolume() const;

    UFUNCTION(BlueprintCallable)
    void SetCameraSensitivity(float Sensitivity);

    UFUNCTION(BlueprintPure)
    float GetCameraSensitivity() const;

    UFUNCTION(BlueprintCallable)
    void SetGraphicsQuality(EGraphicsQuality Quality);

    UFUNCTION(BlueprintPure)
    EGraphicsQuality GetGraphicsQuality() const;

    UFUNCTION(BlueprintCallable)
    static UMyGameUserSettings* GetMyGameUserSettings();

    UFUNCTION(BlueprintPure, Category = "GraphicsQuality")
    int32 GetGraphicsQualityIndex() const;

    UFUNCTION(BlueprintCallable, Category = "GraphicsQuality")
    void SetGraphicsQualityByIndex(int32 Index);

protected:
    UPROPERTY(config, BlueprintReadWrite)
    float CameraSensitivity;

    UPROPERTY(config, BlueprintReadWrite)
    float MasterVolume;

    UPROPERTY(config, BlueprintReadWrite)
    float MusicVolume;

    UPROPERTY(config, BlueprintReadWrite)
    float SFXVolume;

    UPROPERTY(config, BlueprintReadWrite)
    float UIVolume;

    UPROPERTY(config, BlueprintReadWrite)
    EGraphicsQuality GraphicsQuality;
};