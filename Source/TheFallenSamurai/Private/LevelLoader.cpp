// MyLevelLoader.cpp
#include "LevelLoader.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"

ALevelLoader::ALevelLoader()
{
	// Constructor logic, if any
}

void ALevelLoader::LoadLevelWithLoadingScreen(FName LevelName)
{
	// Set the target level name to load
	TargetLevelName = LevelName;

	// Show the loading screen
	ShowLoadingScreen();

	// Set a short delay before loading the next level (to ensure loading screen is visible)
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ALevelLoader::OnLevelLoaded, 0.5f, false);
}

void ALevelLoader::ShowLoadingScreen()
{
	// Check if we have a valid LoadingScreenClass to create a widget from
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

void ALevelLoader::OnLevelLoaded()
{
	// Load the new level using OpenLevel
	if (!TargetLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, TargetLevelName, true);
	}

	// Once the level is loaded, hide the loading screen
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
