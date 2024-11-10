// Fill out your copyright notice in the Description page of Project Settings.


#include "TheFallenSamurai/LevelSummary/LevelSummaryData.h"

#include "ComboSystem.h"
#include "PlayerGameModeBase.h"

ULevelSummaryData* ULevelSummaryData::Instance = nullptr;

ULevelSummaryData::ULevelSummaryData()
{
	SummaryData = FLevelSummaryDataStruct();
}

ULevelSummaryData* ULevelSummaryData::GetDataInstance()
{
	if (!Instance)
	{
		Instance = NewObject<ULevelSummaryData>();
		Instance->AddToRoot();
	}
	return Instance;
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

int32 ULevelSummaryData::GatherAndReturnPlayerDeaths(APlayerGameModeBase* GameMode)
{
	if (GameMode)
	{
		SummaryData.PlayerDeaths = GameMode->PlayerDeaths;
		return SummaryData.PlayerDeaths;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("GatherAndReturnPlayerDeaths: GameMode is null"));
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

FString ULevelSummaryData::SaveSummaryDataToFile()
{
	if (SummaryData.LevelName == "None")
	{
		return "Level not found";
	}
	
	if (SummaryData.SteamID == "Unknown")
	{
		return "Steam not connected";
	}
	
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	JsonObject->SetStringField("SteamID", SummaryData.SteamID);
	JsonObject->SetStringField("SteamUsername", SummaryData.SteamUsername);
	JsonObject->SetStringField("LevelName", SummaryData.LevelName);
	JsonObject->SetNumberField("ComboPoints", SummaryData.ComboPoints);
	JsonObject->SetNumberField("PlayerDeaths", SummaryData.PlayerDeaths);
	JsonObject->SetNumberField("ElapsedTime", SummaryData.ElapsedTime);
	JsonObject->SetNumberField("TotalScore", SummaryData.TotalScore);
	
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
	{
		return "Failed to serialize save data";
	}
	
	FString DirectoryPath = FPaths::ProjectDir() / TEXT("Saved") / SummaryData.SteamID;
	IFileManager& FileManager = IFileManager::Get();
	if (!FileManager.DirectoryExists(*DirectoryPath))
	{
		FileManager.MakeDirectory(*DirectoryPath);
	}
	
	FString FileName = FString::Printf(TEXT("%s_summary_data.json"), *SummaryData.LevelName);
	FString FilePath = DirectoryPath / FileName;
	if (!FFileHelper::SaveStringToFile(OutputString, *FilePath))
	{
		return "Failed to save data to file";
	}
	
	return "Data saved successfully";
}


int32 ULevelSummaryData::CalculateTotalScore()
{
	SummaryData.TotalScore = SummaryData.ComboPoints - (SummaryData.PlayerDeaths * 1000) - (SummaryData.ElapsedTime * 10.0f);

	if(SummaryData.TotalScore < 0)
	{
		SummaryData.TotalScore = 0;
	}
	return FMath::RoundToInt(SummaryData.TotalScore);
}

bool ULevelSummaryData::IsNewTotalScoreHigherThanFile(const FString& SteamID, const FString& LevelName) const
{
	const FString DirectoryPath = FPaths::ProjectDir() / TEXT("Saved") / SteamID;
	const FString FileName = FString::Printf(TEXT("%s_summary_data.json"), *LevelName);
	const FString FilePath = DirectoryPath / FileName;
	
	if (!FPaths::FileExists(FilePath))
	{
		return true;
	}
	
	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
	{
		return true;
	}
	
	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContent);
	
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return true;
	}
	
	float PreviousTotalScore = 0.0f;
	if (!JsonObject->TryGetNumberField("TotalScore", PreviousTotalScore))
	{
		return true;
	}
	
	return SummaryData.TotalScore > PreviousTotalScore;
}

FString ULevelSummaryData::GetFilePath() const
{
	FString DirectoryPath = FPaths::ProjectDir() / TEXT("Saved") / SummaryData.SteamID;
	return DirectoryPath / TEXT("UnlockedLevels.json");
}

bool ULevelSummaryData::EnsureFileExists(const FString& SteamID)
{
	FString DirectoryPath = FPaths::ProjectDir() / TEXT("Saved") / SteamID;
	FString FilePath = DirectoryPath / TEXT("UnlockedLevels.json");
	
	if (FPaths::FileExists(FilePath))
	{
		return true;
	}
	
	IFileManager& FileManager = IFileManager::Get();
	if (!FileManager.DirectoryExists(*DirectoryPath))
	{
		FileManager.MakeDirectory(*DirectoryPath);
	}
	
	TSharedPtr<FJsonObject> DefaultJsonObject = MakeShareable(new FJsonObject);
	DefaultJsonObject->SetBoolField("Level1", true);
	DefaultJsonObject->SetBoolField("Level2", false);
	DefaultJsonObject->SetBoolField("Level3", false);

	FString DefaultContent;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&DefaultContent);
	if (!FJsonSerializer::Serialize(DefaultJsonObject.ToSharedRef(), Writer))
	{
		return false;
	}
	
	return FFileHelper::SaveStringToFile(DefaultContent, *FilePath);
}

bool ULevelSummaryData::UnlockLevel(const FString& LevelName)
{
	if (SummaryData.SteamID == "Unknown")
	{
		return false;
	}

	if (!LevelName.StartsWith(TEXT("Level")) || LevelName.Len() <= 5)
	{
		return false;
	}

	FString NumericPart = LevelName.Mid(5);
	int32 LevelNumber = FCString::Atoi(*NumericPart);

	if (LevelNumber <= 0)
	{
		return false;
	}

	FString FilePath = GetFilePath();
	
	if (!EnsureFileExists(SummaryData.SteamID))
	{
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	FString FileContent;
	if (FFileHelper::LoadFileToString(FileContent, *FilePath))
	{
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContent);
		FJsonSerializer::Deserialize(Reader, JsonObject);
	}

	JsonObject->SetBoolField(LevelName, true);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
	{
		return false;
	}

	return FFileHelper::SaveStringToFile(OutputString, *FilePath);
}

TArray<FString> ULevelSummaryData::GetUnlockedLevels(const FString& SteamID)
{
	TArray<FString> UnlockedLevels;
	FString DirectoryPath = FPaths::ProjectDir() / TEXT("Saved") / SteamID;
	FString FilePath = DirectoryPath / TEXT("UnlockedLevels.json");
	
	if (!EnsureFileExists(SteamID))
	{
		return UnlockedLevels;
	}

	FString FileContent;
	if (FFileHelper::LoadFileToString(FileContent, *FilePath))
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContent);

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			for (auto& Entry : JsonObject->Values)
			{
				bool bIsUnlocked = false;
				if (Entry.Value->TryGetBool(bIsUnlocked) && bIsUnlocked)
				{
					UnlockedLevels.Add(Entry.Key);
				}
			}
		}
	}

	return UnlockedLevels;
}