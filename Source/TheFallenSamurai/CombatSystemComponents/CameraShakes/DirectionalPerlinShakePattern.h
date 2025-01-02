// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PerlinNoiseCameraShakePattern.h"
#include "DirectionalPerlinShakePattern.generated.h"

//class UCurveFloat;

UCLASS()
class THEFALLENSAMURAI_API UDirectionalPerlinShakePattern : public UPerlinNoiseCameraShakePattern
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Curves")
	UCurveFloat* InterpCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	float RotationAmplitude;
	
	FVector ShakeLocalDirection;

protected:
	virtual void StartShakePatternImpl(const FCameraShakeStartParams& Params) override;
	virtual void UpdateShakePatternImpl(const FCameraShakeUpdateParams& Params, FCameraShakeUpdateResult& OutResult) override;
	virtual void ScrubShakePatternImpl(const FCameraShakeScrubParams& Params, FCameraShakeUpdateResult& OutResult) override;

private:
	float CurveCurrentTime;

	void UpdateShake(float DeltaTime, FCameraShakeUpdateResult& Result);
};
