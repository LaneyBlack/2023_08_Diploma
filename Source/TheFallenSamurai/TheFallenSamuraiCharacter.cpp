// Copyright Epic Games, Inc. All Rights Reserved.

#include "TheFallenSamuraiCharacter.h"

#include "ComboSystem.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "RewindComponent.h"
#include "PlayerGameModeBase.h"
#include "CombatSystemComponents\CombatSystemComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/PlayerAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TimelineComponent.h"
#include "Kismet/KismetMathLibrary.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ATheFallenSamuraiCharacter


#define PRINT(mess, mtime)  GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::Green, TEXT(mess));
#define PRINTC(mess, color)  GEngine->AddOnScreenDebugMessage(-1, 3, color, TEXT(mess));
#define PRINT_F(prompt, mess, mtime) GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::Green, FString::Printf(TEXT(prompt), mess));
#define PRINTC_F(prompt, mess, mtime, color) GEngine->AddOnScreenDebugMessage(-1, mtime, color, FString::Printf(TEXT(prompt), mess));
//#define PRINT_B(prompt, mess) GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Green, FString::Printf(TEXT(prompt), mess ? TEXT("TRUE") : TEXT("FALSE")));


void ATheFallenSamuraiCharacter::EnableJumpLock()
{
	//PRINT("finihed timeline", 3);
	bLockedJump = true;
}

void ATheFallenSamuraiCharacter::InterpolateGravity(float Value)
{
	//PRINT("timeline", 3);
	GetCharacterMovement()->GravityScale = UKismetMathLibrary::Lerp(MinGravity, GravityCache, Value);
}

void ATheFallenSamuraiCharacter::ResetCoyoteProperties()
{
	bLockedJump = false;

	GetCharacterMovement()->GravityScale = GravityCache;
	CoyoteGravityTimeline.Stop();
}

ATheFallenSamuraiCharacter::ATheFallenSamuraiCharacter()
{
	// Setup a rewind component that snapshots 30 times per second
	RewindComponent = CreateDefaultSubobject<URewindComponent>(TEXT("RewindComponent"));
	RewindComponent->SnapshotFrequencySeconds = 1.0f / 30.0f;
	RewindComponent->bSnapshotMovementVelocityAndMode = true;
	RewindComponent->bPauseAnimationDuringTimeScrubbing = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	ThirdPersonCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	ThirdPersonCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	bFirstJump = true;
	bDoubleJumpingFromGround = false;

	//setup combat system component
	CombatSystemComponent = CreateDefaultSubobject<UCombatSystemComponent>(TEXT("CombatSystem_cpp"));

	//setup GAS
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ATheFallenSamuraiCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Capture game mode for driving global rewind
	GameMode = Cast<APlayerGameModeBase>(GetWorld()->GetAuthGameMode());

	/*if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Equals("Level_0_Tutorial"))
		LockPlayerPerfectParry = true;*/

	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Equals("Level_0_Tutorial"))
		bLockPlayerAbilities = true;

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	//Initialize AttributeSets
	if (IsValid(AbilitySystemComponent))
	{
		PlayerAttributeSet = AbilitySystemComponent->GetSet<UPlayerAttributeSet>();
	}

	//Reset combo on start
	ResetCombo();

	//set up coyote time timeline dependencies
	GravityCache = GetCharacterMovement()->GravityScale;
	if (bUseGravityTimeline)
	{
		//PRINTC_F("Gravity Cache = %f", GravityCache, 2, FColor::Red);

		FOnTimelineFloat TimelineProgressGravity;
		TimelineProgressGravity.BindUFunction(this, FName("InterpolateGravity"));
		CoyoteGravityTimeline.AddInterpFloat(GravityCurve, TimelineProgressGravity);

		FOnTimelineEvent GravityTimelineFinished;
		GravityTimelineFinished.BindUFunction(this, FName("EnableJumpLock"));
		CoyoteGravityTimeline.SetTimelineFinishedFunc(GravityTimelineFinished);

		CoyoteGravityTimeline.SetLooping(false);

		CoyoteGravityTimeline.SetPlayRate(1.f / CoyoteTime);
	}
}

void ATheFallenSamuraiCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*PRINT_F("MOVEMENT STATE: %s", *UEnum::GetValueAsString(GetCharacterMovement()->MovementMode), 0.f);
	PRINT_F("bLockedJump: %i", bLockedJump, 0.f);*/

	if (bUseGravityTimeline)
		CoyoteGravityTimeline.TickTimeline(DeltaTime);


}

bool ATheFallenSamuraiCharacter::CanJumpInternal_Implementation() const
{
	//PRINT("can jump internal", 3)
	return Super::CanJumpInternal_Implementation() || !bLockedJump;
	//return Super::CanJumpInternal_Implementation();
}

void ATheFallenSamuraiCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	ResetDoubleJump();

	GetWorld()->GetTimerManager().ClearTimer(CoyoteTimeTimer);

	ResetCoyoteProperties();
}

void ATheFallenSamuraiCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (GetCharacterMovement()->MovementMode == MOVE_None)
	{
		bFirstJump = true;
		bDoubleJumpingFromGround = false;

		ResetCoyoteProperties();
	}
	else if (GetCharacterMovement()->MovementMode == MOVE_Falling && bFirstJump)
	{
		if (!bUseGravityTimeline)
			GetWorld()->GetTimerManager().SetTimer(CoyoteTimeTimer, this, &ATheFallenSamuraiCharacter::EnableJumpLock,
			                                       CoyoteTime, false);
		else
			CoyoteGravityTimeline.PlayFromStart();
	}
}

void ATheFallenSamuraiCharacter::DoubleJump()
{
	if (NoJumpState == ENoJumpState::None)
	{
		if (bFirstJump && !bDoubleJumpingFromGround)
		{
			bFirstJump = false;
			bDoubleJumpingFromGround = true;

			CoyoteGravityTimeline.Stop();
			GetCharacterMovement()->GravityScale = GravityCache;

			ACharacter::Jump();
		}
		else if (bIsWallrunJumping && !bDoubleJumpingFromGround && !bFirstJump)
		{
			DoubleJumpLogic();
		}
		else if (!bFirstJump && bDoubleJumpingFromGround)
		{
			DoubleJumpLogic();
		}
	}
}

void ATheFallenSamuraiCharacter::DoubleJumpLogic()
{
	PRINTC("double jump", FColor::Red);

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController)
	{
		FVector LaunchDirection = GetLastMovementInputVector();
		if (LaunchDirection.IsNearlyZero())
		{
			LaunchDirection = FVector(0, 0, 1);;
		}
		else
		{
			LaunchDirection = GetLastMovementInputVector();
		}

		LaunchDirection.Normalize();
		FVector LaunchVelocity = LaunchDirection * 750.0f;
		LaunchVelocity.Z = 750.0f;

		LaunchCharacter(LaunchVelocity, false, true);

		bIsWallrunJumping = false;
		bDoubleJumpingFromGround = false;
	}
}

void ATheFallenSamuraiCharacter::ResetDoubleJump()
{
	bFirstJump = true;
	bDoubleJumpingFromGround = false;
	bIsWallrunJumping = false;
}


void ATheFallenSamuraiCharacter::ResetCombo()
{
	if (UComboSystem* ComboSystem = UComboSystem::GetInstance())
	{
		ComboSystem->ResetCombo();
		ComboSystem->EndKillStreak();
		ComboSystem->ResetComboState();
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATheFallenSamuraiCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this,
		                                   &ATheFallenSamuraiCharacter::DoubleJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this,
		                                   &ATheFallenSamuraiCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this,
		                                   &ATheFallenSamuraiCharacter::Look);

		//Attack
		/*EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, CombatSystemComponent,
			&UCombatSystemComponent::Attack);*/

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this,
		                                   &ATheFallenSamuraiCharacter::Attack);

		//Perfect Parry
		/*EnhancedInputComponent->BindAction(PerfectParryAction, ETriggerEvent::Started, CombatSystemComponent,
			&UCombatSystemComponent::PerfectParry);*/

		EnhancedInputComponent->BindAction(PerfectParryAction, ETriggerEvent::Started, this,
		                                   &ATheFallenSamuraiCharacter::PerfectParry);

		//Super Ability
		/*EnhancedInputComponent->BindAction(SuperAbilityAction, ETriggerEvent::Started, CombatSystemComponent,
			&UCombatSystemComponent::SuperAbility);*/

		EnhancedInputComponent->BindAction(SuperAbilityAction, ETriggerEvent::Started, this,
		                                   &ATheFallenSamuraiCharacter::ToggleSuperAbility);

		// Rewind
		EnhancedInputComponent->BindAction(RewindAction, ETriggerEvent::Started, this,
		                                   &ATheFallenSamuraiCharacter::Rewind);
		EnhancedInputComponent->BindAction(RewindAction, ETriggerEvent::Completed, this,
		                                   &ATheFallenSamuraiCharacter::StopRewinding);

		// Toggle Rewind Participation
		EnhancedInputComponent->BindAction(ToggleRewindParticipationAction, ETriggerEvent::Started, this,
		                                   &ATheFallenSamuraiCharacter::ToggleRewindParticipation);

		// Toggle Time Scrub
		EnhancedInputComponent->BindAction(ToggleTimeScrubAction, ETriggerEvent::Started, this,
		                                   &ATheFallenSamuraiCharacter::ToggleTimeScrub);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error,
		       TEXT(
			       "'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."
		       ), *GetNameSafe(this));
	}
}

void ATheFallenSamuraiCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ATheFallenSamuraiCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>() * LookRotationScale * CombatSystemComponent->GetLookRate();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);

		DesiredCharacterYaw = Cast<APlayerController>(Controller)->RotationInput.Yaw;
	}
}

void ATheFallenSamuraiCharacter::Attack(const FInputActionValue& Value)
{
	if (!LockPlayerAttack)
		CombatSystemComponent->Attack();
}

void ATheFallenSamuraiCharacter::PerfectParry(const FInputActionValue& Value)
{
	if (!LockPlayerPerfectParry)
		CombatSystemComponent->PerfectParry();
}

void ATheFallenSamuraiCharacter::Rewind(const FInputActionValue& Value)
{
	check(GameMode);
	if (GameMode) { GameMode->StartGlobalRewind(); }
}

void ATheFallenSamuraiCharacter::StopRewinding(const FInputActionValue& Value)
{
	check(GameMode);
	if (GameMode) { GameMode->StopGlobalRewind(); }
}

void ATheFallenSamuraiCharacter::ToggleRewindParticipation(const FInputActionValue& Value)
{
	RewindComponent->SetIsRewindingEnabled(!RewindComponent->IsRewindingEnabled());
}

void ATheFallenSamuraiCharacter::ToggleRewindParticipationNoInput()
{
	RewindComponent->SetIsRewindingEnabled(!RewindComponent->IsRewindingEnabled());
}

void ATheFallenSamuraiCharacter::ToggleTimeScrub(const FInputActionValue& Value)
{
	if (bLockPlayerAbilities)
		return;

	UComboSystem* ComboSystem = UComboSystem::GetInstance();

	if (!CombatSystemComponent->IsSuperAbilityActive())
	{
		if (RewindComponent->bIsTimeScrubbingForDuration)
		{
			RewindComponent->StopTimeScrubForDuration();
		}
		else if (ComboSystem->AbilityComboPoints >= ComboSystem->TimeStopCost)
		{
			ComboSystem->AbilityComboPoints -= ComboSystem->TimeStopCost;
			ToggleRewindParticipationNoInput();
			RewindComponent->TimeScrubForDuration(5.0f);
		}
		else
		{
			ComboSystem->OnTimeStopCalled.Broadcast("Not enough Combo Points");
		}
	}
}

void ATheFallenSamuraiCharacter::ToggleSuperAbility(const FInputActionValue& Value)
{
	if (bLockPlayerAbilities)
		return;

	if (!RewindComponent->bIsTimeScrubbingForDuration)
		CombatSystemComponent->SuperAbility();
}
