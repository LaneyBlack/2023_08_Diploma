#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "ComboSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewKillStreakMessage, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResetCombo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboStart);

UCLASS()
class THEFALLENSAMURAI_API UComboSystem : public UObject
{
	GENERATED_BODY()

public:
	UComboSystem();

	UPROPERTY(BlueprintReadOnly)
	int32 killCount;

	UPROPERTY(BlueprintReadOnly)
	int32 killStreakCount;

	UPROPERTY(BlueprintReadOnly)
	int32 ComboLevel;

	UPROPERTY(BlueprintReadOnly)
	int32 totalComboPoints;

	UPROPERTY(BlueprintReadOnly)
	int32 currentComboPoints;

	UPROPERTY(BlueprintReadOnly)
	FString killStreakName;

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> killStreakMessages;

	UFUNCTION(BlueprintCallable, Category = "ComboSystem")
	void IncreaseKillCount();

	void ResetCombo();

	void StartKillStreak();

	void EndKillStreak();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ComboSystem")
	static UComboSystem* GetInstance();

	DECLARE_EVENT(UComboSystem, FOnKillIncreased)
	FOnKillIncreased& OnKillIncreased() { return KillIncreasedEvent; }

	UPROPERTY(BlueprintAssignable)
	FOnNewKillStreakMessage OnNewKillStreakMessage;

	UPROPERTY(BlueprintAssignable)
	FOnResetCombo OnResetCombo;

	UPROPERTY(BlueprintAssignable)
	FOnComboStart OnComboStart;

protected:
	virtual void BeginDestroy() override;

	int32 PreviousKillCount = 0;

private:
	static UComboSystem* instance;

	void UpdateComboLevel();

	FOnKillIncreased KillIncreasedEvent;
};