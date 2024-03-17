// ComboSystem.cpp

#include "ComboSystem.h"

UComboSystem* UComboSystem::Instance = nullptr;

UComboSystem::UComboSystem()
{
	KillCount = 0;
	ComboLevel = 0;
}

void UComboSystem::IncreaseKillCount()
{
	KillCount++;
	// Adjust Combo Level based on KillCount, for example:
	// ComboLevel = KillCount / 5;
}

void UComboSystem::ResetCombo()
{
	KillCount = 0;
	ComboLevel = 0;
}

UComboSystem* UComboSystem::GetInstance()
{
	if (Instance == nullptr)
	{
		Instance = NewObject<UComboSystem>();
		Instance->AddToRoot(); // Dodano dodanie obiektu do drzewa referencji
	}
	return Instance;
}

void UComboSystem::BeginDestroy()
{
	Super::BeginDestroy();
	Instance = nullptr; // Dodano zwolnienie obiektu podczas zamykania gry
}