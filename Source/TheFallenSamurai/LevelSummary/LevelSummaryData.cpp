#include "LevelSummaryData.h"
#include "Kismet/GameplayStatics.h"

ULevelSummaryData* ULevelSummaryData::Instance = nullptr;

ULevelSummaryData::ULevelSummaryData()
{
    SaveGameInstance = NewObject<ULevelSummarySaveGame>();
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
    if (SaveGameInstance)
    {
        SaveGameInstance->SteamID = SteamID;
        SaveGameInstance->SteamUsername = SteamName;
    }
}

FString ULevelSummaryData::GetSlotName() const
{
    return SaveGameInstance && !SaveGameInstance->SteamID.IsEmpty()
        ? SaveGameInstance->SteamID + "_LevelSummary"
        : "Local_LevelSummary";
}

float ULevelSummaryData::CalculateTotalScore(int64 ComboPoints, int32 PlayerDeaths, float ElapsedTime)
{
    float TotalScore = FMath::RoundToInt64(ComboPoints - (PlayerDeaths * 1000) - (ElapsedTime * 10.0f));

    if (TotalScore < 0)
    {
        TotalScore = 0;
    }

    return TotalScore;
}

bool ULevelSummaryData::AddLevelData(const FLevelData& LevelData)
{
    if (!SaveGameInstance)
    {
        return false;
    }

    for (FLevelData& ExistingData : SaveGameInstance->LevelSummaries)
    {
        if (ExistingData.LevelName == LevelData.LevelName)
        {
            if (IsNewLevelScoreHigher(LevelData))
            {
                ExistingData = LevelData;
                return true;
            }
            
            return false;
        }
    }

    SaveGameInstance->LevelSummaries.Add(LevelData);
    return true;
}

bool ULevelSummaryData::SaveSummaryDataToSlot()
{
    if (SaveGameInstance)
    {
        FString SlotName = GetSlotName();
        bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0);
        

        return bSuccess;
    }
    
    return false;
}

bool ULevelSummaryData::LoadSaveData()
{
    FString SlotName = GetSlotName();
    ULevelSummarySaveGame* LoadedData = Cast<ULevelSummarySaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0));

    if (LoadedData)
    {
        SaveGameInstance = LoadedData;
        return true;
    }
    
    return false;
}

bool ULevelSummaryData::IsNewLevelScoreHigher(const FLevelData& NewData)
{
    for (const FLevelData& ExistingData : SaveGameInstance->LevelSummaries)
    {
        if (ExistingData.LevelName == NewData.LevelName)
        {
            return NewData.TotalScore > ExistingData.TotalScore;
        }
    }

    return true;
}

bool ULevelSummaryData::UnlockLevel(const FString& LevelName)
{
    FRegexPattern LevelPattern(TEXT("^Level\\d+$"));
    FRegexMatcher LevelMatcher(LevelPattern, LevelName);

    if (!LevelMatcher.FindNext())
    {
        return false;
    }

    if (!SaveGameInstance->UnlockedLevels.Contains(LevelName))
    {
        SaveGameInstance->UnlockedLevels.Add(LevelName);
        return true;
    }
    return false;
}

TArray<FString> ULevelSummaryData::GetUnlockedLevels() const
{
    return SaveGameInstance ? SaveGameInstance->UnlockedLevels : TArray<FString>();
}

FLevelData ULevelSummaryData::GetLevelStatsByName(const FString& LevelName) const
{
    if (!SaveGameInstance)
    {
        return FLevelData();
    }

    for (const FLevelData& LevelData : SaveGameInstance->LevelSummaries)
    {
        if (LevelData.LevelName == LevelName)
        {
            return LevelData;
        }
    }
    
    return FLevelData();
}

FString ULevelSummaryData::NameIDMerger(const FString& StringA, const FString& StringB)
{
    return StringA + TEXT("@") + StringB;
}

FString ULevelSummaryData::NameSeparator(const FString& InputString)
{
    int32 Separator;
    if (InputString.FindChar('@', Separator))
    {
        return InputString.Left(Separator);
    }
    return InputString;
}


//DEBUG
void ULevelSummaryData::PrintSaveDataWithSteamID()
{
    if (SaveGameInstance)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("SteamID: %s"), *SaveGameInstance->SteamID));

        for (const FLevelData& LevelData : SaveGameInstance->LevelSummaries)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Level: %s"), *LevelData.LevelName));
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("ComboPoints: %lld"), LevelData.ComboPoints));
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("PlayerDeaths: %d"), LevelData.PlayerDeaths));
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("ElapsedTime: %f"), LevelData.ElapsedTime));
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("TotalScore: %f"), LevelData.TotalScore));
        }
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("SaveGameInstance is null!"));
    }
}

void ULevelSummaryData::PrintUnlockedLevels()
{
    if (SaveGameInstance)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("SteamID: %s"), *SaveGameInstance->SteamID));

        if (SaveGameInstance->UnlockedLevels.Num() > 0)
        {
            for (const FString& LevelName : SaveGameInstance->UnlockedLevels)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Unlocked Level: %s"), *LevelName));
            }
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("No levels unlocked!"));
        }
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("SaveGameInstance is null!"));
    }
}
