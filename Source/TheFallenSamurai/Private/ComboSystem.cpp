#include "ComboSystem.h"

#include "TheFallenSamurai/TheFallenSamuraiCharacter.h"


UComboSystem* UComboSystem::Instance = nullptr;

UComboSystem::UComboSystem()
{
	KillCount = 0;
	KillStreakCount = 0;
	ComboLevel = 0;
	TotalComboPoints = 0;
	CurrentComboPoints = 0;
	ComboState = EComboState::None;
	KillStreakMessages.Empty();
}

void UComboSystem::IncreaseKillCount()
{
	if (!this)
	{
		return;
	}

	KillCount++;
	
	StartKillStreak();
	
	UpdateComboLevel();

	KillIncreasedEvent.Broadcast();
	
}

void UComboSystem::UpdateComboLevel()
{
	if (KillCount != PreviousKillCount)
	{
		ComboLevel++;
		
		float Multiplier = FMath::Pow(2.0f, static_cast<float>(ComboLevel));
		
		CurrentComboPoints += Multiplier * 100;

		PreviousKillCount = KillCount;

		OnComboStart.Broadcast();
	}
}

int32 UComboSystem::GetCurrentComboPoints() const
{
	return CurrentComboPoints;
}

void UComboSystem::HandleComboState()
{
	if (this->ComboState == EComboState::WalljumpKill)
	{
		KillStreakCount = (KillStreakCount > 0) ? KillStreakCount + 1 : 1;
		int32 Points = 1000;
		FString StreakName = "Walljump Kill!";
		CurrentComboPoints += Points;
		KillStreakName = StreakName + FString::Printf(TEXT(" +%d"), Points);
		KillStreakMessages.Add(KillStreakName);
		OnNewKillStreakMessage.Broadcast(KillStreakName);
	}
}

void UComboSystem::StartKillStreak()
{
	HandleComboState();

	if (this->ComboState == EComboState::None)
	{
		KillStreakCount = (KillStreakCount > 0) ? KillStreakCount + 1 : 1;
	}
	else
	{
		ResetComboState();
	}
	
	int32 Points = 0;
	FString StreakName;

	switch (KillStreakCount)
	{
	case 2:
		Points = 1000;
		StreakName = "Double Kill!";
		break;
	case 3:
		Points = 2000;
		StreakName = "Triple Kill!";
		break;
	case 4:
		Points = 3000;
		StreakName = "Quadra Kill!";
		break;
	default:
		if (KillStreakCount >= 5)
		{
			Points = 5000;
			StreakName = "Killing Machine!";
		}
		break;
	}

	if (!StreakName.IsEmpty())
	{
		CurrentComboPoints += Points;
		KillStreakName = StreakName + FString::Printf(TEXT(" +%d"), Points);
		KillStreakMessages.Add(KillStreakName);
		OnNewKillStreakMessage.Broadcast(KillStreakName);
	}
}


void UComboSystem::EndKillStreak()
{

	//Fuszera do kosztów ability
	if (AbilityComboPoints <= SuperAbilityCost)
	{
		AbilityComboPoints += CurrentComboPoints;

		if (AbilityComboPoints > SuperAbilityCost)
		{
			AbilityComboPoints = SuperAbilityCost;
		}
	}
	//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Cyan, FString::Printf(TEXT("Combo Points = %i"), AbilityComboPoints));
	KillStreakCount = 0;
	OnResetKillstreak.Broadcast();
	KillStreakMessages.Empty();
}


void UComboSystem::ResetCombo()
{
	TotalComboPoints += CurrentComboPoints;
	CurrentComboPoints = 0;
	ComboLevel = 0;
	OnResetCombo.Broadcast();
	ResetComboState();
}

void UComboSystem::ResetComboState()
{
	this->ComboState = EComboState::None;
}

void UComboSystem::InitializeComboStateTimer()
{
	if (OwnerCharacter != nullptr)
	{
		UComboTimerComponent* ComboTimerComponent = OwnerCharacter->FindComponentByClass<UComboTimerComponent>();
		if (ComboTimerComponent != nullptr)
		{
			ComboTimerComponent->StartComboStateTimer();
		}
	}
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
