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
#include "CombatSystemComponents\CombatSystemComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/PlayerAttributeSet.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ATheFallenSamuraiCharacter

ATheFallenSamuraiCharacter::ATheFallenSamuraiCharacter()
{
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
	ThirdPersonCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
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

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	//Initialize AttributeSets
	if(IsValid(AbilitySystemComponent))
	{
		PlayerAttributeSet = AbilitySystemComponent -> GetSet<UPlayerAttributeSet>();
	}

	//Reset combo on start
	ResetCombo();
}

void ATheFallenSamuraiCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	bFirstJump = true;
	bDoubleJumpingFromGround = false;
	bIsWallrunJumping = false;
}

void ATheFallenSamuraiCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (GetCharacterMovement()->MovementMode == MOVE_None)
	{
		bFirstJump = true;
		bDoubleJumpingFromGround = false;
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
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATheFallenSamuraiCharacter::DoubleJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATheFallenSamuraiCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATheFallenSamuraiCharacter::Look);

		//Attack
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, CombatSystemComponent,
			&UCombatSystemComponent::Attack);

		//Perfect Parry
		EnhancedInputComponent->BindAction(PerfectParryAction, ETriggerEvent::Started, CombatSystemComponent,
			&UCombatSystemComponent::PerfectParry);

		//Perfect Parry Interrupt
		/*EnhancedInputComponent->BindAction(PerfectParryAction, ETriggerEvent::Completed, CombatSystemComponent,
			&UCombatSystemComponent::InterruptPerfectParry);*/

		//Super Ability
		EnhancedInputComponent->BindAction(SuperAbilityAction, ETriggerEvent::Started, CombatSystemComponent,
			&UCombatSystemComponent::SuperAbility);

		////Cancel Super Ability
		//EnhancedInputComponent->BindAction(SuperAbilityAction, ETriggerEvent::Completed, CombatSystemComponent,
		//	&UCombatSystemComponent::CancelSuperAbility);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
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
	}
}