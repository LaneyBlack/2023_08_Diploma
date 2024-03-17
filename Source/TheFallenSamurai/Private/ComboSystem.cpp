#include "ComboSystem.h"

UComboSystem* UComboSystem::Instance = nullptr;

UComboSystem* UComboSystem::GetInstance()
{
	if (!Instance)
	{
		Instance = NewObject<UComboSystem>(GetTransientPackage());
	}
	return Instance;
}


UComboSystem::UComboSystem()
{
	// Inicjalizacja ComboSystemu
}

UComboSystem::~UComboSystem()
{
	// Czyszczenie ComboSystemu
}

UComboSystem* UComboSystem::GetComboSystemInstance_Implementation()
{
	return GetInstance();
}

void UComboSystem::EnemyKilled()
{
	if (!Instance)
	{
		UE_LOG(LogTemp, Warning, TEXT("ComboSystem instance not found"));
		return;
	}

	currentComboKills++;
	ComboData.CurrentComboLevel++;
	UE_LOG(LogTemp, Warning, TEXT("KILL!"));
	if (ComboData.CurrentComboLevel > ComboData.MaxComboLevel)
	{
		// Osiągnięto maksymalny poziom comba, wykonaj odpowiednie działania
		// Tutaj możesz wywołać efekty dźwiękowe, wizualne itp.
	}
	else
	{
		// Combo nie osiągnął jeszcze maksymalnego poziomu
	}
}

int32 UComboSystem::GetCurrentComboLevel() const
{
	return ComboData.CurrentComboLevel;
}
