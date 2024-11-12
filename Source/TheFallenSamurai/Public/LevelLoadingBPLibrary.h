#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LevelLoadingBPLibrary.generated.h"

/**
 *	A blueprint library class to async load the Level Assets
 */
UCLASS()
class THEFALLENSAMURAI_API ULevelLoadingBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Initiates the loading process with a loading screen
	UFUNCTION(BlueprintCallable, Category = "Level Loading")
	static void LoadLevelWithLoadingScreen(ACharacter* Character, FName LevelName, TSubclassOf<UUserWidget> LoadingScreenClass);

private:
	// Shows the loading screen widget
	static UUserWidget* ShowLoadingScreen(ACharacter* Character, TSubclassOf<UUserWidget> LoadingScreenClass);

	// Hides the loading screen widget
	static void HideLoadingScreen(UUserWidget* LoadingScreenWidget);

	// Callback for when the level has finished loading
	static void OnLevelLoaded(const FName& PackageName, UPackage* LoadedPackage, EAsyncLoadingResult::Type Result, ACharacter* Character, FName TargetLevelName, UUserWidget* LoadingScreenWidget);
};
