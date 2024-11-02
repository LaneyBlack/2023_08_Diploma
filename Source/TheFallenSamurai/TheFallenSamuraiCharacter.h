// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "AbilitySystemInterface.h"
#include "TheFallenSamuraiCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class URewindComponent;
class APlayerGameModeBase;

UENUM(BlueprintType)
enum class ENoJumpState : uint8
{
	None UMETA(DisplayName = "None"),
	Crouch UMETA(DisplayName = "Crouch")
};

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

class UComboSystem;

UCLASS(config=Game)

class ATheFallenSamuraiCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Rewind component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rewind", meta = (AllowPrivateAccess = "true"))
	URewindComponent* RewindComponent;

	/** Toggle Rewind Participation Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleRewindParticipationAction;

	UFUNCTION(BlueprintCallable, Category = "Rewind")
	void ToggleRewindParticipationNoInput();

	/** Rewind Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RewindAction;

	/** Time Scrub Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleTimeScrubAction;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* ThirdPersonCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PerfectParryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SuperAbilityAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	float LookRotationScale = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatSystem, meta = (AllowPrivateAccess = "true"))
	class UCombatSystemComponent* CombatSystemComponent;


public:
	UPROPERTY(BlueprintReadWrite, Category = "Tutorial", meta = (AllowPrivateAccess = "true"))
	bool LockPlayerAttack = false;

	UPROPERTY(BlueprintReadWrite, Category = "Tutorial", meta = (AllowPrivateAccess = "true"))
	bool LockPlayerPerfectParry = false;

	ATheFallenSamuraiCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Parkour", meta = (AllowPrivateAccess = "true"))
	bool bIsWallrunJumping = false;

	UFUNCTION(BlueprintCallable, Category = "NoJump")
	void SetNoJumpState(ENoJumpState NewNoJumpState)
	{
		NoJumpState = NewNoJumpState;
	}

	UFUNCTION(BlueprintCallable, Category = "NoJump")
	void ResetNoJumpState()
	{
		NoJumpState = ENoJumpState::None;
	}

	UFUNCTION(BlueprintCallable, Category = "Jump")
	void ResetDoubleJump();

protected:
	/** Called for player attack action */
	void Attack(const FInputActionValue& Value);

	/** Called for player parry action */
	void PerfectParry(const FInputActionValue& Value);


	/** Called for rewind input */
	void Rewind(const FInputActionValue& Value);

	/** Called for rewind input */
	void StopRewinding(const FInputActionValue& Value);

	/** Called when toggling rewind participation */
	void ToggleRewindParticipation(const FInputActionValue& Value);

	/** Called for time scrub input */
	void ToggleTimeScrub(const FInputActionValue& Value);

	/** Called for Super Ability Execution */
	void ToggleSuperAbility(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS", meta = (AllowPrivateAccess = true))
	class UAbilitySystemComponent* AbilitySystemComponent;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override
	{
		return AbilitySystemComponent;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS", meta = (AllowPrivateAccess = true))
	const class UPlayerAttributeSet* PlayerAttributeSet;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	// Reset Double Jump
	virtual void Landed(const FHitResult& Hit) override;

	bool bFirstJump = true;
	
	bool bDoubleJumpingFromGround = false;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return ThirdPersonCamera; }

	void DoubleJump();

private:
	// Game mode for driving global time manipulation operations
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	APlayerGameModeBase* GameMode;
	
	void DoubleJumpLogic();
	
	void ResetCombo();

	ENoJumpState NoJumpState;
};

