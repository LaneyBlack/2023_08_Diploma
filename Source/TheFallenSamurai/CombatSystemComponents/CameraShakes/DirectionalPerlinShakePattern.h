// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PerlinNoiseCameraShakePattern.h"
#include "DirectionalPerlinShakePattern.generated.h"

/**
 * 
 */
UCLASS()
class THEFALLENSAMURAI_API UDirectionalPerlinShakePattern : public UPerlinNoiseCameraShakePattern
{
	GENERATED_BODY()
	
protected:
	virtual void StartShakePatternImpl(const FCameraShakeStartParams& Params) override;
	virtual void UpdateShakePatternImpl(const FCameraShakeUpdateParams& Params, FCameraShakeUpdateResult& OutResult) override;
	virtual void ScrubShakePatternImpl(const FCameraShakeScrubParams& Params, FCameraShakeUpdateResult& OutResult) override;
};
