#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LevelSummarySaveGame.generated.h"

USTRUCT(BlueprintType)
struct FLevelData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SaveGame")
	FString LevelName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SaveGame")
	int64 ComboPoints;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SaveGame")
	int32 PlayerDeaths;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SaveGame")
	float ElapsedTime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SaveGame")
	float TotalScore;

	FLevelData()
		: LevelName("None"), ComboPoints(0), PlayerDeaths(0), ElapsedTime(0.0f), TotalScore(0.0f) {}
};

UCLASS()
class THEFALLENSAMURAI_API ULevelSummarySaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SaveGame")
	FString SteamID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SaveGame")
	FString SteamUsername;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SaveGame")
	TArray<FLevelData> LevelSummaries;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SaveGame")
	TArray<FString> UnlockedLevels;
};
