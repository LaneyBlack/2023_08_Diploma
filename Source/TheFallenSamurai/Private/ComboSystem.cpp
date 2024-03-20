#include "ComboSystem.h"

UComboSystem* UComboSystem::Instance = nullptr;

UComboSystem::UComboSystem()
{
	KillCount = 0;
	ComboLevel = 0;
}

void UComboSystem::IncreaseKillCount()
{
	if (!this)
	{
		return;
	}

	KillCount++;
	FString KillCountlString = FString::Printf(TEXT("Kill Count: %d"), KillCount);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, KillCountlString);
	UpdateComboLevel();

	KillIncreasedEvent.Broadcast();
	
}

void UComboSystem::UpdateComboLevel()
{
	if (KillCount != PreviousKillCount)
	{
		ComboLevel++;
		PreviousKillCount = KillCount;
		FString ComboLevelString = FString::Printf(TEXT("Combo Level: %d"), ComboLevel);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, ComboLevelString);

		/**
		if (ComboLevel > 1)
		{
			FString KillStreakString = FString::Printf(TEXT("%d kills!"), ComboLevel);
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, KillStreakString);
		}
		**/
	}
}


void UComboSystem::ResetCombo()
{
	ComboLevel = 0;

	FString ResetString = FString::Printf(TEXT("Combo reset!"));
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, ResetString);
}

UComboSystem* UComboSystem::GetInstance()
{
	if (Instance == nullptr)
	{
		Instance = NewObject<UComboSystem>();
		Instance->AddToRoot();
	}
	return Instance;
}

void UComboSystem::BeginDestroy()
{
	Super::BeginDestroy();
	Instance = nullptr;
}