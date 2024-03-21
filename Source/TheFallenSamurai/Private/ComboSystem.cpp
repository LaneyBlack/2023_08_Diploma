#include "ComboSystem.h"

UComboSystem* UComboSystem::instance = nullptr;

UComboSystem::UComboSystem()
{
	killCount = 0;
	ComboLevel = 0;
	totalComboPoints = 0;
	currentComboPoints = 0;
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

		currentComboPoints = FMath::Pow(10.0f, static_cast<float>(ComboLevel));

		PreviousKillCount = killCount;

		FString ComboLevelString = FString::Printf(TEXT("Combo Level: %d"), ComboLevel);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, ComboLevelString);
	}
}


void UComboSystem::ResetCombo()
{
	totalComboPoints += currentComboPoints;
	currentComboPoints = 0;
	ComboLevel = 0;

	FString ResetString = FString::Printf(TEXT("Combo reset!"));
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, ResetString);
	FString ComboPoints = FString::Printf(TEXT("Combo points: %d"), totalComboPoints);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, ComboPoints);
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