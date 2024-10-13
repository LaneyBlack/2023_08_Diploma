// LevelLoader.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelLoader.generated.h"

UCLASS()
class THEFALLENSAMURAI_API ALevelLoader : public AActor
{
    GENERATED_BODY()

public:
    ALevelLoader();

    // Function to initiate the loading process with a loading screen
    UFUNCTION(BlueprintCallable, Category = "Level Loading")
    void LoadLevelWithLoadingScreen(FName LevelName);

private:
    void OnLevelLoaded(const FName& PackageName, UPackage* LoadedPackage, EAsyncLoadingResult::Type Result);
    
    // Shows the loading screen widget
    void ShowLoadingScreen();

    // Hides the loading screen widget
    void HideLoadingScreen();

    // The name of the level we're going to load
    FName TargetLevelName;

    // Pointer to hold a reference to the loading screen widget
    UPROPERTY()
    UUserWidget* LoadingScreenWidget;

    // The class type of the loading screen widget
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> LoadingScreenClass;

    // Handle to manage the async level loading
    FDelegateHandle LevelLoadingHandle;
};
