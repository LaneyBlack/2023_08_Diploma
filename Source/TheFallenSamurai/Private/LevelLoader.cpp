// LevelLoader.cpp
#include "LevelLoader.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Blueprint/UserWidget.h"

ALevelLoader::ALevelLoader()
{
	// Constructor logic if needed
}

void ALevelLoader::ShowLoadingScreen()
{
	if (LoadingScreenClass)
	{
		LoadingScreenWidget = CreateWidget<UUserWidget>(GetWorld(), LoadingScreenClass);

		if (LoadingScreenWidget)
		{
			// Add it to the viewport to display the loading screen
			LoadingScreenWidget->AddToViewport();
		}
	}
}

void ALevelLoader::LoadLevelWithLoadingScreen(FName LevelName)
{
	TargetLevelName = LevelName;

	// Show the loading screen
	ShowLoadingScreen();

	// Start asynchronously loading the new level's package
	FString LevelPath = FString::Printf(TEXT("/Game/Maps/%s"), *TargetLevelName.ToString());

	// Use LoadPackageAsync to load the level package asynchronously
	LoadPackageAsync(
		FName(*LevelPath).ToString(),
		FLoadPackageAsyncDelegate::CreateUObject(this, &ALevelLoader::OnLevelLoaded), // Delegate binding
		0
	);
}

void ALevelLoader::OnLevelLoaded(const FName& PackageName, UPackage* LoadedPackage, EAsyncLoadingResult::Type Result)
{
	if (Result == EAsyncLoadingResult::Succeeded)
	{
		// Load the level when the package has finished loading
		UGameplayStatics::OpenLevel(this, TargetLevelName);
	}

	// Hide the loading screen after the level has loaded
	HideLoadingScreen();
}



void ALevelLoader::HideLoadingScreen()
{
	if (LoadingScreenWidget)
	{
		// Remove the loading screen from the viewport
		LoadingScreenWidget->RemoveFromParent();
	}
}
