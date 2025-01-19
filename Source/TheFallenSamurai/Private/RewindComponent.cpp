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
#include "TheFallenSamurai/TheFallenSamuraiCharacter.h"


#define PRINT(mess, mtime)  GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::Green, TEXT(mess));
#define PRINTC(mess, color)  GEngine->AddOnScreenDebugMessage(-1, 3, color, TEXT(mess));
#define PRINT_F(prompt, mess, mtime) GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::Green, FString::Printf(TEXT(prompt), mess));
#define PRINTC_F(prompt, mess, mtime, color) GEngine->AddOnScreenDebugMessage(-1, mtime, color, FString::Printf(TEXT(prompt), mess));
//#define PRINT_B(prompt, mess) GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, FString::Printf(TEXT(prompt), mess ? TEXT("TRUE") : TEXT("FALSE")));

URewindComponent::URewindComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void URewindComponent::BeginPlay()
{
	Super::BeginPlay();

	GameMode = Cast<APlayerGameModeBase>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		SetComponentTickEnabled(false);
		return;
	}

	OwnerRootComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());

	if (bSnapshotMovementVelocityAndMode)
	{
		ACharacter* Character = Cast<ACharacter>(GetOwner());
		OwnerMovementComponent = Character ? Cast<UCharacterMovementComponent>(Character->GetMovementComponent()) : nullptr;
	}

	if (bPauseAnimationDuringTimeScrubbing && GetOwner()->IsA<ACharacter>())
	{
		OwnerSkeletalMesh = Cast<ACharacter>(GetOwner())->GetMesh();
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
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
	if (bIsRewindingForDuration)
	{
		
		if (LatestSnapshotIndex == 1)
		{
			bIsRewindingForDuration = false;
			StopRewindForDuration();
			return;
		}
		
		RemainingRewindDuration -= DeltaTime;
		
		if (RemainingRewindDuration <= 0.0f)
		{
			ATheFallenSamuraiCharacter* Player = Cast<ATheFallenSamuraiCharacter>(GetOwner());

			if (Player)
			{
				bool bIsLocationSafe = PerformSafetyTrace(Player->GetActorLocation());

				if (bIsLocationSafe)
				{
					bIsRewindingForDuration = false;
					StopRewindForDuration();
				}
				else
				{
					PlaySnapshots(DeltaTime, true);
				}
			}
			else
			{
				PlaySnapshots(DeltaTime, true);
			}
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
	constexpr uint32 OneMB = 1024 * 1024;
	
	MaxSnapshots = FMath::CeilToInt32(MaxRewindSeconds / SnapshotFrequencySeconds);
	
	uint32 SnapshotBytes = bSnapshotMovementVelocityAndMode ?
		sizeof(FTransformAndVelocitySnapshot) + sizeof(FMovementVelocityAndModeSnapshot) :
		sizeof(FTransformAndVelocitySnapshot);
	
	uint32 MaxTotalSnapshotBytes = bSnapshotMovementVelocityAndMode ? 3 * OneMB : OneMB;
	
	uint32 TotalSnapshotBytes = MaxSnapshots * SnapshotBytes;
	ensureMsgf(
		TotalSnapshotBytes < MaxTotalSnapshotBytes,
		TEXT("Actor %s has a rewind component that requested %d bytes of snapshots. Check snapshot frequency!"),
		*GetOwner()->GetName(),
		TotalSnapshotBytes);
	
	MaxSnapshots = FMath::Min(MaxSnapshots, MaxTotalSnapshotBytes / SnapshotBytes);
	
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

	if (TimeSinceSnapshotsChanged < SnapshotFrequencySeconds && TransformAndVelocitySnapshots.Num() != 0)
	{
		return;
	}

	if (TransformAndVelocitySnapshots.Num() == MaxSnapshots)
	{
		TransformAndVelocitySnapshots.PopFront();
	}

	FTransform Transform = GetOwner()->GetActorTransform();
	FVector LinearVelocity = OwnerRootComponent ? OwnerRootComponent->GetPhysicsLinearVelocity() : FVector::Zero();
	FVector AngularVelocityInRadians = OwnerRootComponent ? OwnerRootComponent->GetPhysicsAngularVelocityInRadians() : FVector::Zero();

	LatestSnapshotIndex = TransformAndVelocitySnapshots.Emplace(TimeSinceSnapshotsChanged, Transform, LinearVelocity, AngularVelocityInRadians);

	if (bSnapshotMovementVelocityAndMode && OwnerMovementComponent)
	{
		if (MovementVelocityAndModeSnapshots.Num() == MaxSnapshots)
		{
			MovementVelocityAndModeSnapshots.PopFront();
		}

		FVector MovementVelocity = OwnerMovementComponent->Velocity;
		TEnumAsByte<EMovementMode> MovementMode = OwnerMovementComponent->MovementMode;

		int32 LatestMovementSnapshotIndex = MovementVelocityAndModeSnapshots.Emplace(TimeSinceSnapshotsChanged, MovementVelocity, MovementMode);
		check(LatestSnapshotIndex == LatestMovementSnapshotIndex);
	}

	TimeSinceSnapshotsChanged = 0.0f;
}

bool URewindComponent::PerformSafetyTrace(const FVector& Location) const
{
	FVector Start = Location;
	FVector End = Location - FVector(0, 0, 150.0f);
	
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());
	
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);
	
	return bHit;
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
    UnpauseAnimation();
	
    if (HandleInsufficientSnapshots()) { return; }
	
    DeltaTime *= GameMode->GetGlobalRewindSpeed();
    TimeSinceSnapshotsChanged += DeltaTime;
	
    float LatestSnapshotTime = TransformAndVelocitySnapshots[LatestSnapshotIndex].TimeSinceLastSnapshot;
	
    if (bRewinding)
    {
        while (LatestSnapshotIndex > 0 && TimeSinceSnapshotsChanged > LatestSnapshotTime)
        {
            TimeSinceSnapshotsChanged -= LatestSnapshotTime;
            LatestSnapshotTime = TransformAndVelocitySnapshots[--LatestSnapshotIndex].TimeSinceLastSnapshot;
        }
    	
        if (LatestSnapshotIndex == TransformAndVelocitySnapshots.Num() - 1)
        {
            ApplySnapshot(TransformAndVelocitySnapshots[LatestSnapshotIndex], false);
            if (bSnapshotMovementVelocityAndMode)
            {
                ApplySnapshot(MovementVelocityAndModeSnapshots[LatestSnapshotIndex], true);
            }
            return;
        }
    }
    else
    {
        while (LatestSnapshotIndex < TransformAndVelocitySnapshots.Num() - 1 && TimeSinceSnapshotsChanged > LatestSnapshotTime)
        {
            TimeSinceSnapshotsChanged -= LatestSnapshotTime;
            LatestSnapshotTime = TransformAndVelocitySnapshots[++LatestSnapshotIndex].TimeSinceLastSnapshot;
        }
    }
	
    bool bReachedEndOfTrack = (bRewinding && LatestSnapshotIndex == 0) || (!bRewinding && LatestSnapshotIndex == TransformAndVelocitySnapshots.Num() - 1);
	
    if (bReachedEndOfTrack)
    {
        TimeSinceSnapshotsChanged = FMath::Min(TimeSinceSnapshotsChanged, LatestSnapshotTime);
        if (bAnimationsPausedAtStartOfTimeManipulation) { PauseAnimation(); }
    }
	
    InterpolateAndApplySnapshots(bRewinding);
}

void URewindComponent::PauseTime(float DeltaTime, bool bRewinding)
{
	if (HandleInsufficientSnapshots()) { return; }
	
	if (bRewinding && LatestSnapshotIndex == TransformAndVelocitySnapshots.Num() - 1)
	{
		ApplySnapshot(TransformAndVelocitySnapshots[LatestSnapshotIndex], false);
		if (bSnapshotMovementVelocityAndMode)
		{
			ApplySnapshot(MovementVelocityAndModeSnapshots[LatestSnapshotIndex], true);
		}
		if (OwnerSkeletalMesh)
		{
			PauseAnimation();
		}
		return;
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
			ApplySnapshot(TransformAndVelocitySnapshots[LatestSnapshotIndex], true);
			if (bSnapshotMovementVelocityAndMode)
			{
				ApplySnapshot(MovementVelocityAndModeSnapshots[LatestSnapshotIndex], false);
				
				if (bResetMovementVelocity && OwnerMovementComponent) { OwnerMovementComponent->Velocity = FVector::ZeroVector; }
			}
		}
		
		EraseFutureSnapshots();
	}

	return true;
}

void URewindComponent::PausePhysics()
{
	if (OwnerRootComponent && OwnerRootComponent->IsSimulatingPhysics())
	{
		bPausedPhysics = true;
		OwnerRootComponent->SetSimulatePhysics(false);
	}
}

void URewindComponent::UnpausePhysics()
{
	if (!bPausedPhysics || !OwnerRootComponent) { return; }

	bPausedPhysics = false;
	OwnerRootComponent->SetSimulatePhysics(true);
	OwnerRootComponent->RecreatePhysicsState();
}

void URewindComponent::PauseAnimation()
{
	if (bPauseAnimationDuringTimeScrubbing && OwnerSkeletalMesh)
	{
		bPausedAnimation = true;
		OwnerSkeletalMesh->bPauseAnims = true;
	}
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

	if (LatestSnapshotIndex < 0 || TransformAndVelocitySnapshots.Num() == 0)
	{
		return true;
	}

	if (TransformAndVelocitySnapshots.Num() == 1)
	{
		ApplySnapshot(TransformAndVelocitySnapshots[0], false);
		if (bSnapshotMovementVelocityAndMode)
		{
			ApplySnapshot(MovementVelocityAndModeSnapshots[0], true);
		}
		return true;
	}

	check(LatestSnapshotIndex >= 0 && LatestSnapshotIndex < TransformAndVelocitySnapshots.Num());
	return false;
}

void URewindComponent::InterpolateAndApplySnapshots(bool bRewinding)
{
	constexpr int MinSnapshotsForInterpolation = 2;
	check(TransformAndVelocitySnapshots.Num() >= MinSnapshotsForInterpolation);
	check((bRewinding && LatestSnapshotIndex < TransformAndVelocitySnapshots.Num() - 1) ||
		  (!bRewinding && LatestSnapshotIndex > 0));

	int PreviousIndex = bRewinding ? LatestSnapshotIndex + 1 : LatestSnapshotIndex - 1;
	float InterpolationFactor = TimeSinceSnapshotsChanged / TransformAndVelocitySnapshots[LatestSnapshotIndex].TimeSinceLastSnapshot;

	const FTransformAndVelocitySnapshot& PreviousSnapshot = TransformAndVelocitySnapshots[PreviousIndex];
	const FTransformAndVelocitySnapshot& NextSnapshot = TransformAndVelocitySnapshots[LatestSnapshotIndex];
	ApplySnapshot(BlendSnapshots(PreviousSnapshot, NextSnapshot, InterpolationFactor), false);

	if (bSnapshotMovementVelocityAndMode)
	{
		const FMovementVelocityAndModeSnapshot& PreviousMovementSnapshot = MovementVelocityAndModeSnapshots[PreviousIndex];
		const FMovementVelocityAndModeSnapshot& NextMovementSnapshot = MovementVelocityAndModeSnapshots[LatestSnapshotIndex];
		ApplySnapshot(BlendSnapshots(PreviousMovementSnapshot, NextMovementSnapshot, InterpolationFactor), true);
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
	RemainingRewindDuration = Duration;
	bIsRewindingForDuration = true;
	OnGlobalRewindStarted();
}

void URewindComponent::StopRewindForDuration()
{
	if (!GameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode is not set in URewindComponent::StopRewindForDuration"));
		return;
	}

	bIsRewindingForDuration = false;
	GameMode->SetRewindSpeedNormal();
	GameMode->StopGlobalRewind();
	OnGlobalRewindCompleted();
}

void URewindComponent::TimeScrubForDuration(float Duration)
{
	if (!GameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode is not set in URewindComponent::TimeScrubForDuration"));
		return;
	}

	TotalTimeScrub = Duration;
	TimeScrubStartedAt = GetWorld()->GetTimeSeconds();

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

	if (bIsTimeScrubbingForDuration)
	{
		GetWorld()->GetTimerManager().ClearTimer(RewindTimerHandle);
	}

	GameMode->ToggleTimeScrub();

	bIsTimeScrubbingForDuration = false;
}

void URewindComponent::TimeScrubForDurationDeath()
{
	if (bIsTimeScrubbingForDuration)
	{
		GetWorld()->GetTimerManager().ClearTimer(RewindTimerHandle);
	}

	bIsTimeScrubbingForDuration = false;
}

float URewindComponent::GetRemainingTimeScrub() const
{
	if (bIsTimeScrubbingForDuration)
	{
		return TotalTimeScrub - (GetWorld()->GetTimeSeconds() - TimeScrubStartedAt);
	}
	else
	{
		return 0.0f;
	}
}

float URewindComponent::GetTotalTimeScrub() const
{
	return TotalTimeScrub;
}

//redacted
bool URewindComponent::IsLatestSnapshotLocationSafe() const
{
	if (LatestSnapshotIndex >= 0 && LatestSnapshotIndex < TransformAndVelocitySnapshots.Num())
	{
		return TransformAndVelocitySnapshots[LatestSnapshotIndex].bIsLocationSafe;
	}
	return false;
}

//redacted
void URewindComponent::CheckSafeLocationAfterRewind()
{
	if (IsLatestSnapshotLocationSafe())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Latest Snapshot Location is Safe"));
		StopRewindForDuration();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Latest Snapshot Location is Not Safe"));
		bContinueRewindUntilSafe = true;
	}
}