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
	// initiate
}

UComboSystem::~UComboSystem()
{
	// clear
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
		
	}
	else
	{
		
	}
}

int32 UComboSystem::GetCurrentComboLevel() const
{
	return ComboData.CurrentComboLevel;
}
