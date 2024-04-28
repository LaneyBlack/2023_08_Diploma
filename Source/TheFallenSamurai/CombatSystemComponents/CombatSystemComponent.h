// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "CombatSystemComponent.generated.h"

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

	FRotator RotationToEnemy;

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

	UPROPERTY(EditAnywhere, Category = "Attack Data")
	TArray<UAnimMontage*> AttackMontages;

	UPROPERTY(EditAnywhere, Category = "Attack Data")
	float AttackSpeedMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Attack Data")
	TSubclassOf<UCameraShakeBase> AttackCamShake;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data")
	UAnimMontage* PerfectParryMontage;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data")
	TSubclassOf<UCameraShakeBase> ParryCamShake;

	UPROPERTY(EditAnywhere, Category = "Combat VFX")
	class UParticleSystem* BloodParticles;

	UPROPERTY(EditAnywhere, Category = "Combat VFX")
	FVector BloodScale = FVector(.6f, .6f, .8f);

	UPROPERTY(EditAnywhere, Category = "Combat VFX")
	class UParticleSystem* PerfectParryParticles;

	UPROPERTY(EditAnywhere, Category = "Katana Collider")
	float KatanaBladeTriggerScale = 2.f;

	UPROPERTY(EditAnywhere, Category = "Teleport Data")
	float TeleportTriggerScale = 3.f;

	UPROPERTY(EditAnywhere, Category = "Teleport Data")
	float TotalTeleportTime = .3f;

	UPROPERTY(EditAnywhere, Category = "Teleport Data")
	UCurveFloat* TeleportCurve;

	UPROPERTY(EditAnywhere, Category = "Teleport Data")
	UCurveFloat* FOVCurve;

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
	void PerfectParryResponse();

	UFUNCTION()
	void PlayMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void PlayMontageNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void PlayMontageFinished(class UAnimMontage* MontagePlayed, bool bWasInterrupted);

	UFUNCTION()
	void TimelineProgess(float Value);

	UFUNCTION()
	void EnablePlayerVariables();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	
};
