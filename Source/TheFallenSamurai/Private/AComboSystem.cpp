// AComboSystem.cpp

#include "AComboSystem.h"

// Sets default values
AAComboSystem::AAComboSystem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ComboCount = 0;
}

void AAComboSystem::IncreaseCombo()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastKillTime <= DoubleKillTimeLimit)
	{
		KillCount++;
		CheckIfMultiKill();
	}
	else
	{
		KillCount = 1;
	}

	LastKillTime = CurrentTime;
	ComboCount++;

	UE_LOG(LogTemp, Warning, TEXT("Aktualna wartość ComboCount: %d"), ComboCount);
}

void AAComboSystem::ResetPlayerCombo()
{
	ComboCount = 0;
}

void AAComboSystem::CheckIfMultiKill()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastKillTime <= DoubleKillTimeLimit)
	{
		if (KillCount == 2)
		{
			UE_LOG(LogTemp, Warning, TEXT("Double kill!"));
		}
		else if (KillCount == 3)
		{
			UE_LOG(LogTemp, Warning, TEXT("Triple kill!"));
		}
		else if (KillCount >= 4)
		{
			UE_LOG(LogTemp, Warning, TEXT("BESTMODE!"));
		}
	}
}

// Called when the game starts or when spawned
void AAComboSystem::BeginPlay()
{
	Super::BeginPlay();
	ResetPlayerCombo();
}

// Called every frame
void AAComboSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Reset Combo afret some time
	if(ComboCount >= 1)
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastKillTime >= MaxComboTime)
		{
		ResetPlayerCombo();
		}
	}
}