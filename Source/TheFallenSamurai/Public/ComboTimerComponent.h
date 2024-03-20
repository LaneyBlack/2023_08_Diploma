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

protected:
	virtual void BeginPlay() override;

private:
	FTimerHandle ComboTimerHandle;
	void StartComboTimer();
	void OnComboTimerEnd();
	void OnKillIncreased();
};

