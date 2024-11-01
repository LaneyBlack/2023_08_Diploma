// Fill out your copyright notice in the Description page of Project Settings.


#include "TheFallenSamurai/LevelSummary/LevelSummaryData.h"

#include "ComboSystem.h"
#include "PlayerGameModeBase.h"

ULevelSummaryData::ULevelSummaryData()
{
	SummaryData = FLevelSummaryDataStruct();
}

void ULevelSummaryData::GatherSteamData(FString SteamName, FString SteamID)
{
	SummaryData.SteamID = SteamID;
	SummaryData.SteamUsername = SteamName;
}


int64 ULevelSummaryData::GatherAndReturnComboPoints()
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
}

void ULevelSummaryData::GatherLevelName(FString LevelName)
{
	SummaryData.LevelName = LevelName;
}

bool ULevelSummaryData::SaveSummaryDataToFile()
{

	if (SummaryData.LevelName.IsEmpty() || SummaryData.SteamID.IsEmpty())
	{
		return false;
	}
	
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	JsonObject->SetStringField("SteamID", SummaryData.SteamID);
	JsonObject->SetStringField("SteamUsername", SummaryData.SteamUsername);
	JsonObject->SetStringField("LevelName", SummaryData.LevelName);
	JsonObject->SetNumberField("ComboPoints", SummaryData.ComboPoints);
	JsonObject->SetNumberField("PlayerDeaths", SummaryData.PlayerDeaths);
	JsonObject->SetNumberField("ElapsedTime", SummaryData.ElapsedTime);
	
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
	{
		return false;
	}
	
	FString DirectoryPath = FPaths::ProjectDir() / TEXT("Saved") / SummaryData.SteamID;
	
	IFileManager& FileManager = IFileManager::Get();
	if (!FileManager.DirectoryExists(*DirectoryPath))
	{
		FileManager.MakeDirectory(*DirectoryPath);
	}
	
	FString FileName = FString::Printf(TEXT("%s_summary_data.json"), *SummaryData.LevelName);
	FString FilePath = DirectoryPath / FileName;
	
	return FFileHelper::SaveStringToFile(OutputString, *FilePath);
}

