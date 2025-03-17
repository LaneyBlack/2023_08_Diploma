// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CustomBPLibrary.generated.h"

/**
 *	Blueprint Library for small methods that could be done easier in Cpp
 */
UCLASS()
class THEFALLENSAMURAI_API UCustomBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get App Version"), Category = "Game Config")
	static FString GetAppVersion();
	
};
