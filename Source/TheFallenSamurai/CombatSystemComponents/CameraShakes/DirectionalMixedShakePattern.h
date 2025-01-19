// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SimpleCameraShakePattern.h"
#include "PerlinNoiseCameraShakePattern.h"
#include "WaveOscillatorCameraShakePattern.h"
#include "DirectionalMixedShakePattern.generated.h"


USTRUCT(BlueprintType)
struct FCurveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* InterpCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationAmplitude = 11.f;
};

UCLASS()
class THEFALLENSAMURAI_API UDirectionalMixedShakePattern : public USimpleCameraShakePattern
{
public:

	GENERATED_BODY()

	UDirectionalMixedShakePattern(const FObjectInitializer& ObjInit);

	//UPROPERTY(BlueprintReadWrite)
	static float ShakeGlobalIntensity;

	UFUNCTION(BlueprintCallable)
	static void SetShakeIntensity(float Intensity)
	{
		ShakeGlobalIntensity = Intensity;
	}

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement")
	TArray<FCurveData> CustomMovementData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWaveOscillator WaveShakeData;

	UFUNCTION()
	void SetDirectionVectors(const FVector& Direction);

protected:
	virtual void StartShakePatternImpl(const FCameraShakeStartParams& Params) override;
	virtual void UpdateShakePatternImpl(const FCameraShakeUpdateParams& Params, FCameraShakeUpdateResult& OutResult) override;
	virtual void ScrubShakePatternImpl(const FCameraShakeScrubParams& Params, FCameraShakeUpdateResult& OutResult) override;

private:
	float ShakeCurrentTime;

	FCurveData* CurrentShakeData;

	FVector ShakeLocalDirection;

	FVector ShakePerpDirection;

	float WaveVectorTime;

	void UpdateShake(float DeltaTime, FCameraShakeUpdateResult& Result);
};