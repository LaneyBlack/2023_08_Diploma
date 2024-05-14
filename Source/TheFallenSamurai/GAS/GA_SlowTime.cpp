// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_SlowTime.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

UGA_SlowTime::UGA_SlowTime()
{
}

void UGA_SlowTime::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ActivateSlowMotion();
	
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UGA_SlowTime::EndAbilityTimer, 10.0f, false);
}

void UGA_SlowTime::EndAbilityTimer()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_SlowTime::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	
	DeactivateSlowMotion();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}


void UGA_SlowTime::ActivateSlowMotion()
{
	AActor* Owner = GetAvatarActorFromActorInfo();
	if (Owner == nullptr)
	{
		return;
	}
	
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (*ActorItr != Owner)
		{
			ActorItr->CustomTimeDilation = 0.2f;
		}
		else
		{
			Owner->CustomTimeDilation = 1.0f;
		}
	}
}

void UGA_SlowTime::DeactivateSlowMotion()
{
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ActorItr->CustomTimeDilation = 1.0f;
	}
}


