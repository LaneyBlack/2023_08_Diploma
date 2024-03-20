#include "ComboTimerComponent.h"

#include "ComboSystem.h"

UComboTimerComponent::UComboTimerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UComboTimerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	UComboSystem::GetInstance()->OnKillIncreased().AddUObject(this, &UComboTimerComponent::OnKillIncreased);
}

void UComboTimerComponent::OnKillIncreased()
{
	StartComboTimer();
}

void UComboTimerComponent::StartComboTimer()
{
	GetWorld()->GetTimerManager().SetTimer(ComboTimerHandle, this, &UComboTimerComponent::OnComboTimerEnd, 5.0f, false);
}

void UComboTimerComponent::OnComboTimerEnd()
{
	UComboSystem::GetInstance()->ResetCombo();
}


