// Fill out your copyright notice in the Description page of Project Settings.

#include "RewindComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/MovementComponent.h"
#include "PlayerGameModeBase.h"


#define PRINT(mess, mtime)  GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::Green, TEXT(mess));
#define PRINTC(mess, color)  GEngine->AddOnScreenDebugMessage(-1, 3, color, TEXT(mess));
#define PRINT_F(prompt, mess, mtime) GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::Green, FString::Printf(TEXT(prompt), mess));
#define PRINTC_F(prompt, mess, mtime, color) GEngine->AddOnScreenDebugMessage(-1, mtime, color, FString::Printf(TEXT(prompt), mess));
#define PRINT_B(prompt, mess) GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, FString::Printf(TEXT(prompt), mess ? TEXT("TRUE") : TEXT("FALSE")));


URewindComponent::URewindComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void URewindComponent::BeginPlay()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(URewindComponent::BeginPlay);

	Super::BeginPlay();

	GameMode = Cast<APlayerGameModeBase>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		SetComponentTickEnabled(false);
		return;
	}

	OwnerRootComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (bSnapshotMovementVelocityAndMode)
	{
		OwnerMovementComponent = Character ? Cast<UCharacterMovementComponent>(Character->GetMovementComponent()) : nullptr;
	}

	if (bPauseAnimationDuringTimeScrubbing && Character)
	{
		OwnerSkeletalMesh = Character->GetMesh();
		ensureMsgf(OwnerSkeletalMesh, TEXT("OwnerSkeletalMesh is null for %s"), *GetOwner()->GetName());
	}

	GameMode->OnGlobalRewindStarted.AddUniqueDynamic(this, &URewindComponent::OnGlobalRewindStarted);
	GameMode->OnGlobalRewindCompleted.AddUniqueDynamic(this, &URewindComponent::OnGlobalRewindCompleted);
	GameMode->OnGlobalTimeScrubStarted.AddUniqueDynamic(this, &URewindComponent::OnGlobalTimeScrubStarted);
	GameMode->OnGlobalTimeScrubCompleted.AddUniqueDynamic(this, &URewindComponent::OnGlobalTimeScrubCompleted);

	InitializeRingBuffers(GameMode->MaxRewindSeconds);
}


void URewindComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(URewindComponent::TickComponent);

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsRewindingForDuration)
	{
		RemainingRewindDuration -= DeltaTime;
		if (RemainingRewindDuration <= 0.0f)
		{
			bIsRewindingForDuration = false;
			OnGlobalRewindCompleted();
		}
		else
		{
			PlaySnapshots(DeltaTime, true);
		}
	}
	else if (bIsRewinding)
	{
		PlaySnapshots(DeltaTime, true);
	}
	else if (bIsTimeScrubbing)
	{
		PauseTime(DeltaTime, bLastTimeManipulationWasRewind);
	}
	else
	{
		RecordSnapshot(DeltaTime);
	}

	//PRINT_B("is time scrubbing %s", bIsTimeScrubbing);
}

void URewindComponent::SetIsRewindingEnabled(bool bEnabled)
{
	bIsRewindingEnabled = bEnabled;
	if (!bIsRewindingEnabled)
	{
		if (bIsRewinding) { OnGlobalRewindCompleted(); }

		if (bIsTimeScrubbing) { OnGlobalTimeScrubCompleted(); }
	}
	else
	{
		check(GameMode);
		
		if (!bIsRewinding && GameMode->IsGlobalRewinding()) { OnGlobalRewindStarted(); }

		if (!bIsTimeScrubbing && GameMode->IsGlobalTimeScrubbing()) { OnGlobalTimeScrubStarted(); }
	}
}

void URewindComponent::OnGlobalRewindStarted()
{
	bool bAlreadyManipulatingTime = IsTimeBeingManipulated();
	if (TryStartTimeManipulation(bIsRewinding, !bIsTimeScrubbing))
	{
		OnRewindStarted.Broadcast();
		if (!bAlreadyManipulatingTime) { OnTimeManipulationStarted.Broadcast(); }
	}
}

void URewindComponent::OnGlobalRewindCompleted()
{
	if (TryStopTimeManipulation(bIsRewinding, !bIsTimeScrubbing, false /*bResetMovementVelocity*/))
	{
		bLastTimeManipulationWasRewind = true;

		OnRewindCompleted.Broadcast();
		if (!IsTimeBeingManipulated()) { OnTimeManipulationCompleted.Broadcast(); }
	}
}

void URewindComponent::InitializeRingBuffers(float MaxRewindSeconds)
{
	MaxSnapshots = FMath::CeilToInt32(MaxRewindSeconds / SnapshotFrequencySeconds);
	
	constexpr uint32 OneMB = 1024 * 1024;
	constexpr uint32 ThreeMB = 3 * OneMB;
	if (!bSnapshotMovementVelocityAndMode)
	{
		uint32 SnapshotBytes = sizeof(FTransformAndVelocitySnapshot);
		uint32 TotalSnapshotBytes = MaxSnapshots * SnapshotBytes;
		ensureMsgf(
			TotalSnapshotBytes < OneMB,
			TEXT("Actor %s has rewind component that requested %d bytes of snapshots. Check snapshot frequency!"),
			*GetOwner()->GetName(),
			TotalSnapshotBytes);

		MaxSnapshots = FMath::Min(MaxSnapshots, static_cast<uint32>(OneMB / SnapshotBytes));
	}
	else
	{
		uint32 SnapshotBytes = sizeof(FTransformAndVelocitySnapshot) + sizeof(FMovementVelocityAndModeSnapshot);
		uint32 TotalSnapshotBytes = MaxSnapshots * SnapshotBytes;
		ensureMsgf(
			TotalSnapshotBytes < ThreeMB,
			TEXT("Actor %s has rewind component that requested %d bytes of snapshots. Check snapshot frequency!"),
			*GetOwner()->GetName(),
			TotalSnapshotBytes);

		MaxSnapshots = FMath::Min(MaxSnapshots, static_cast<uint32>(ThreeMB / SnapshotBytes));
	}
	
	TransformAndVelocitySnapshots.Reserve(MaxSnapshots);
	
	if (bSnapshotMovementVelocityAndMode && OwnerMovementComponent)
	{
		MovementVelocityAndModeSnapshots.Reserve(MaxSnapshots);
	}
}

void URewindComponent::RecordSnapshot(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(URewindComponent::RecordSnapshot);

	TimeSinceSnapshotsChanged += DeltaTime;
	
	if (TimeSinceSnapshotsChanged < SnapshotFrequencySeconds && TransformAndVelocitySnapshots.Num() != 0) { return; }
	
	if (TransformAndVelocitySnapshots.Num() == MaxSnapshots) { TransformAndVelocitySnapshots.PopFront(); }
	
	FTransform Transform = GetOwner()->GetActorTransform();
	FVector LinearVelocity = OwnerRootComponent ? OwnerRootComponent->GetPhysicsLinearVelocity() : FVector::Zero();
	FVector AngularVelocityInRadians = OwnerRootComponent ? OwnerRootComponent->GetPhysicsAngularVelocityInRadians() : FVector::Zero();
	LatestSnapshotIndex =
		TransformAndVelocitySnapshots.Emplace(TimeSinceSnapshotsChanged, Transform, LinearVelocity, AngularVelocityInRadians);

	if (bSnapshotMovementVelocityAndMode && OwnerMovementComponent)
	{
		if (MovementVelocityAndModeSnapshots.Num() == MaxSnapshots) { MovementVelocityAndModeSnapshots.PopFront(); }
		
		FVector MovementVelocity = OwnerMovementComponent->Velocity;
		TEnumAsByte<EMovementMode> MovementMode = OwnerMovementComponent->MovementMode;
		int32 LatestMovementSnapshotIndex =
			MovementVelocityAndModeSnapshots.Emplace(TimeSinceSnapshotsChanged, MovementVelocity, MovementMode);
		check(LatestSnapshotIndex == LatestMovementSnapshotIndex);
	}

	TimeSinceSnapshotsChanged = 0.0f;
}

void URewindComponent::EraseFutureSnapshots()
{
	while (LatestSnapshotIndex < TransformAndVelocitySnapshots.Num() - 1)
	{
		TransformAndVelocitySnapshots.Pop();
	}

	if (bSnapshotMovementVelocityAndMode)
	{
		while (LatestSnapshotIndex < MovementVelocityAndModeSnapshots.Num() - 1)
		{
			MovementVelocityAndModeSnapshots.Pop();
		}
	}
}

void URewindComponent::PlaySnapshots(float DeltaTime, bool bRewinding)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(URewindComponent::PlaySnapshots);

	UnpauseAnimation();

	if (HandleInsufficientSnapshots()) { return; }
	
	DeltaTime *= GameMode->GetGlobalRewindSpeed();
	TimeSinceSnapshotsChanged += DeltaTime;

	bool bReachedEndOfTrack = false;
	float LatestSnapshotTime = TransformAndVelocitySnapshots[LatestSnapshotIndex].TimeSinceLastSnapshot;
	if (bRewinding)
	{
		while (LatestSnapshotIndex > 0 && TimeSinceSnapshotsChanged > LatestSnapshotTime)
		{
			TimeSinceSnapshotsChanged -= LatestSnapshotTime;
			LatestSnapshotTime = TransformAndVelocitySnapshots[LatestSnapshotIndex].TimeSinceLastSnapshot;
			--LatestSnapshotIndex;
		}
		
		if (LatestSnapshotIndex == TransformAndVelocitySnapshots.Num() - 1)
		{
			ApplySnapshot(TransformAndVelocitySnapshots[LatestSnapshotIndex], false /*bApplyPhysics*/);
			if (bSnapshotMovementVelocityAndMode)
			{
				ApplySnapshot(MovementVelocityAndModeSnapshots[LatestSnapshotIndex], true /*bApplyTimeDilationToVelocity*/);
			}
			return;
		}

		bReachedEndOfTrack = LatestSnapshotIndex == 0;
	}
	else
	{
		while (LatestSnapshotIndex < TransformAndVelocitySnapshots.Num() - 1 && TimeSinceSnapshotsChanged > LatestSnapshotTime)
		{
			TimeSinceSnapshotsChanged -= LatestSnapshotTime;
			LatestSnapshotTime = TransformAndVelocitySnapshots[LatestSnapshotIndex].TimeSinceLastSnapshot;
			++LatestSnapshotIndex;
		}

		bReachedEndOfTrack = LatestSnapshotIndex == TransformAndVelocitySnapshots.Num() - 1;
	}
	
	if (bReachedEndOfTrack)
	{
		TimeSinceSnapshotsChanged = FMath::Min(TimeSinceSnapshotsChanged, LatestSnapshotTime);
		if (bAnimationsPausedAtStartOfTimeManipulation) { PauseAnimation(); }
	}

	InterpolateAndApplySnapshots(bRewinding);
}

void URewindComponent::PauseTime(float DeltaTime, bool bRewinding)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(URewindComponent::PauseTime);

	if (HandleInsufficientSnapshots()) { return; }

	if (bRewinding)
	{
		if (LatestSnapshotIndex == TransformAndVelocitySnapshots.Num() - 1)
		{
			ApplySnapshot(TransformAndVelocitySnapshots[LatestSnapshotIndex], false /*bApplyPhysics*/);
			if (bSnapshotMovementVelocityAndMode)
			{
				ApplySnapshot(MovementVelocityAndModeSnapshots[LatestSnapshotIndex], true /*bApplyTimeDilationToVelocity*/);
			}
			if (OwnerSkeletalMesh)
			{
				PauseAnimation();
			}
			return;
		}
	}

	float LatestSnapshotTime = TransformAndVelocitySnapshots[LatestSnapshotIndex].TimeSinceLastSnapshot;
	if (TimeSinceSnapshotsChanged < LatestSnapshotTime)
	{
		DeltaTime *= GameMode->GetGlobalRewindSpeed();
		TimeSinceSnapshotsChanged = FMath::Min(TimeSinceSnapshotsChanged + DeltaTime, LatestSnapshotTime);
	}

	InterpolateAndApplySnapshots(bRewinding);

	if (FMath::IsNearlyEqual(TimeSinceSnapshotsChanged, LatestSnapshotTime) && OwnerSkeletalMesh) 
	{
		PauseAnimation();
	}
}


bool URewindComponent::TryStartTimeManipulation(bool& bStateToSet, bool bResetTimeSinceSnapshotsChanged)
{
	if (!bIsRewindingEnabled || bStateToSet) { return false; }

	bStateToSet = true;

	if (bResetTimeSinceSnapshotsChanged) { TimeSinceSnapshotsChanged = 0.0f; }
	
	PausePhysics();
	
	bAnimationsPausedAtStartOfTimeManipulation = bPausedAnimation;

	return true;
}

bool URewindComponent::TryStopTimeManipulation(bool& bStateToSet, bool bResetTimeSinceSnapshotsChanged, bool bResetMovementVelocity)
{
	if (!bStateToSet) { return false; }
	
	bStateToSet = false;
	
	if (!bIsTimeScrubbing)
	{
		if (bResetTimeSinceSnapshotsChanged) { TimeSinceSnapshotsChanged = 0.0f; }
		
		UnpausePhysics();
		
		UnpauseAnimation();
		
		if (LatestSnapshotIndex >= 0)
		{
			ApplySnapshot(TransformAndVelocitySnapshots[LatestSnapshotIndex], true /*bApplyPhysics*/);
			if (bSnapshotMovementVelocityAndMode)
			{
				ApplySnapshot(MovementVelocityAndModeSnapshots[LatestSnapshotIndex], false /*bApplyTimeDilationToVelocity*/);
				
				if (bResetMovementVelocity && OwnerMovementComponent) { OwnerMovementComponent->Velocity = FVector::ZeroVector; }
			}
		}
		
		EraseFutureSnapshots();
	}

	return true;
}

void URewindComponent::PausePhysics()
{
	if (OwnerRootComponent && OwnerRootComponent->BodyInstance.bSimulatePhysics)
	{
		bPausedPhysics = true;
		OwnerRootComponent->SetSimulatePhysics(false);
	}
}

void URewindComponent::UnpausePhysics()
{
	if (!bPausedPhysics) { return; }

	check(OwnerRootComponent);
	bPausedPhysics = false;
	OwnerRootComponent->SetSimulatePhysics(true);
	OwnerRootComponent->RecreatePhysicsState();
}

void URewindComponent::PauseAnimation()
{
	if (!bPauseAnimationDuringTimeScrubbing || !OwnerSkeletalMesh) { return; }

	bPausedAnimation = true;
	OwnerSkeletalMesh->bPauseAnims = true;
}

void URewindComponent::UnpauseAnimation()
{
	if (!bPausedAnimation || !OwnerSkeletalMesh) { return; }

	bPausedAnimation = false;
	OwnerSkeletalMesh->bPauseAnims = false;
}


bool URewindComponent::HandleInsufficientSnapshots()
{
	check(!bSnapshotMovementVelocityAndMode || TransformAndVelocitySnapshots.Num() == MovementVelocityAndModeSnapshots.Num());
	if (LatestSnapshotIndex < 0 || TransformAndVelocitySnapshots.Num() == 0) { return true; }
	
	if (TransformAndVelocitySnapshots.Num() == 1)
	{
		ApplySnapshot(TransformAndVelocitySnapshots[0], false /*bApplyPhysics*/);
		if (bSnapshotMovementVelocityAndMode) { ApplySnapshot(MovementVelocityAndModeSnapshots[0], true /*bApplyTimeDilationToVelocity*/); }
		return true;
	}
	check(LatestSnapshotIndex >= 0 && LatestSnapshotIndex < TransformAndVelocitySnapshots.Num());
	return false;
}

void URewindComponent::InterpolateAndApplySnapshots(bool bRewinding)
{
	constexpr int MinSnapshotsForInterpolation = 2;
	check(TransformAndVelocitySnapshots.Num() >= MinSnapshotsForInterpolation);
	check(bRewinding && LatestSnapshotIndex < TransformAndVelocitySnapshots.Num() - 1 || !bRewinding && LatestSnapshotIndex > 0);
	int PreviousIndex = bRewinding ? LatestSnapshotIndex + 1 : LatestSnapshotIndex - 1;
	
	{
		const FTransformAndVelocitySnapshot& PreviousSnapshot = TransformAndVelocitySnapshots[PreviousIndex];
		const FTransformAndVelocitySnapshot& NextSnapshot = TransformAndVelocitySnapshots[LatestSnapshotIndex];
		ApplySnapshot(
			BlendSnapshots(PreviousSnapshot, NextSnapshot, TimeSinceSnapshotsChanged / NextSnapshot.TimeSinceLastSnapshot),
			false /*bApplyPhysics*/);
	}
	
	if (bSnapshotMovementVelocityAndMode)
	{
		const FMovementVelocityAndModeSnapshot& PreviousSnapshot = MovementVelocityAndModeSnapshots[PreviousIndex];
		const FMovementVelocityAndModeSnapshot& NextSnapshot = MovementVelocityAndModeSnapshots[LatestSnapshotIndex];
		ApplySnapshot(
			BlendSnapshots(PreviousSnapshot, NextSnapshot, TimeSinceSnapshotsChanged / NextSnapshot.TimeSinceLastSnapshot),
			true /*bApplyTimeDilationToVelocity*/);
	}
}

FTransformAndVelocitySnapshot URewindComponent::BlendSnapshots(
	const FTransformAndVelocitySnapshot& A,
	const FTransformAndVelocitySnapshot& B,
	float Alpha)
{
	Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
	FTransformAndVelocitySnapshot BlendedSnapshot;
	BlendedSnapshot.Transform.Blend(A.Transform, B.Transform, Alpha);
	BlendedSnapshot.LinearVelocity = FMath::Lerp(A.LinearVelocity, B.LinearVelocity, Alpha);
	BlendedSnapshot.AngularVelocityInRadians = FMath::Lerp(A.AngularVelocityInRadians, B.AngularVelocityInRadians, Alpha);
	return BlendedSnapshot;
}

FMovementVelocityAndModeSnapshot URewindComponent::BlendSnapshots(
	const FMovementVelocityAndModeSnapshot& A,
	const FMovementVelocityAndModeSnapshot& B,
	float Alpha)
{
	Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
	FMovementVelocityAndModeSnapshot BlendedSnapshot;
	BlendedSnapshot.MovementVelocity = FMath::Lerp(A.MovementVelocity, B.MovementVelocity, Alpha);
	BlendedSnapshot.MovementMode = Alpha < 0.5f ? A.MovementMode : B.MovementMode;
	return BlendedSnapshot;
}

void URewindComponent::ApplySnapshot(const FTransformAndVelocitySnapshot& Snapshot, bool bApplyPhysics)
{
	GetOwner()->SetActorTransform(Snapshot.Transform);
	if (OwnerRootComponent && bApplyPhysics)
	{
		OwnerRootComponent->SetPhysicsLinearVelocity(Snapshot.LinearVelocity);
		OwnerRootComponent->SetPhysicsAngularVelocityInRadians(Snapshot.AngularVelocityInRadians);
	}
}

void URewindComponent::ApplySnapshot(const FMovementVelocityAndModeSnapshot& Snapshot, bool bApplyTimeDilationToVelocity)
{
	if (OwnerMovementComponent)
	{
		OwnerMovementComponent->Velocity =
			bApplyTimeDilationToVelocity ? Snapshot.MovementVelocity * GameMode->GetGlobalRewindSpeed() : Snapshot.MovementVelocity;
		OwnerMovementComponent->SetMovementMode(Snapshot.MovementMode);
	}
}

void URewindComponent::OnGlobalTimeScrubStarted()
{
	bool bAlreadyManipulatingTime = IsTimeBeingManipulated();
	if (TryStartTimeManipulation(bIsTimeScrubbing, false))
	{
		OnTimeScrubStarted.Broadcast();
		if (!bAlreadyManipulatingTime) { OnTimeManipulationStarted.Broadcast(); }
	}
}

void URewindComponent::OnGlobalTimeScrubCompleted()
{
	if (TryStopTimeManipulation(bIsTimeScrubbing, false /*bResetTimeSinceSnapshotsChanged*/, true /*bResetMovementVelocity*/))
	{
		if (!IsTimeBeingManipulated()) { OnTimeManipulationCompleted.Broadcast(); }
	}
}

void URewindComponent::RewindForDuration(float Duration)
{
	if (!GameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode is not set in URewindComponent::RewindForDuration"));
		return;
	}
	
	GameMode->SetRewindSpeedFastest();
	GameMode->StartGlobalRewind();

	GetWorld()->GetTimerManager().SetTimer(RewindTimerHandle, this, &URewindComponent::StopRewindForDuration, Duration, false);
}

void URewindComponent::StopRewindForDuration()
{
	if (!GameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode is not set in URewindComponent::StopRewind"));
		return;
	}

	GameMode->SetRewindSpeedNormal();
	GameMode->StopGlobalRewind();
}

void URewindComponent::TimeScrubForDuration(float Duration)
{
	if (!GameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode is not set in URewindComponent::TimeScrubForDuration"));
		return;
	}

	bIsTimeScrubbingForDuration = true;

	GameMode->ToggleTimeScrub();

	GetWorld()->GetTimerManager().SetTimer(RewindTimerHandle, this, &URewindComponent::StopTimeScrubForDuration, Duration, false);
}

void URewindComponent::StopTimeScrubForDuration()
{
	if (!GameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode is not set in URewindComponent::StopTimeScrub"));
		return;
	}

	GameMode->ToggleTimeScrub();

	bIsTimeScrubbingForDuration = false;
}