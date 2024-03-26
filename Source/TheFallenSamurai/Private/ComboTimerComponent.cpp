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
    StartKillStreakTimer();
}

void UComboTimerComponent::StartComboTimer()
{
    GetWorld()->GetTimerManager().SetTimer(ComboTimerHandle, this, &UComboTimerComponent::OnComboTimerEnd, 5.0f, false);
}

void UComboTimerComponent::OnComboTimerEnd()
{
    UComboSystem::GetInstance()->ResetCombo();
}

void UComboTimerComponent::StartKillStreakTimer()
{
    if (GetWorld()->GetTimerManager().IsTimerActive(KillStreakTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(KillStreakTimerHandle);
    }
    
    GetWorld()->GetTimerManager().SetTimer(KillStreakTimerHandle, this, &UComboTimerComponent::OnKillStreakTimerEnd, 2.0f, false);
}

void UComboTimerComponent::StartComboStateTimer()
{
    if (UComboSystem::GetInstance()->GetComboState() != EComboState::None)
    {
        GetWorld()->GetTimerManager().SetTimer(ComboStateTimerHandle, this, &UComboTimerComponent::OnComboStateTimerEnd, 2.0f, false);
    }
}

void UComboTimerComponent::OnKillStreakTimerEnd()
{
    UComboSystem::GetInstance()->EndKillStreak();
}

void UComboTimerComponent::OnComboStateTimerEnd()
{
    UComboSystem::GetInstance()-> ResetComboState();
}

float UComboTimerComponent::GetRemainingComboTime() const
{
    return GetWorld()->GetTimerManager().GetTimerRemaining(ComboTimerHandle);
}

float UComboTimerComponent::GetTotalComboTime() const
{
    return GetWorld()->GetTimerManager().GetTimerRate(ComboTimerHandle);
}
