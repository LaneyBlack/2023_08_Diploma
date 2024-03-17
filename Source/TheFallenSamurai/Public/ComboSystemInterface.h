#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ComboSystemInterface.generated.h"

UINTERFACE(MinimalAPI)
class UComboSystemInterface : public UInterface
{
	GENERATED_BODY()
};

class THEFALLENSAMURAI_API IComboSystemInterface
{
	GENERATED_BODY()

public:
	// Deklaracja funkcji, nie ma implementacji tutaj
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ComboSystem")
	class UComboSystem* GetComboSystemInstance();
};
