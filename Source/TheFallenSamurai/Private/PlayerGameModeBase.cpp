// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerGameModeBase.h"

#include "UObject/ConstructorHelpers.h"

APlayerGameModeBase::APlayerGameModeBase()
{
	
}

void APlayerGameModeBase::StartGlobalRewind()
{
	TRACE_BOOKMARK(TEXT("ARewindGameMode::StartGlobalRewind"));

	bIsGlobalRewinding = true;
	OnGlobalRewindStarted.Broadcast();
}

void APlayerGameModeBase::ToggleTimeScrub()
{
	bIsGlobalTimeScrubbing = !bIsGlobalTimeScrubbing;
	if (bIsGlobalTimeScrubbing)
	{
		TRACE_BOOKMARK(TEXT("ARewindGameMode::ToggleTimeScrub - Start Time Scrubbing"));
		OnGlobalTimeScrubStarted.Broadcast();
	}
	else
	{
		TRACE_BOOKMARK(TEXT("ARewindGameMode::ToggleTimeScrub - Stop Time Scrubbing"));
		OnGlobalTimeScrubCompleted.Broadcast();
	}
}

void APlayerGameModeBase::StopGlobalRewind()
{
	TRACE_BOOKMARK(TEXT("ARewindGameMode::StopGlobalRewind"));

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