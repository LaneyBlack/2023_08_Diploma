// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "CombatSystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIFramesChanged, bool, bIsImmortal);

class AKatana;
class UCameraShakeBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THEFALLENSAMURAI_API UCombatSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatSystemComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:	

	UPROPERTY()
	class ACharacter* playerCharacter;

	UPROPERTY()
	AKatana* Katana;

	UPROPERTY()
	class UDidItHitActorComponent* HitTracer;

	float MaxViewPitchValue = 35.f;

	float MinViewPitchValue = -70.f;

	bool bIsAttacking;

	bool bInterputedByItself;

	class UAnimInstance* AnimInstance;

	UAnimMontage* CurrentAttackMontage;

	class APlayerCameraManager* PlayerCameraManager;

	FVector KatanaPreviousPosition;

	FName KatanaSocketForDirection = "TraceEnd";

	FTimerHandle EnemiesTraceTimerHandle = FTimerHandle();

	FRotator CameraRotationRate;

	FRotator CurrentCameraRotation;

	FVector HandSwayLookOffset;

	FVector LocationLagPosition;

	float KatanaTriggerLenSquared;

	float TeleportTriggerLength;

	float CharacterArmsLength;

	bool bShouldIgnoreTeleport = false;

	//FTimerHandle TeleportTimerHandle = FTimerHandle();

	FTimeline TeleportTimeline;

	FVector PlayerStartForTeleport;

	FVector PlayerDestinationForTeleport;

	FRotator PlayerOnTeleportRotation;

	FRotator RotationToEnemy;

	float PlayerCameraFOV;

	int StolenTokens = 0;

	FTimeline ParrySlowMoTimeline;

	float TimeDilationBeforeParry = 1.f;

	bool bShouldSpeedUpSlowMoTimeline = false;

	float DebugTimeStamp;

	UFUNCTION()
	bool CheckIfCanAttack();

	UFUNCTION()
	UAnimMontage* DetermineNextMontage();

	UFUNCTION()
	void HandleAttackEnd();

	UFUNCTION()
	void ProcessHitReaction(AActor* HitActor, FVector ImpactPoint);

	UFUNCTION()
	FVector DetermineKatanaDirection();

	UFUNCTION()
	FVector GetKatanaSocketWorldPosition(FName SocketName);

	UFUNCTION()
	void GetEnemiesInViewportOnAttack();

	UFUNCTION()
	void GetLookInputVariables(FRotator PreviousCameraRotation);

	UFUNCTION()
	void GetVelocityVariables();

	/*UFUNCTION()
	void TraceForEnemiesToTeleport();*/

	UFUNCTION()
	void TeleportToClosestEnemy(ABaseEnemy* Enemy);

public:

	UPROPERTY(EditAnywhere, Category = "Attack Data|Animation")
	TArray<UAnimMontage*> AttackMontages;

	UPROPERTY(EditAnywhere, Category = "Attack Data|Animation")
	float AttackSpeedMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Attack Data|Camera Shakes")
	TSubclassOf<UCameraShakeBase> AttackCameraShake;

	UPROPERTY(EditAnywhere, Category = "Attack Data|VFX")
	class UParticleSystem* BloodParticles;

	UPROPERTY(EditAnywhere, Category = "Attack Data|VFX")
	FVector BloodScale = FVector(.6f, .6f, .8f);

	UPROPERTY(EditAnywhere, Category = "Attack Data|Katana Collider")
	float KatanaBladeTriggerScale = 2.f;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|Animation")
	UAnimMontage* PerfectParryMontage;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|Animation")
	float PerfectParryMontageSpeed = 1.4f;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|Animation")
	UAnimMontage* ParryImpactMontage;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|Animation")
	float ParryImpactMontageSpeed = 1.3f;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|Camera Shakes")
	TSubclassOf<UCameraShakeBase> ParryCameraShake;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|Slow-mo")
	UCurveFloat* SlowMoCurve;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|Slow-mo")
	float MinTimeDilation = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|VFX")
	class UParticleSystem* PerfectParrySparks;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|VFX")
	FVector PerfectParrySparksSize = FVector(3.f, 3.f, 3.f);

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|VFX")
	class UParticleSystem* PerfectParryShockwave;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|VFX")
	FVector PerfectParryShockwaveSize = FVector(1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, Category = "Teleport Data")
	float TeleportTriggerScale = 3.f;

	UPROPERTY(EditAnywhere, Category = "Teleport Data")
	float TotalTeleportTime = .3f;

	UPROPERTY(EditAnywhere, Category = "Teleport Data|Interpolation Curves")
	UCurveFloat* LocationCurve;

	UPROPERTY(EditAnywhere, Category = "Teleport Data|Interpolation Curves")
	UCurveFloat* RotationCurve;

	UPROPERTY(EditAnywhere, Category = "Teleport Data|Interpolation Curves")
	UCurveFloat* FOVCurve;

	UPROPERTY(EditAnywhere, Category = "Super Ability")
	int MaxStolenTokens = 3;

	UPROPERTY(BlueprintAssignable)
	FOnIFramesChanged OnIFramesChanged;

	UPROPERTY(BlueprintReadOnly)
	bool bCanRigUpdate;
	
	UPROPERTY(BlueprintReadOnly)
	bool bInCombat;

	UPROPERTY(BlueprintReadOnly)
	bool bInParry;

	UPROPERTY(BlueprintReadOnly)
	FVector HandTotalOffset;

	UPROPERTY(BlueprintReadOnly)
	float YawSwayValue;

	UPROPERTY(BlueprintReadOnly)
	FVector TargetPointOffset;

	UPROPERTY(BlueprintReadOnly)
	bool bInTeleport;

	UFUNCTION(BlueprintCallable)
	void InitializeCombatSystem(ACharacter* player, TSubclassOf<AKatana> KatanaActor);

	UFUNCTION(BlueprintCallable)
	void Attack();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	void GetLeftTransforms(FTransform& KatanaGripWorldTransform, FTransform& LeftHandSocket, FTransform& RightHandSocket);

	UFUNCTION(BlueprintCallable)
	void PerfectParry();

	UFUNCTION(BlueprintCallable)
	void InterruptPerfectParry();

	UFUNCTION(BlueprintCallable)
	void PerfectParryResponse(int InTokens, bool bEnableSlowMo);

	UFUNCTION(BlueprintCallable)
	void SpeedUpSlowMoTimeline();

	UFUNCTION()
	void PlayMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void PlayMontageNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void PlayMontageFinished(class UAnimMontage* MontagePlayed, bool bWasInterrupted);

	UFUNCTION()
	void TimelineProgessLocation(float Value);

	UFUNCTION()
	void TimelineProgessRotation(float Value);

	UFUNCTION()
	void TimelineProgessFOV(float Value);

	UFUNCTION()
	void TimelineProgessSlowMo(float Value);

	UFUNCTION()
	void TeleportTimelineFinish();

	UFUNCTION()
	void SlowMoTimelineFinish();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	
};
