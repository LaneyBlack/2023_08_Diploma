#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ComboSystem.h"
#include "ComboSystemInstance.generated.h"

UCLASS()
class THEFALLENSAMURAI_API UComboSystemInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	UComboSystem* ComboSystemInstance;

	virtual void Init() override;
};
