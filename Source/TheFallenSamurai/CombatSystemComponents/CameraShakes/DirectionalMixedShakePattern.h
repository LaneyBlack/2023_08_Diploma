// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SimpleCameraShakePattern.h"
#include "PerlinNoiseCameraShakePattern.h"
#include "WaveOscillatorCameraShakePattern.h"
#include "DirectionalMixedShakePattern.generated.h"

/**
 * 
 */
UCLASS()
class THEFALLENSAMURAI_API UDirectionalMixedShakePattern : public USimpleCameraShakePattern
{
public:

	GENERATED_BODY()

	UDirectionalMixedShakePattern(const FObjectInitializer& ObjInit);

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* InterpCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	float RotationAmplitude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	float LocationAmplitudeMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	float LocationFrequencyMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FPerlinNoiseShaker X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FPerlinNoiseShaker Y;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FPerlinNoiseShaker Z;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experimental")
	bool bUseExperimentalPerlin = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experimental")
	FPerlinNoiseShaker PerlinVectorShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experimental")
	bool bUseExperimentalWave = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experimental")
	FWaveOscillator WaveVectorShake;

	UFUNCTION()
	void SetDirectionVectors(const FVector& Direction);

protected:
	virtual void StartShakePatternImpl(const FCameraShakeStartParams& Params) override;
	virtual void UpdateShakePatternImpl(const FCameraShakeUpdateParams& Params, FCameraShakeUpdateResult& OutResult) override;
	virtual void ScrubShakePatternImpl(const FCameraShakeScrubParams& Params, FCameraShakeUpdateResult& OutResult) override;

private:
	float ShakeCurrentTime;

	FVector3f ShakeInitialLocationTimes;
	FVector3f ShakeCurrentLocationTimes;

	FVector ShakeLocalDirection;

	FVector ShakePerpDirection;
	float PerlinVectorTime;
	float WaveVectorTime;

	void UpdateShake(float DeltaTime, FCameraShakeUpdateResult& Result);
};
