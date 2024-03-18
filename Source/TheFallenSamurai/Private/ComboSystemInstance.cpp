#include "ComboSystemInstance.h"

void UComboSystemInstance::Init()
{
	Super::Init();
	
	ComboSystemInstance = UComboSystem::GetInstance();
}
