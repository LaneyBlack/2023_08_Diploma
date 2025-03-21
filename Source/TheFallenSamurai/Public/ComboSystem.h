#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "ComboTimerComponent.h"
#include "ComboSystem.generated.h"

UENUM(BlueprintType)
enum class EComboState : uint8
{
	None UMETA(DisplayName = "None"),
	WalljumpKill UMETA(DisplayName = "WalljumpKill")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewKillStreakMessage, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResetCombo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResetKillstreak);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboPointsChanged, int32, NewComboPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeStopCalled, FString, FailReason); 

UCLASS()
class THEFALLENSAMURAI_API UComboSystem : public UObject
{
	GENERATED_BODY()

public:
	UComboSystem();

	UPROPERTY(BlueprintReadOnly)
	int32 KillCount;

	UPROPERTY(BlueprintReadOnly)
	int32 KillStreakCount;

	UPROPERTY(BlueprintReadOnly)
	int32 ComboLevel;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalComboPoints;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentComboPoints;

	UPROPERTY(BlueprintReadOnly)
	FString KillStreakName;

	UPROPERTY(BlueprintReadWrite)
	int32 ComboCache;

	UPROPERTY(BlueprintReadWrite)
	int32 AbilityComboPoints;

	UPROPERTY(BlueprintReadOnly)
	int32 SuperAbilityCost = 20000;

	UPROPERTY(BlueprintReadOnly)
	int32 TimeStopCost = 13333;

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> KillStreakMessages;

	UFUNCTION(BlueprintCallable, Category = "ComboSystem")
	void IncreaseKillCount();

	UFUNCTION(BlueprintCallable, Category = "ComboSystem")
	void CompleteReset();

	void StartKillStreak();

	UFUNCTION(BlueprintCallable, Category = "ComboSystem")
	void ResetCombo();

	void ResetComboState();

	UFUNCTION(BlueprintCallable, Category = "ComboSystem")
	void EndKillStreak();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ComboSystem")
	static UComboSystem* GetInstance();

	DECLARE_EVENT(UComboSystem, FOnKillIncreased)
	FOnKillIncreased& OnKillIncreased() { return KillIncreasedEvent; }

	UPROPERTY(BlueprintAssignable)
	FOnNewKillStreakMessage OnNewKillStreakMessage;

	UPROPERTY(BlueprintAssignable)
	FOnTimeStopCalled OnTimeStopCalled;

	UPROPERTY(BlueprintAssignable)
	FOnResetKillstreak OnResetKillstreak;

	UPROPERTY(BlueprintAssignable)
	FOnResetCombo OnResetCombo;

	UPROPERTY(BlueprintAssignable)
	FOnComboStart OnComboStart;

	UFUNCTION(BlueprintCallable, Category = "ComboSystem")
	void SetOwnerCharacter(ATheFallenSamuraiCharacter* NewOwner)
	{
		OwnerCharacter = NewOwner;
	}

	EComboState GetComboState() const
	{
		return ComboState;
	}

	UFUNCTION(BlueprintCallable, Category = "ComboSystem")
	void SetComboState(EComboState NewComboState)
	{
		ComboState = NewComboState;
		InitializeComboStateTimer();
	}

	int32 GetTotalComboPoints() const;

	int32 GetCurrentComboPoints() const;

	FOnComboPointsChanged OnComboPointsChanged;

protected:
	virtual void BeginDestroy() override;

private:
	static UComboSystem* Instance;

	ATheFallenSamuraiCharacter* OwnerCharacter = nullptr;
	
	EComboState ComboState;

	void UpdateComboLevel();

	void HandleComboState();

	void InitializeComboStateTimer();

	FOnKillIncreased KillIncreasedEvent;

	int32 PreviousKillCount = 0;
};
