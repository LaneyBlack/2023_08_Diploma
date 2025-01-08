// Fill out your copyright notice in the Description page of Project Settings.


#include "DirectionalPerlinShakePattern.h"

#define PRINT(mess, mtime)  GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::Green, TEXT(mess));
#define PRINTC(mess, color)  GEngine->AddOnScreenDebugMessage(-1, 0.33, color, TEXT(mess));
#define PRINT_F(prompt, mess, mtime) GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::White, FString::Printf(TEXT(prompt), mess));
#define PRINTC_F(prompt, mess, mtime, color) GEngine->AddOnScreenDebugMessage(-1, mtime, color, FString::Printf(TEXT(prompt), mess));
#define PRINT_B(prompt, mess) GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Magenta, FString::Printf(TEXT(prompt), mess ? TEXT("TRUE") : TEXT("FALSE")));


void UDirectionalPerlinShakePattern::StartShakePatternImpl(const FCameraShakeStartParams& Params)
{
	Super::Super::StartShakePatternImpl(Params);

	//CurrentRotationOffset = FVector3f(0.f);
	//PRINT_F("StartShakePatternImpl, vector = %s", *ShakeLocalDirection.ToCompactString(), 5);
	CurveCurrentTime = 0.f;
}

void UDirectionalPerlinShakePattern::UpdateShakePatternImpl(const FCameraShakeUpdateParams& Params, FCameraShakeUpdateResult& OutResult)
{
	//PRINT("UpdateShakePatternImpl", 5);
	//PRINT_F("UpdateShakePatternImpl, vector = %s", *ShakeLocalDirection.ToCompactString(), 2);

	UpdateShake(Params.DeltaTime, OutResult);

	const float BlendWeight = State.Update(Params.DeltaTime);
	OutResult.ApplyScale(BlendWeight);
}

void UDirectionalPerlinShakePattern::ScrubShakePatternImpl(const FCameraShakeScrubParams& Params, FCameraShakeUpdateResult& OutResult)
{
	//PRINT("ScrubShakePatternImpl", 5);

	CurveCurrentTime = 0.f;

	UpdateShake(Params.AbsoluteTime, OutResult);

	const float BlendWeight = State.Scrub(Params.AbsoluteTime);
	OutResult.ApplyScale(BlendWeight);
}

void UDirectionalPerlinShakePattern::UpdateShake(float DeltaTime, FCameraShakeUpdateResult& OutResult)
{
	CurveCurrentTime += DeltaTime / Duration;
	//OutResult.Rotation.Roll = InterpCurve->GetFloatValue(CurveCurrentTime) * RotationAmplitude * ShakeLocalDirection.X;
	OutResult.Rotation.Yaw = InterpCurve->GetFloatValue(CurveCurrentTime) * RotationAmplitude * ShakeLocalDirection.Y;
	OutResult.Rotation.Pitch = InterpCurve->GetFloatValue(CurveCurrentTime) * RotationAmplitude * ShakeLocalDirection.Z;


}
