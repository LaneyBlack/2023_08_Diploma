// LevelLoaderBPLibrary.cpp
#include "LevelLoadingBPLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

void ULevelLoadingBPLibrary::LoadLevelWithLoadingScreen(ACharacter* Character, FName LevelName, TSubclassOf<UUserWidget> LoadingScreenClass)
{
    // Show the loading screen
    UUserWidget* LoadingScreenWidget = ShowLoadingScreen(Character, LoadingScreenClass);

    // Start asynchronously loading the new level's package
    FString LevelPath = FString::Printf(TEXT("/Game/Maps/%s"), *LevelName.ToString());

    // Use LoadPackageAsync to load the level package asynchronously
    LoadPackageAsync(
        FName(*LevelPath).ToString(),
        FLoadPackageAsyncDelegate::CreateStatic(&ULevelLoadingBPLibrary::OnLevelLoaded, Character, LevelName, LoadingScreenWidget),
        0
    );
}

UUserWidget* ULevelLoadingBPLibrary::ShowLoadingScreen(ACharacter* Character, TSubclassOf<UUserWidget> LoadingScreenClass)
{
    if (LoadingScreenClass && Character)
    {
        UWorld* World = Character->GetWorld();
        UUserWidget* LoadingScreenWidget = CreateWidget<UUserWidget>(World, LoadingScreenClass);

        if (LoadingScreenWidget)
        {
            // Add it to the viewport to display the loading screen
            LoadingScreenWidget->AddToViewport();
        }
        return LoadingScreenWidget;
    }
    return nullptr;
}

void ULevelLoadingBPLibrary::HideLoadingScreen(UUserWidget* LoadingScreenWidget)
{
    if (LoadingScreenWidget)
    {
        // Remove the loading screen from the viewport
        LoadingScreenWidget->RemoveFromParent();
    }
}

void ULevelLoadingBPLibrary::OnLevelLoaded(const FName& PackageName, UPackage* LoadedPackage, EAsyncLoadingResult::Type Result, ACharacter* Character, FName TargetLevelName, UUserWidget* LoadingScreenWidget)
{
    if (Result == EAsyncLoadingResult::Succeeded && Character)
    {
        // Load the level when the package has finished loading
        UGameplayStatics::OpenLevel(Character, TargetLevelName);
    }

    // Hide the loading screen after the level has loaded
    HideLoadingScreen(LoadingScreenWidget);
}
