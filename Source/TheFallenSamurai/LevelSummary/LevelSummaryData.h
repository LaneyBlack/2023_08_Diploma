#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LevelSummarySaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "LevelSummaryData.generated.h"

UCLASS(BlueprintType)
class THEFALLENSAMURAI_API ULevelSummaryData : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Summary Data")
	static ULevelSummaryData* GetDataInstance();

	UFUNCTION(BlueprintCallable, Category = "Summary Data")
	void GatherSteamData(FString SteamName, FString SteamID);

	UFUNCTION(BlueprintCallable, Category = "Summary Data")
	float CalculateTotalScore(int64 ComboPoints, int32 PlayerDeaths, float ElapsedTime);

	UFUNCTION(BlueprintCallable, Category = "Summary Data")
	bool AddLevelData(const FLevelData& LevelData);

	UFUNCTION(BlueprintCallable, Category = "Summary Data")
	bool SaveSummaryDataToSlot();

	UFUNCTION(BlueprintCallable, Category = "Summary Data")
	bool LoadSaveData();

	UFUNCTION(BlueprintCallable, Category = "Level Unlock")
	bool UnlockLevel(const FString& LevelName);

	UFUNCTION(BlueprintCallable, Category = "Level Unlock")
	TArray<FString> GetUnlockedLevels() const;

	UFUNCTION(BlueprintCallable, Category = "String Utilities")
	FString NameIDMerger(const FString& StringA, const FString& StringB);

	UFUNCTION(BlueprintCallable, Category = "String Utilities")
	FString NameSeparator(const FString& InputString);

	UFUNCTION(BlueprintCallable, Category = "DEBUG")
	void PrintSaveDataWithSteamID();

	UFUNCTION(BlueprintCallable, Category = "DEBUG")
	void PrintUnlockedLevels();

protected:
	ULevelSummaryData();

private:
	static ULevelSummaryData* Instance;

	UPROPERTY()
	ULevelSummarySaveGame* SaveGameInstance;

	FString GetSlotName() const;
	bool IsNewLevelScoreHigher(const FLevelData& NewData);
};
