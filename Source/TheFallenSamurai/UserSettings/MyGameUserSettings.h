// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "Sound/SoundClass.h"
#include "MyGameUserSettings.generated.h"

/**
 * 
 */
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
	static UMyGameUserSettings* GetMyGameUserSettings();

protected:
	UPROPERTY(config, BlueprintReadWrite)
	float MasterVolume;

	UPROPERTY(config, BlueprintReadWrite)
	float MusicVolume;

	UPROPERTY(config, BlueprintReadWrite)
	float SFXVolume;

	UPROPERTY(config, BlueprintReadWrite)
	float UIVolume;

};
