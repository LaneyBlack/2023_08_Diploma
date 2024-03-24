#include "ComboSystem.h"

UComboSystem* UComboSystem::instance = nullptr;

UComboSystem::UComboSystem()
{
	killCount = 0;
	ComboLevel = 0;
	totalComboPoints = 0;
	currentComboPoints = 0;
	killStreakMessages.Empty();
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
	
	StartKillStreak();
	
	UpdateComboLevel();

	KillIncreasedEvent.Broadcast();
	
}

void UComboSystem::UpdateComboLevel()
{
	if (killCount != PreviousKillCount)
	{
		ComboLevel++;

		currentComboPoints += FMath::Pow(10.0f, static_cast<float>(ComboLevel));

		PreviousKillCount = killCount;

		FString ComboLevelString = FString::Printf(TEXT("Combo Level: %d"), ComboLevel);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, ComboLevelString);

		OnComboStart.Broadcast();
	}
}

void UComboSystem::StartKillStreak()
{
	if (killStreakCount > 0)
	{
		killStreakCount++;
	}
	else
	{
		killStreakCount = 1;
	}
	
	switch (killStreakCount)
	{
	case 2:
		currentComboPoints += 10;
		killStreakName = "Double Kill!";
		killStreakMessages.Add(killStreakName);
		OnNewKillStreakMessage.Broadcast(killStreakName);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, killStreakName);
		break;
	case 3:
		currentComboPoints += 20;
		killStreakName = "Triple Kill!";
		killStreakMessages.Add(killStreakName);
		OnNewKillStreakMessage.Broadcast(killStreakName);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, killStreakName);
		break;
	case 4:
		currentComboPoints += 30;
		killStreakName = "Quadra Kill!";
		killStreakMessages.Add(killStreakName);
		OnNewKillStreakMessage.Broadcast(killStreakName);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, killStreakName);
		break;
	default:
		if (killStreakCount >= 5)
		{
			currentComboPoints += 50;
			killStreakName = "Killing Machine!";
			killStreakMessages.Add(killStreakName);
			OnNewKillStreakMessage.Broadcast(killStreakName);
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, killStreakName);
		}
		break;
	}
}

void UComboSystem::EndKillStreak()
{
	killStreakCount = 0;
	killStreakMessages.Empty();
}


void UComboSystem::ResetCombo()
{
	totalComboPoints += currentComboPoints;
	currentComboPoints = 0;
	ComboLevel = 0;
	OnResetCombo.Broadcast();

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