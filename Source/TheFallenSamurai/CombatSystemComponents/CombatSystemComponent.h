// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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

	float MinViewPitchValue = -80.f;

	bool bIsAttacking;

	bool bInterputedByItself;

	class UAnimInstance* AnimInstance;

	class APlayerCameraManager* PlayerCameraManager;

	FVector KatanaPreviousPosition;

	FName KatanaSocketForDirection = "TraceEnd";

	FTimerHandle EnemiesTraceTimerHandle = FTimerHandle();

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

public:

	UPROPERTY(EditAnywhere, Category = "Combat Animations")
	TArray<UAnimMontage*> AttackMontages;

	UPROPERTY(EditAnywhere, Category = "Combat Animations")
	float AttackSpeedMultiplier = 1.5f;

	UPROPERTY(BlueprintReadOnly)
	bool bCanRigUpdate;
	
	UPROPERTY(BlueprintReadOnly)
	bool bInCombat;

	UPROPERTY(EditAnywhere, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> AttackCamShake;

	UPROPERTY(EditAnywhere, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> ParryCamShake;

	UPROPERTY(EditAnywhere, Category = "Combat VFX")
	class UParticleSystem* BloodParticles;

	UPROPERTY(EditAnywhere, Category = "Combat VFX")
	FVector BloodScale = FVector(.6f, .6f, .8f);

	UPROPERTY(EditAnywhere, Category = "Combat VFX")
	class UParticleSystem* PerfectParryParticles;

	UPROPERTY(BlueprintReadOnly)
	FVector TargetPointOffset;

	UFUNCTION(BlueprintCallable)
	void InitializeCombatSystem(ACharacter* player, TSubclassOf<AKatana> KatanaActor);

	UFUNCTION(BlueprintCallable)
	void Attack();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	void GetLeftTransforms(FTransform& KatanaGripWorldTransform, FTransform& LeftHandSocket, FTransform& RightHandSocket);

	UFUNCTION()
	void PlayMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void PlayMontageNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void PlayMontageFinished(class UAnimMontage* MontagePlayed, bool bWasInterrupted);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	
};
