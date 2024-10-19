// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LevelSummaryData.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct THEFALLENSAMURAI_API FLevelSummaryDataStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Summary Data")
	int32 ComboPoints;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Summary Data")
	int32 PlayerDeaths;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Summary Data")
	float ElapsedTime;
	
	FLevelSummaryDataStruct()
		: ComboPoints(0), PlayerDeaths(0), ElapsedTime(0.0f)
	{
	}
};

UCLASS(BlueprintType)
class THEFALLENSAMURAI_API ULevelSummaryData : public UObject
{
	GENERATED_BODY()

public:
	ULevelSummaryData();

	UFUNCTION(BlueprintCallable, Category = "Summary Data")
	int32 GatherAndReturnComboPoints();

	UFUNCTION(BlueprintCallable, Category = "Summary Data")
	int32 GatherAndReturnPlayerDeaths();

	UFUNCTION(BlueprintCallable, Category = "Summary Data")
	void GatherElapsedTime(float ElapsedTime);

private:
	FLevelSummaryDataStruct SummaryData;
};
