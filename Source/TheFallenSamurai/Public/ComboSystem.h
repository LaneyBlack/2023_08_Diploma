// ComboSystem.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ComboSystem.generated.h"

UCLASS()
class THEFALLENSAMURAI_API UComboSystem : public UObject
{
	GENERATED_BODY()

public:
	UComboSystem();

	UPROPERTY(BlueprintReadOnly)
	int32 KillCount;

	UPROPERTY(BlueprintReadOnly)
	int32 ComboLevel;

	void IncreaseKillCount();

	void ResetCombo();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ComboSystem")
	static UComboSystem* GetInstance();

protected:
	virtual void BeginDestroy() override; // Dodano funkcję BeginDestroy()

	private:
	static UComboSystem* Instance;
};