#include "ComboSystemInstance.h"
#include "TheFallenSamurai/UserSettings/MyGameUserSettings.h"

void UComboSystemInstance::Init()
{
	Super::Init();
	
	ComboSystemInstance = UComboSystem::GetInstance();
}
