// Fill out your copyright notice in the Description page of Project Settings.


#include "DirectionalMixedShakePattern.h"

#define PRINT(mess, mtime)  GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::Green, TEXT(mess));
#define PRINTC(mess, color)  GEngine->AddOnScreenDebugMessage(-1, 0.33, color, TEXT(mess));
#define PRINT_F(prompt, mess, mtime) GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::White, FString::Printf(TEXT(prompt), mess));
#define PRINTC_F(prompt, mess, mtime, color) GEngine->AddOnScreenDebugMessage(-1, mtime, color, FString::Printf(TEXT(prompt), mess));
#define PRINT_B(prompt, mess) GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Magenta, FString::Printf(TEXT(prompt), mess ? TEXT("TRUE") : TEXT("FALSE")));


UDirectionalMixedShakePattern::UDirectionalMixedShakePattern(const FObjectInitializer& ObjInit)
	: USimpleCameraShakePattern(ObjInit) {}

void UDirectionalMixedShakePattern::SetDirectionVectors(const FVector& Direction)
{
	ShakeLocalDirection = Direction;
	ShakePerpDirection = FVector(Direction.X, -Direction.Z, Direction.X);
}

void UDirectionalMixedShakePattern::StartShakePatternImpl(const FCameraShakeStartParams& Params)
{
	USimpleCameraShakePattern::StartShakePatternImpl(Params);

	//CurrentRotationOffset = FVector3f(0.f);
	//PRINT_F("StartShakePatternImpl, vector = %s", *ShakeLocalDirection.ToCompactString(), 5);
	ShakeCurrentTime = 0.f;

	if (!Params.bIsRestarting)
	{
		ShakeInitialLocationTimes = FVector3f((float)FMath::RandHelper(255), (float)FMath::RandHelper(255), (float)FMath::RandHelper(255));

		ShakeCurrentLocationTimes = ShakeInitialLocationTimes;
	}
}

void UDirectionalMixedShakePattern::UpdateShakePatternImpl(const FCameraShakeUpdateParams& Params, FCameraShakeUpdateResult& OutResult)
{
	//PRINT("UpdateShakePatternImpl", 5);
	//PRINT_F("UpdateShakePatternImpl, vector = %s", *ShakeLocalDirection.ToCompactString(), 2);

	UpdateShake(Params.DeltaTime, OutResult);

	const float BlendWeight = State.Update(Params.DeltaTime);
	OutResult.ApplyScale(BlendWeight);
}

void UDirectionalMixedShakePattern::ScrubShakePatternImpl(const FCameraShakeScrubParams& Params, FCameraShakeUpdateResult& OutResult)
{
	//PRINT("ScrubShakePatternImpl", 5);

	ShakeCurrentTime = 0.f;
	ShakeCurrentLocationTimes = ShakeInitialLocationTimes;

	UpdateShake(Params.AbsoluteTime, OutResult);

	const float BlendWeight = State.Scrub(Params.AbsoluteTime);
	OutResult.ApplyScale(BlendWeight);
}

void UDirectionalMixedShakePattern::UpdateShake(float DeltaTime, FCameraShakeUpdateResult& OutResult)
{
	ShakeCurrentTime += DeltaTime;

	float CurveCurrentTime = ShakeCurrentTime / Duration;
	OutResult.Rotation.Yaw = InterpCurve->GetFloatValue(CurveCurrentTime) * RotationAmplitude * ShakeLocalDirection.Y;
	OutResult.Rotation.Pitch = InterpCurve->GetFloatValue(CurveCurrentTime) * RotationAmplitude * ShakeLocalDirection.Z;

	OutResult.Location.X = X.Update(DeltaTime, LocationAmplitudeMultiplier, LocationFrequencyMultiplier, ShakeCurrentLocationTimes.X);
	OutResult.Location.Y = Y.Update(DeltaTime, LocationAmplitudeMultiplier, LocationFrequencyMultiplier, ShakeCurrentLocationTimes.Y);
	OutResult.Location.Z = Z.Update(DeltaTime, LocationAmplitudeMultiplier, LocationFrequencyMultiplier, ShakeCurrentLocationTimes.Z);
}