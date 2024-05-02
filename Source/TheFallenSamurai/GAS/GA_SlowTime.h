// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_SlowTime.generated.h"

/**
 * 
 */
UCLASS()
class THEFALLENSAMURAI_API UGA_SlowTime : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SlowTime();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	void EndAbilityTimer();

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	FTimerHandle TimerHandle;
	void ActivateSlowMotion();
	void DeactivateSlowMotion();
};
