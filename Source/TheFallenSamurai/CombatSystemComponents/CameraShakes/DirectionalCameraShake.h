// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
//#include "DirectionalPerlinShakePattern.h"
#include "DirectionalMixedShakePattern.h"
#include "DirectionalCameraShake.generated.h"

/**
 * 
 */
UCLASS()
class THEFALLENSAMURAI_API UDirectionalCameraShake : public UCameraShakeBase
{
	GENERATED_BODY()

	//UPROPERTY(EditAnywhere, Category="Test")
	//FVector swing;

public:
	void SetSwingVector(const FVector& InSwingDirection)
	{
		//UDirectionalPerlinShakePattern* pattern = Cast<UDirectionalPerlinShakePattern>(GetRootShakePattern());
		UDirectionalMixedShakePattern* pattern = Cast<UDirectionalMixedShakePattern>(GetRootShakePattern());
		pattern->SetDirectionVectors(InSwingDirection);
	}
};
