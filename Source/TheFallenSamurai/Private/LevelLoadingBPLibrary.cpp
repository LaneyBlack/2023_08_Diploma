// LevelLoaderBPLibrary.cpp
#include "LevelLoadingBPLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

void ULevelLoadingBPLibrary::LoadLevelWithLoadingScreen(UObject* WorldContextObject, FName LevelName, TSubclassOf<UUserWidget> LoadingScreenClass)
{
    UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;

    if (!World) return;

    UUserWidget* LoadingScreenWidget = ShowLoadingScreen(World, LoadingScreenClass);

    FString LevelPath = FString::Printf(TEXT("/Game/Maps/%s"), *LevelName.ToString());

    LoadPackageAsync(
        FName(*LevelPath).ToString(),
        FLoadPackageAsyncDelegate::CreateStatic(&ULevelLoadingBPLibrary::OnLevelLoaded, World, LevelName, LoadingScreenWidget),
        0
    );
}

UUserWidget* ULevelLoadingBPLibrary::ShowLoadingScreen(UWorld* World, TSubclassOf<UUserWidget> LoadingScreenClass)
{
    if (LoadingScreenClass && World)
    {
        UUserWidget* LoadingScreenWidget = CreateWidget<UUserWidget>(World, LoadingScreenClass);
        if (LoadingScreenWidget)
        {
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

void ULevelLoadingBPLibrary::OnLevelLoaded(const FName& PackageName, UPackage* LoadedPackage, EAsyncLoadingResult::Type Result, UWorld* World, FName TargetLevelName, UUserWidget* LoadingScreenWidget)
{
    if (Result == EAsyncLoadingResult::Succeeded && World)
    {
        // Load the level when the package has finished loading
        UGameplayStatics::OpenLevel(World, TargetLevelName);
    }

    // Hide the loading screen after the level has loaded
    HideLoadingScreen(LoadingScreenWidget);
}
