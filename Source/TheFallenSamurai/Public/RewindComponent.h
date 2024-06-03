// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "Runtime/Core/Public/Containers/RingBuffer.h"

#include "RewindComponent.generated.h"

class UCharacterMovementComponent;
class USkeletalMeshComponent;
class APlayerGameModeBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeManipulationStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeManipulationCompleted);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRewindStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRewindCompleted);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeScrubStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeScrubCompleted);

USTRUCT()
struct FTransformAndVelocitySnapshot
{
	GENERATED_BODY();
	
	UPROPERTY(Transient)
	float TimeSinceLastSnapshot = 0.0f;
	
	UPROPERTY(Transient)
	FTransform Transform{ FVector::ZeroVector };
	
	UPROPERTY(Transient)
	FVector LinearVelocity = FVector::ZeroVector;
	
	UPROPERTY(Transient)
	FVector AngularVelocityInRadians = FVector::ZeroVector;

	UPROPERTY(Transient)
	bool bIsLocationSafe;
};

USTRUCT()
struct FMovementVelocityAndModeSnapshot
{
	GENERATED_BODY();
	
	UPROPERTY(Transient)
	float TimeSinceLastSnapshot = 0.0f;
	
	UPROPERTY(Transient)
	FVector MovementVelocity = FVector::ZeroVector;
	
	UPROPERTY(Transient)
	TEnumAsByte<enum EMovementMode> MovementMode = EMovementMode::MOVE_None;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THEFALLENSAMURAI_API URewindComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Rewind")
	float SnapshotFrequencySeconds = 1.0f / 30.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Rewind")
	bool bSnapshotMovementVelocityAndMode = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "Rewind")
	bool bPauseAnimationDuringTimeScrubbing = true;

	UPROPERTY(BlueprintReadOnly, Category = "Rewind")
	bool bIsTimeScrubbingForDuration = false;
	
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnTimeManipulationStarted OnTimeManipulationStarted;
	
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnTimeManipulationCompleted OnTimeManipulationCompleted;
	
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnRewindStarted OnRewindStarted;
	
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnRewindCompleted OnRewindCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnTimeScrubStarted OnTimeScrubStarted;

	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnTimeScrubCompleted OnTimeScrubCompleted;

	UFUNCTION(BlueprintCallable, Category = "Rewind")
	float GetRemainingTimeScrub() const;

	UFUNCTION(BlueprintCallable, Category = "Rewind")
	float GetTotalTimeScrub() const;

protected:
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Rewind")
	bool bIsRewinding = false;

protected:
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Rewind")
	bool bIsTimeScrubbing = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	bool IsTimeScrubbing() const { return bIsTimeScrubbing; };
	
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	bool IsRewinding() const { return bIsRewinding; };

public:
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	bool IsTimeBeingManipulated() const { return bIsRewinding || bIsTimeScrubbing; };

	UFUNCTION(BlueprintCallable, Category = "Rewind")
	void RewindForDuration(float Duration);

	UFUNCTION(BlueprintCallable, Category = "Rewind")
	void TimeScrubForDuration(float Duration);

	UFUNCTION(BlueprintCallable, Category = "Rewind")
	void StopTimeScrubForDuration();

	void StopRewindForDuration();

private:
	UPROPERTY(VisibleAnywhere, Category = "Rewind")
	bool bIsRewindingEnabled = true;

public:
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	bool IsRewindingEnabled() const { return bIsRewindingEnabled; }
	
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	void SetIsRewindingEnabled(bool bEnabled);

public:
	URewindComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	TRingBuffer<FTransformAndVelocitySnapshot> TransformAndVelocitySnapshots;
	
	TRingBuffer<FMovementVelocityAndModeSnapshot> MovementVelocityAndModeSnapshots;

	bool PerformSafetyTrace(const FVector& Location) const;
	
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	uint32 MaxSnapshots = 1;
	
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	float TimeSinceSnapshotsChanged = 0.0f;

	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	int32 LatestSnapshotIndex = -1;
	
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	UPrimitiveComponent* OwnerRootComponent;
	
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	UCharacterMovementComponent* OwnerMovementComponent;
	
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	USkeletalMeshComponent* OwnerSkeletalMesh;
	
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	bool bPausedPhysics = false;
	
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	bool bPausedAnimation = false;
	
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	bool bAnimationsPausedAtStartOfTimeManipulation = false;
	
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	bool bLastTimeManipulationWasRewind = true;
	
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	APlayerGameModeBase* GameMode;

	bool bIsRewindingForDuration = false;
	
	float RemainingRewindDuration = 0.0f;

	float TotalTimeScrub = 0.0f;

	float TimeScrubStartedAt = 0.0f;

	bool bContinueRewindUntilSafe = false;
	
	bool IsLatestSnapshotLocationSafe() const;
	
	void CheckSafeLocationAfterRewind();
	
	void CompleteRewind();
	
	UFUNCTION()
	void OnGlobalRewindStarted();
	
	UFUNCTION()
	void OnGlobalRewindCompleted();

	UFUNCTION()
	void OnGlobalTimeScrubStarted();

	UFUNCTION()
	void OnGlobalTimeScrubCompleted();
	
	void InitializeRingBuffers(float MaxRewindSeconds);
	
	void RecordSnapshot(float DeltaTime);
	
	void EraseFutureSnapshots();
	
	void PlaySnapshots(float DeltaTime, bool bRewinding);
	
	void PauseTime(float DeltaTime, bool bRewinding);
	
	bool TryStartTimeManipulation(bool& bStateToSet, bool bResetTimeSinceSnapshotsChanged);
	
	bool TryStopTimeManipulation(bool& bStateToSet, bool bResetTimeSinceSnapshotsChanged, bool bResetMovementVelocity);
	
	void PausePhysics();
	
	void UnpausePhysics();
	
	void PauseAnimation();
	
	void UnpauseAnimation();
	
	bool HandleInsufficientSnapshots();
	
	void InterpolateAndApplySnapshots(bool bRewinding);
	
	FTransformAndVelocitySnapshot BlendSnapshots(
		const FTransformAndVelocitySnapshot& A,
		const FTransformAndVelocitySnapshot& B,
		float Alpha);
	
	FMovementVelocityAndModeSnapshot BlendSnapshots(
		const FMovementVelocityAndModeSnapshot& A,
		const FMovementVelocityAndModeSnapshot& B,
		float Alpha);
	
	void ApplySnapshot(const FTransformAndVelocitySnapshot& Snapshot, bool bApplyPhysics);
	
	void ApplySnapshot(const FMovementVelocityAndModeSnapshot& Snapshot, bool bApplyTimeDilationToVelocity);

	FTimerHandle RewindTimerHandle;
};
