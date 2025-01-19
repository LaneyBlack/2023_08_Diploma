// Fill out your copyright notice in the Description page of Project Settings.


#include "DirectionalMixedShakePattern.h"
#include "Kismet/KismetSystemLibrary.h"

#define PRINT(mess, mtime)  GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::Green, TEXT(mess));
#define PRINTC(mess, color)  GEngine->AddOnScreenDebugMessage(-1, 0.33, color, TEXT(mess));
#define PRINT_F(prompt, mess, mtime) GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::White, FString::Printf(TEXT(prompt), mess));
#define PRINTC_F(prompt, mess, mtime, color) GEngine->AddOnScreenDebugMessage(-1, mtime, color, FString::Printf(TEXT(prompt), mess));
#define PRINT_B(prompt, mess) GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Magenta, FString::Printf(TEXT(prompt), mess ? TEXT("TRUE") : TEXT("FALSE")));

float UDirectionalMixedShakePattern::ShakeGlobalIntensity = 1.f;

UDirectionalMixedShakePattern::UDirectionalMixedShakePattern(const FObjectInitializer& ObjInit)
	: USimpleCameraShakePattern(ObjInit) {}

void UDirectionalMixedShakePattern::StartShakePatternImpl(const FCameraShakeStartParams& Params)
{
	USimpleCameraShakePattern::StartShakePatternImpl(Params);

	ShakeCurrentTime = 0.f;
	WaveShakeData.Initialize(WaveVectorTime);
	CurrentShakeData = &CustomMovementData[FMath::RandRange(0, CustomMovementData.Num() - 1)];

	PRINTC_F("Curve picked = %s", *UKismetSystemLibrary::GetDisplayName(CurrentShakeData->InterpCurve), 5, FColor::Cyan);
}

void UDirectionalMixedShakePattern::UpdateShakePatternImpl(const FCameraShakeUpdateParams& Params, FCameraShakeUpdateResult& OutResult)
{
	UpdateShake(Params.DeltaTime, OutResult);

	const float BlendWeight = State.Update(Params.DeltaTime);
	OutResult.ApplyScale(BlendWeight);
}

void UDirectionalMixedShakePattern::ScrubShakePatternImpl(const FCameraShakeScrubParams& Params, FCameraShakeUpdateResult& OutResult)
{
	ShakeCurrentTime = 0.f;

	UpdateShake(Params.AbsoluteTime, OutResult);

	const float BlendWeight = State.Scrub(Params.AbsoluteTime);
	OutResult.ApplyScale(BlendWeight);
}

void UDirectionalMixedShakePattern::UpdateShake(float DeltaTime, FCameraShakeUpdateResult& OutResult)
{
	ShakeCurrentTime += DeltaTime;

	float CurveCurrentTime = ShakeCurrentTime / Duration;

	OutResult.Rotation.Yaw =	CurrentShakeData->InterpCurve->GetFloatValue(CurveCurrentTime) * CurrentShakeData->RotationAmplitude * ShakeLocalDirection.Y;
	OutResult.Rotation.Pitch =	CurrentShakeData->InterpCurve->GetFloatValue(CurveCurrentTime) * CurrentShakeData->RotationAmplitude * ShakeLocalDirection.Z;

	OutResult.Location = WaveShakeData.Update(DeltaTime, 1.f, 1.f, WaveVectorTime) * ShakePerpDirection;
	OutResult.Location.X = 0.f;

	OutResult.Rotation *= ShakeGlobalIntensity;
	OutResult.Location *= ShakeGlobalIntensity;
}

void UDirectionalMixedShakePattern::SetDirectionVectors(const FVector& Direction)
{
	ShakeLocalDirection = Direction;
	ShakePerpDirection = FVector(Direction.X, -Direction.Z, Direction.X);
}