// Fill out your copyright notice in the Description page of Project Settings.


#include "TheFallenSamurai/LevelSummary/LevelSummaryData.h"

#include "ComboSystem.h"
#include "PlayerGameModeBase.h"

ULevelSummaryData::ULevelSummaryData()
{
	SummaryData = FLevelSummaryDataStruct();
}

int32 ULevelSummaryData::GatherAndReturnComboPoints()
{
	UComboSystem* ComboSystem = UComboSystem::GetInstance();

	if (ComboSystem)
	{
		SummaryData.ComboPoints = ComboSystem->GetTotalComboPoints();

		return SummaryData.ComboPoints;
	}
	return 0;
}

int32 ULevelSummaryData::GatherAndReturnPlayerDeaths()
{
	APlayerGameModeBase* GameMode = GetWorld()->GetAuthGameMode<APlayerGameModeBase>();

	if (GameMode)
	{
		SummaryData.PlayerDeaths = GameMode->PlayerDeaths;

		return SummaryData.PlayerDeaths;
	}
	return 0;
}

void ULevelSummaryData::GatherElapsedTime(float ElapsedTime)
{
	SummaryData.ElapsedTime = ElapsedTime;

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Elapsed Time: %f"), SummaryData.ElapsedTime));
}