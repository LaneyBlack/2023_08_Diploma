#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ComboSystemInterface.h"
#include "ComboSystem.generated.h"

USTRUCT(BlueprintType)
struct FComboData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentComboLevel;

	UPROPERTY(BlueprintReadWrite)
	int32 MaxComboLevel;

	FComboData() : CurrentComboLevel(0), MaxComboLevel(5) {}
};

UCLASS(BlueprintType)
class THEFALLENSAMURAI_API UComboSystem : public UObject, public IComboSystemInterface
{
	GENERATED_BODY()

public:
	virtual UComboSystem* GetComboSystemInstance_Implementation() override;
	
	UFUNCTION(BlueprintCallable, Category = "ComboSystem")
	static UComboSystem* GetInstance();

	UFUNCTION(BlueprintCallable, Category = "ComboSystem")
	void EnemyKilled();
	
	UFUNCTION(BlueprintPure, Category = "ComboSystem")
	int32 GetCurrentComboLevel() const;

	UPROPERTY(BlueprintReadOnly, Category = "ComboSystem")
	int32 currentComboKills;

private:
	static UComboSystem* Instance;
	FComboData ComboData;

	UComboSystem();

public:
	~UComboSystem();
};
