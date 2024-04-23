// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
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

class ATheFallenSamuraiCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Rewind component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rewind", meta = (AllowPrivateAccess = "true"))
	URewindComponent* RewindComponent;

	/** Rewind Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RewindAction;

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

public:
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

protected:
	/** Called for rewind input */
	void Rewind(const FInputActionValue& Value);

	/** Called for rewind input */
	void StopRewinding(const FInputActionValue& Value);

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

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

