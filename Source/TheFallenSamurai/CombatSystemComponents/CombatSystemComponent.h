// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatSystemComponent.generated.h"

class AKatana;

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

	UFUNCTION()
	bool CheckIfCanAttack();

	UFUNCTION()
	UAnimMontage* DetermineNextMontage();

	UFUNCTION()
	void HandleAttackEnd();

public:

	UPROPERTY(EditAnywhere, Category = "Combat Animations")
	TArray<UAnimMontage*> AttackMontages;

	UPROPERTY(EditAnywhere, Category = "Combat Animations")
	float AttackSpeedMultiplier;

	UPROPERTY(EditAnywhere)
	bool bCanRigUpdate;
	
	UPROPERTY(EditAnywhere)
	bool bInCombat;

	UPROPERTY(EditAnywhere)
	FVector TargetPointOffset;

	UFUNCTION(BlueprintCallable)
	void InitializeCombatSystem(ACharacter* player, TSubclassOf<AKatana> KatanaActor);

	UFUNCTION(BlueprintCallable)
	void Attack();

	UFUNCTION()
	void PlayMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void PlayMontageNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void PlayMontageFinished(class UAnimMontage* MontagePlayed, bool bWasInterrupted);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	
};
