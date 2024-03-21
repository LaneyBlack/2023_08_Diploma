#include "ComboSystem.h"

UComboSystem* UComboSystem::instance = nullptr;

UComboSystem::UComboSystem()
{
	killCount = 0;
	ComboLevel = 0;
}

void UComboSystem::IncreaseKillCount()
{
	if (!this)
	{
		return;
	}

	killCount++;
	
	FString KillCountlString = FString::Printf(TEXT("Kill Count: %d"), killCount);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, KillCountlString);
	
	UpdateComboLevel();

	KillIncreasedEvent.Broadcast();
	
}

void UComboSystem::UpdateComboLevel()
{
	if (killCount != PreviousKillCount)
	{
		ComboLevel++;
		PreviousKillCount = killCount;
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
	if (instance == nullptr)
	{
		instance = NewObject<UComboSystem>();
		instance->AddToRoot();
	}
	return instance;
}

void UComboSystem::BeginDestroy()
{
	Super::BeginDestroy();
	instance = nullptr;
}