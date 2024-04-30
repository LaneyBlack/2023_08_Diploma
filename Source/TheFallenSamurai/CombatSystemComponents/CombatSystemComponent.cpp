// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatSystemComponent.h"

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Kismet/GameplayStatics.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

#include "DidItHitActorComponent.h"

#include "TheFallenSamurai/KatanaSource/Katana.h"
#include "TheFallenSamurai/BaseEnemySource/BaseEnemy.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

//DEBUG
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UCombatSystemComponent::UCombatSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UCombatSystemComponent::BeginPlay()
{
	Super::BeginPlay();	
}

bool UCombatSystemComponent::CheckIfCanAttack()
{
	return !bIsAttacking && (bInterputedByItself || !AttackMontages.Contains(playerCharacter->GetCurrentMontage()));
}

UAnimMontage* UCombatSystemComponent::DetermineNextMontage()
{
	/*auto montage = AttackMontages[FMath::RandRange(0, AttackMontages.Num() - 1)];
	return montage;*/

	static int index = 0;
	CurrentAttackMontage = AttackMontages[index++ % AttackMontages.Num()];
	return CurrentAttackMontage;
}

void UCombatSystemComponent::HandleAttackEnd()
{
	TargetPointOffset = FVector::Zero();

	GetWorld()->GetTimerManager().ClearTimer(EnemiesTraceTimerHandle);

	bIsAttacking = false;
	bInCombat = false;

	HitTracer->ToggleTraceCheck(false);

	for (auto result : HitTracer->HitArray)
		ProcessHitReaction(result.GetActor(), result.ImpactPoint);
}

void UCombatSystemComponent::ProcessHitReaction(AActor* HitActor, FVector ImpactPoint)
{
	if (auto Enemy = Cast<ABaseEnemy>(HitActor))
	{
		auto KatanaDirection = DetermineKatanaDirection();

		auto ParticleRotation = UKismetMathLibrary::FindLookAtRotation(Enemy->GetActorUpVector(), KatanaDirection);

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), 
			BloodParticles, ImpactPoint, ParticleRotation, BloodScale);

		Enemy->ApplyDamage();
	}
}

FVector UCombatSystemComponent::DetermineKatanaDirection()
{
	FVector Direction = GetKatanaSocketWorldPosition(KatanaSocketForDirection);
	Direction -= KatanaPreviousPosition;
	Direction.Normalize();

	return Direction;
}

FVector UCombatSystemComponent::GetKatanaSocketWorldPosition(FName SocketName)
{
	return Katana->KatanaMesh->GetSocketLocation(SocketName);
}

void UCombatSystemComponent::GetEnemiesInViewportOnAttack()
{
	for (auto& result : HitTracer->HitArray)
	{
		ProcessHitReaction(result.GetActor(), result.ImpactPoint);
		result.Reset();
	}

	auto CapsuleComponent = playerCharacter->GetCapsuleComponent();

	FVector HalfSize;
	//HalfSize.X = Katana->GetBladeWorldVector().Length() * TeleportTriggerScale;
	HalfSize.X = TeleportTriggerLength / 2;
	HalfSize.Y = CapsuleComponent->GetScaledCapsuleRadius() * 4.;
	HalfSize.Z = CapsuleComponent->GetScaledCapsuleHalfHeight() * 1.5;

	FVector StartEnd = playerCharacter->GetActorLocation() + playerCharacter->GetActorForwardVector() * HalfSize.X;

	//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Cyan, HalfSize.ToCompactString());

	FRotator BoxRotation = playerCharacter->GetControlRotation(); //is this ok, or revert to rotation from forward vector?
	//FRotator BoxRotation = playerCharacter->GetActorForwardVector().Rotation();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjToTrace;
	ObjToTrace.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	TArray<AActor*> Ignore;
	Ignore.Add(playerCharacter);

	FHitResult HitResult;
	bool bHit = UKismetSystemLibrary::BoxTraceSingleForObjects(GetWorld(), StartEnd, StartEnd, HalfSize, BoxRotation,
		ObjToTrace, true, Ignore, EDrawDebugTrace::None, HitResult, true, FColor::Red, FColor::Green, 1.5f);

	auto Enemy = Cast<ABaseEnemy>(HitResult.GetActor());
	if (bHit && Enemy)
	{
		auto ToEnemy = Enemy->GetActorLocation() - playerCharacter->GetActorLocation();

		float MaxAbsoluteYOffset = 45.f;
		float TargetPointYOffset = FMath::Clamp(ToEnemy.Dot(playerCharacter->GetActorRightVector()), 
			-MaxAbsoluteYOffset, MaxAbsoluteYOffset);

		TargetPointOffset = UKismetMathLibrary::VInterpTo(TargetPointOffset,
			FVector(0.f, TargetPointYOffset, 0.f),
			GetWorld()->GetDeltaSeconds(),
			25.f);

		if (!bShouldIgnoreTeleport)
		{
			auto DistanceSquaredToEnemy = ToEnemy.Length();

			/*GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Green, FString::Printf(TEXT("Collider Length = %f"), KatanaTriggerLenSquared));
			GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Green, FString::Printf(TEXT("Distance To Enemy = %f"), DistanceSquaredToEnemy));*/

			if (DistanceSquaredToEnemy > KatanaTriggerLenSquared)
			{
				TeleportToClosestEnemy(Enemy);
				GetWorld()->GetTimerManager().ClearTimer(EnemiesTraceTimerHandle);
				/*GetWorld()->GetTimerManager().SetTimer(EnemiesTraceTimerHandle, this,
					&UCombatSystemComponent::TeleportToClosestEnemy,
					1 / 120.f, true);*/
			}
		}
	}
}

void UCombatSystemComponent::GetLookInputVariables(FRotator PreviousCameraRotation)
{
	//calculate right hand sway value based on mouse look rotation
	CurrentCameraRotation = playerCharacter->GetControlRotation();
	FRotator DeltaLookRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentCameraRotation, PreviousCameraRotation);
	FRotator TargetRotationRate;

	TargetRotationRate.Roll = -UKismetMathLibrary::FClamp(DeltaLookRotation.Pitch, -5., 5.);
	TargetRotationRate.Pitch = 0.;
	TargetRotationRate.Yaw = UKismetMathLibrary::FClamp(DeltaLookRotation.Yaw, -2., 3.);

	CameraRotationRate = UKismetMathLibrary::RInterpTo(CameraRotationRate,
		TargetRotationRate, GetWorld()->GetDeltaSeconds(), 5.);

	YawSwayValue = CameraRotationRate.Yaw * 5.;

	//calculate right hand location lag based on rotation rate
	float AlphaOffsetX = UKismetMathLibrary::NormalizeToRange(CameraRotationRate.Yaw, -3., -3.);
	float AlphaOffsetZ = UKismetMathLibrary::NormalizeToRange(CameraRotationRate.Roll, -5., 5.);

	HandSwayLookOffset.X = UKismetMathLibrary::Lerp(-3., 8., AlphaOffsetX);
	HandSwayLookOffset.Z = UKismetMathLibrary::Lerp(-12., 12., AlphaOffsetZ);
}

void UCombatSystemComponent::GetVelocityVariables()
{
	FVector PlayerVelocity = playerCharacter->GetVelocity();

	auto CharacterMovement = playerCharacter->GetCharacterMovement();
	float NegatedMaxWalkSpeed = -CharacterMovement->MaxWalkSpeed;

	FVector TargetLagPosition;
	TargetLagPosition.X = PlayerVelocity.Dot(playerCharacter->GetActorRightVector()) / NegatedMaxWalkSpeed;
	TargetLagPosition.Y = PlayerVelocity.Dot(playerCharacter->GetActorForwardVector()) / NegatedMaxWalkSpeed;
	TargetLagPosition.Z = PlayerVelocity.Dot(playerCharacter->GetActorUpVector()) / -CharacterMovement->JumpZVelocity;

	TargetLagPosition = UKismetMathLibrary::ClampVectorSize(TargetLagPosition * 5, 0., 3.5);

	LocationLagPosition = UKismetMathLibrary::VInterpTo(LocationLagPosition, TargetLagPosition, GetWorld()->GetDeltaSeconds(), 13.);
}

void UCombatSystemComponent::TeleportToClosestEnemy(ABaseEnemy* Enemy)
{
	//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Cyan, TEXT("teleport"));

	//playerCharacter->GetCharacterMovement()->StopActiveMovement();

	auto ToPlayer = playerCharacter->GetActorLocation() - Enemy->GetActorLocation();

	//float EnemyCapsuleRadius = Enemy->GetCapsuleComponent()->GetScaledCapsuleRadius();

	PlayerStartForTeleport = playerCharacter->GetActorLocation();
	PlayerDestinationForTeleport = Enemy->GetActorLocation() + ToPlayer.GetSafeNormal() * KatanaTriggerLenSquared * 0.5f; //change to unsafe normal for perfomance?
	PlayerDestinationForTeleport.Z = Enemy->GetActorLocation().Z;

	//check if can safely teleport
	float TraceDepth = Enemy->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.f;
	FVector Start = PlayerDestinationForTeleport;
	//Start.Z = Enemy->GetActorLocation().Z;

	FVector End = Start - (Enemy->GetActorUpVector() * TraceDepth);
	//FVector End = Start - (Enemy->GetActorUpVector() * playerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 1.2f);
	FHitResult OutHit;

	//change to capusle trace?
	bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(), Start, End, 
		TEnumAsByte<ETraceTypeQuery>(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Visibility)), 
		true, TArray<AActor*>(), EDrawDebugTrace::ForDuration, OutHit, true, FColor::Red, FColor::Green, 5.f);

	/*if (bHit)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Emerald, FString::Printf(TEXT("Allowed Trace Depth = %f"), TraceDepth * .4));
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Emerald, FString::Printf(TEXT("Distance = %f"), OutHit.Distance));
	}*/

	if (bHit && OutHit.Distance >= (TraceDepth * 0.f))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, TEXT("Got Ground!"));
		//DrawDebugLine(GetWorld(), Start, Start - (Enemy->GetActorUpVector() * OutHit.Distance), FColor::Cyan, false, 5.f, 0, 1.5);
		//OutHit.Distance

		playerCharacter->GetCharacterMovement()->DisableMovement();
		playerCharacter->GetController()->SetIgnoreLookInput(true);
		playerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//change Z so that it matches the Z of hit object
		PlayerDestinationForTeleport = OutHit.Location;
		PlayerDestinationForTeleport.Z += playerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		PlayerOnTeleportRotation = playerCharacter->GetControlRotation();
		RotationToEnemy = playerCharacter->GetControlRotation();

		auto FakeDestination = PlayerDestinationForTeleport;
		FakeDestination.Z = playerCharacter->GetMesh()->GetBoneLocation("head").Z;

		FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(PlayerStartForTeleport, PlayerDestinationForTeleport); //change to enemies location?

		/*FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(playerCharacter->GetControlRotation().Vector(),
			(Enemy->GetActorLocation() - Start).GetUnsafeNormal());*/

		/*FRotator LookAt = UKismetMathLibrary::NormalizedDeltaRotator(playerCharacter->GetControlRotation(), 
			(Enemy->GetActorLocation() - PlayerDestinationForTeleport).Rotation());*/

		RotationToEnemy.Yaw = LookAt.Yaw;
		//RotationToEnemy.Pitch = -LookAt.Pitch;


		/*GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::Printf(TEXT("Length between = %f"), ToPlayer.Length()));
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::Printf(TEXT("Katana Trigger  = %f"), KatanaTriggerLenSquared));
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::Printf(TEXT("Teleport Trigger  = %f"), TeleportTriggerLength));*/

		float NormalizedTeleportTime = UKismetMathLibrary::MapRangeClamped(ToPlayer.Length(), KatanaTriggerLenSquared, TeleportTriggerLength, 0.1, TotalTeleportTime);
		//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Green, FString::Printf(TEXT("Total Time  = %f"), NormalizedTeleportTime));
		TeleportTimeline.SetPlayRate(1.f / NormalizedTeleportTime);

		bInTeleport = true;
		AnimInstance->Montage_Pause(CurrentAttackMontage);
		//AnimInstance->Montage_SetPlayRate(CurrentAttackMontage, .4);
		TeleportTimeline.PlayFromStart();
	}
}

void UCombatSystemComponent::InitializeCombatSystem(ACharacter* player, TSubclassOf<AKatana> KatanaActor)
{
	playerCharacter = player;

	auto PlayerMesh = playerCharacter->GetMesh();
	CharacterArmsLength = FVector::Distance(PlayerMesh->GetBoneLocation("clavicle_r"), PlayerMesh->GetBoneLocation("hand_r"));
	//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Cyan, FString::Printf(TEXT("Player Arms Length = %f"), CharacterArmsLength));

	//PlayerCameraFOV = playerCharacter->get

	FActorSpawnParameters KatanaSpawnParams;
	KatanaSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	KatanaSpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

	Katana = GetWorld()->SpawnActor<AKatana>(KatanaActor, player->GetTransform(), KatanaSpawnParams);
	Katana->OffsetTraceEndSocket(KatanaBladeTriggerScale);
	
	HitTracer = Katana->HitTracer;

	EAttachmentRule KatanaAttachRules = EAttachmentRule::SnapToTarget;

	Katana->K2_AttachToComponent(player->GetMesh(), "KatanaSocket",
		KatanaAttachRules, KatanaAttachRules, KatanaAttachRules,
		true);

	auto BladeVector = Katana->GetBladeWorldVector();
	KatanaTriggerLenSquared = (BladeVector * KatanaBladeTriggerScale).Length() + CharacterArmsLength;
	TeleportTriggerLength = KatanaTriggerLenSquared * TeleportTriggerScale;	// collider scale relative to katana collider
	//TeleportTriggerLength = BladeVector.Length() * TeleportTriggerScale * 2;	//previous solution: collider scale relative to katana blade
	
	//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Cyan, FString::Printf(TEXT("Teleport trigger length = %f"), TeleportTriggerLength));
 
	PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	PlayerCameraManager->ViewPitchMax = MaxViewPitchValue;
	PlayerCameraManager->ViewPitchMin = MinViewPitchValue;

	AnimInstance = playerCharacter->GetMesh()->GetAnimInstance();

	AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UCombatSystemComponent::PlayMontageNotifyBegin);
	AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &UCombatSystemComponent::PlayMontageNotifyEnd);
	AnimInstance->OnMontageBlendingOut.AddDynamic(this, &UCombatSystemComponent::PlayMontageFinished);

	//bind teleport curve data: location
	FOnTimelineFloat TimelineProgressLocation;
	TimelineProgressLocation.BindUFunction(this, FName("TimelineProgessLocation"));
	TeleportTimeline.AddInterpFloat(LocationCurve, TimelineProgressLocation);

	FOnTimelineFloat TimelineProgressRotation;
	TimelineProgressRotation.BindUFunction(this, FName("TimelineProgessRotation"));
	TeleportTimeline.AddInterpFloat(RotationCurve, TimelineProgressRotation);


	FOnTimelineEvent TimelineFinished;
	TimelineFinished.BindUFunction(this, FName("EnablePlayerVariables"));
	TeleportTimeline.SetTimelineFinishedFunc(TimelineFinished);

	TeleportTimeline.SetLooping(false);
}

void UCombatSystemComponent::Attack()
{
	if (!CheckIfCanAttack())
		return;

	//reset this cock-sucking plugin that barely works
	HitTracer->ClearHitArray();

	//quickly stop perfect parry montage
	AnimInstance->Montage_Stop(0.01, PerfectParryMontage);

	bIsAttacking = true;
	bInterputedByItself = false;
	bShouldIgnoreTeleport = false;

	auto MontageToPlay = DetermineNextMontage();
	float AttackMontageStartPercent = .21f;
	AnimInstance->Montage_Play(MontageToPlay, AttackSpeedMultiplier, EMontagePlayReturnType::MontageLength, AttackMontageStartPercent);

	//start timer for auto aim
	GetWorld()->GetTimerManager().SetTimer(EnemiesTraceTimerHandle, this,
		&UCombatSystemComponent::GetEnemiesInViewportOnAttack, 
		1 / 120.f, true);

	//start timer for teleport
	/*GetWorld()->GetTimerManager().SetTimer(TeleportTraceTimerHandle, this,
		&UCombatSystemComponent::TraceForEnemiesToTeleport,
		1 / 120.f, true);*/
}

void UCombatSystemComponent::GetLeftTransforms(FTransform& KatanaGripWorldTransform, FTransform& LeftHandSocket, FTransform& RightHandSocket)
{
	KatanaGripWorldTransform = Katana->KatanaMesh->GetSocketTransform("LeftHand");

	auto PlayerMesh = playerCharacter->GetMesh();
	LeftHandSocket = PlayerMesh->GetSocketTransform("LeftKatanaReference", ERelativeTransformSpace::RTS_ParentBoneSpace);
	RightHandSocket = PlayerMesh->GetSocketTransform("KatanaSocket", ERelativeTransformSpace::RTS_ParentBoneSpace);
}

void UCombatSystemComponent::PerfectParry()
{
	if (bInTeleport || bInParry)
		return;

	bInParry = true;
	bCanRigUpdate = false;
	bInCombat = true;

	AnimInstance->Montage_Play(PerfectParryMontage, 1, EMontagePlayReturnType::MontageLength);
}

void UCombatSystemComponent::InterruptPerfectParry()
{
	AnimInstance->Montage_Stop(0.1, PerfectParryMontage);
}

void UCombatSystemComponent::PerfectParryResponse(int InTokens = 0)
{
	if(StolenTokens < MaxStolenTokens)
		StolenTokens += InTokens;

	//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::Printf(TEXT("Total Stolen Tokens = %i"), StolenTokens));

	//AnimInstance->Montage_Stop(0.5, PerfectParryMontage);

	auto LaunchVelocity = playerCharacter->GetActorForwardVector() * -800.f;
	playerCharacter->LaunchCharacter(LaunchVelocity, false, false);

	UGameplayStatics::SpawnEmitterAttached(PerfectParryParticles, Katana->KatanaMesh,
		"ParryEffect", FVector::Zero(), FRotator::ZeroRotator, FVector(3.f), EAttachLocation::SnapToTargetIncludingScale);

	PlayerCameraManager->StopAllCameraShakes();
	PlayerCameraManager->PlayWorldCameraShake(GetWorld(),
		ParryCamShake,
		playerCharacter->GetActorLocation(),
		0, 500, 1);
}

void UCombatSystemComponent::PlayMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	if (NotifyName.IsEqual("TraceWindow"))
	{
		bInCombat = true;
		HitTracer->ToggleTraceCheck(true);
		
		PlayerCameraManager->PlayWorldCameraShake(GetWorld(), 
			AttackCamShake,
			playerCharacter->GetActorLocation(), 
			0, 500, 1);

		KatanaPreviousPosition = GetKatanaSocketWorldPosition(KatanaSocketForDirection);
	}
	else if (NotifyName.IsEqual("CRigUpdate"))
	{
		bCanRigUpdate = true;
	}
	else if (NotifyName.IsEqual("CRigReset"))
	{
		bCanRigUpdate = false;
		bInCombat = false;
	} 
	else if (NotifyName.IsEqual("TeleportIgnore"))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Green, TEXT("TeleportIgnore hit!"));
		bShouldIgnoreTeleport = true;
	}
}

void UCombatSystemComponent::PlayMontageNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	if (NotifyName.IsEqual("TraceWindow"))
	{
		bInterputedByItself = true;
		HandleAttackEnd();
	}
}

void UCombatSystemComponent::PlayMontageFinished(UAnimMontage* MontagePlayed, bool bWasInterrupted)
{
	if (MontagePlayed == PerfectParryMontage)
	{
		bInCombat = false;
		bInParry = false;
	} 
	else if (AttackMontages.Contains(MontagePlayed))
	{
		HandleAttackEnd();
	}
}

void UCombatSystemComponent::TimelineProgessLocation(float Value)
{
	auto NewLocation = FMath::Lerp(PlayerStartForTeleport, PlayerDestinationForTeleport, Value);
	playerCharacter->SetActorLocation(NewLocation);

	//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::Printf(TEXT("Timeline value = %f"), TeleportTimeline.GetPlaybackPosition()));

	if (TeleportTimeline.GetPlaybackPosition() >= .3f)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, TEXT("playing again"));
		AnimInstance->Montage_Resume(CurrentAttackMontage);
	}
}

void UCombatSystemComponent::TimelineProgessRotation(float Value)
{
	auto NewRotation = FMath::Lerp(PlayerOnTeleportRotation, RotationToEnemy, Value);
	playerCharacter->GetController()->SetControlRotation(NewRotation);
}

void UCombatSystemComponent::TimelineProgessFOV(float Value)
{

}

void UCombatSystemComponent::EnablePlayerVariables()
{
	//playerCharacter->GetCharacterMovement()->StopMovementImmediately();
	playerCharacter->GetController()->SetIgnoreLookInput(false);
	playerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	playerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	bInTeleport = false;
}

// Called every frame
void UCombatSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TeleportTimeline.TickTimeline(DeltaTime);
	
	if (!bInCombat) //change to not attacking??
	{
		GetLookInputVariables(CurrentCameraRotation);
		GetVelocityVariables();

		HandTotalOffset = HandSwayLookOffset + LocationLagPosition;
	}
}