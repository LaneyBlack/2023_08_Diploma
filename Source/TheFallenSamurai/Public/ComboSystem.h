#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TimerManager.h"
#include "ComboSystem.generated.h"

UCLASS()
class THEFALLENSAMURAI_API UComboSystem : public UObject
{
	GENERATED_BODY()

public:
	UComboSystem();

	UPROPERTY(BlueprintReadOnly)
	int32 killCount;

	UPROPERTY(BlueprintReadOnly)
	int32 ComboLevel;

	UPROPERTY(BlueprintReadOnly)
	int32 totalComboPoints;

	UFUNCTION(BlueprintCallable, Category = "ComboSystem")
	void IncreaseKillCount();

	void ResetCombo();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ComboSystem")
	static UComboSystem* GetInstance();

	DECLARE_EVENT(UComboSystem, FOnKillIncreased)
	FOnKillIncreased& OnKillIncreased() { return KillIncreasedEvent; }

protected:
	virtual void BeginDestroy() override;

	int32 PreviousKillCount = 0;

	int32 currentComboPoints;

private:
	static UComboSystem* instance;

	void UpdateComboLevel();

	FOnKillIncreased KillIncreasedEvent;
};