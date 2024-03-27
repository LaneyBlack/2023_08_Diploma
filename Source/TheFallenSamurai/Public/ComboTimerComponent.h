// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComboTimerComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THEFALLENSAMURAI_API UComboTimerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UComboTimerComponent();

	UFUNCTION(BlueprintCallable, Category = "ComboSystem")
	float GetRemainingComboTime() const;

	UFUNCTION(BlueprintCallable, Category = "ComboSystem")
	float GetTotalComboTime() const;
	
	void StartComboStateTimer();

protected:
	virtual void BeginPlay() override;

private:
	FTimerHandle ComboTimerHandle;
	FTimerHandle KillStreakTimerHandle;
	FTimerHandle ComboStateTimerHandle;
	void StartComboTimer();
	void StartKillStreakTimer();
	void OnComboTimerEnd();
	void OnKillStreakTimerEnd();
	void OnKillIncreased();
	void OnComboStateTimerEnd();
};
