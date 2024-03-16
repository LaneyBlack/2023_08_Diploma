// AComboSystem.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AComboSystem.generated.h"

UCLASS()
class THEFALLENSAMURAI_API AAComboSystem : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAComboSystem();

	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	int32 ComboCount;

	UFUNCTION(BlueprintCallable, Category = "Combo")
	void IncreaseCombo();

	UFUNCTION(BlueprintCallable, Category = "Combo")
	void ResetPlayerCombo();

	UFUNCTION(BlueprintCallable, Category = "Combo")
	void CheckIfMultiKill();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	float LastKillTime;
	const float DoubleKillTimeLimit = 5.0f; // Adjust this value as needed
	const float MaxComboTime = 10.0f; // Adjust this value as needed
	int32 KillCount;
};