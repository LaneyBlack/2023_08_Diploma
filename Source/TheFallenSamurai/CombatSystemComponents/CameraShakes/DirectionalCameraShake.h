// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "DirectionalMixedShakePattern.h"
#include "DirectionalCameraShake.generated.h"

/**
 * 
 */
UCLASS()
class THEFALLENSAMURAI_API UDirectionalCameraShake : public UCameraShakeBase
{
	GENERATED_BODY()

public:
	void SetSwingVector(const FVector& InSwingDirection)
	{
		UDirectionalMixedShakePattern* pattern = Cast<UDirectionalMixedShakePattern>(GetRootShakePattern());
		pattern->SetDirectionVectors(InSwingDirection);
	}
};
