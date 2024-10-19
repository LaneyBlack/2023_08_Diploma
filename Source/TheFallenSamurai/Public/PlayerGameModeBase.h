// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/GameModeBase.h"
#include "Components/PostProcessComponent.h"
#include "Engine/PostProcessVolume.h"

#include "PlayerGameModeBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGlobalRewindStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGlobalRewindCompleted);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGlobalTimeScrubStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGlobalTimeScrubCompleted);

UCLASS()
class APlayerGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	APlayerGameModeBase();

	UPROPERTY(BlueprintReadWrite, Category = "Summary Data")
	int32 PlayerDeaths = 0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewind")
	float SlowestRewindSpeed = 0.25f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewind")
	float SlowerRewindSpeed = 0.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewind")
	float NormalRewindSpeed = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewind")
	float FasterRewindSpeed = 2.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewind")
	float FastestRewindSpeed = 4.0f;
	
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	void StartGlobalRewind();
	
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	void StopGlobalRewind();
	
	void SetRewindSpeedSlowest();
	
	void SetRewindSpeedSlower();
	
	void SetRewindSpeedNormal();
	
	void SetRewindSpeedFaster();
	
	void SetRewindSpeedFastest();

	UFUNCTION(BlueprintCallable, Category = "Rewind")
	void ToggleTimeScrub();

	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnGlobalTimeScrubStarted OnGlobalTimeScrubStarted;

	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnGlobalTimeScrubCompleted OnGlobalTimeScrubCompleted;

	// Event for when global rewinds start
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnGlobalRewindStarted OnGlobalRewindStarted;

	// Event for when global rewinds stop
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnGlobalRewindCompleted OnGlobalRewindCompleted;

	// Desired length of longest rewind; used to compute the rewind buffer size
	UPROPERTY(EditDefaultsOnly, Category = "Rewind")
	float MaxRewindSeconds = 20.0f;

private:
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind")
	bool bIsGlobalTimeScrubbing = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	bool IsGlobalTimeScrubbing() const { return bIsGlobalTimeScrubbing; };
	
private:
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind")
	bool bIsGlobalRewinding = false;

public:
	// Returns whether rewinding is currently enabled
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	bool IsGlobalRewinding() const { return bIsGlobalRewinding; };

private:
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind")
	float GlobalRewindSpeed = 1.0f;

public:
	// Returns the current global time dilation
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	float GetGlobalRewindSpeed() const { return GlobalRewindSpeed; }

};
