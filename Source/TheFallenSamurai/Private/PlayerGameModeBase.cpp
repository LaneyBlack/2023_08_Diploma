// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerGameModeBase.h"

#include "UObject/ConstructorHelpers.h"

APlayerGameModeBase::APlayerGameModeBase()
{
	
}

void APlayerGameModeBase::StartGlobalRewind()
{
	bIsGlobalRewinding = true;
	OnGlobalRewindStarted.Broadcast();
}

void APlayerGameModeBase::ToggleTimeScrub()
{
	bIsGlobalTimeScrubbing = !bIsGlobalTimeScrubbing;
	if (bIsGlobalTimeScrubbing)
	{
		OnGlobalTimeScrubStarted.Broadcast();
	}
	else
	{
		OnGlobalTimeScrubCompleted.Broadcast();
	}
}

void APlayerGameModeBase::StopGlobalRewind()
{
	bIsGlobalRewinding = false;
	OnGlobalRewindCompleted.Broadcast();
}

void APlayerGameModeBase::SetRewindSpeedSlowest()
{
	GlobalRewindSpeed = SlowestRewindSpeed;
}

void APlayerGameModeBase::SetRewindSpeedSlower()
{
	GlobalRewindSpeed = SlowerRewindSpeed;
}

void APlayerGameModeBase::SetRewindSpeedNormal()
{
	GlobalRewindSpeed = NormalRewindSpeed;
}

void APlayerGameModeBase::SetRewindSpeedFaster()
{
	GlobalRewindSpeed = FasterRewindSpeed;
}

void APlayerGameModeBase::SetRewindSpeedFastest()
{
	GlobalRewindSpeed = FastestRewindSpeed;
}