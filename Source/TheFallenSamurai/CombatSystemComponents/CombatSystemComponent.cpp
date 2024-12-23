// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatSystemComponent.h"

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Kismet/GameplayStatics.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimTypes.h"
#include "AnimNotifies/AnimNotify_PlayMontageNotify.h"

#include "DidItHitActorComponent.h"

#include "TheFallenSamurai/KatanaSource/Katana.h"
#include "TheFallenSamurai/BaseEnemySource/BaseEnemy.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include "GameFramework/WorldSettings.h"

#include "Particles/ParticleSystemComponent.h"

#include "TheFallenSamurai/TheFallenSamuraiCharacter.h"

#include "UObject/Class.h"

#include "ComboSystem.h"

#include "TheFallenSamurai/Slicebles/SlicableActor.h"

//DEBUG
#include "DrawDebugHelpers.h"


#define PRINT(mess, mtime)  GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::Green, TEXT(mess));
#define PRINTC(mess, color)  GEngine->AddOnScreenDebugMessage(-1, 0.33, color, TEXT(mess));
#define PRINT_F(prompt, mess, mtime) GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::White, FString::Printf(TEXT(prompt), mess));
#define PRINTC_F(prompt, mess, mtime, color) GEngine->AddOnScreenDebugMessage(-1, mtime, color, FString::Printf(TEXT(prompt), mess));
#define PRINT_B(prompt, mess) GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Magenta, FString::Printf(TEXT(prompt), mess ? TEXT("TRUE") : TEXT("FALSE")));


// Sets default values for this component's properties
UCombatSystemComponent::UCombatSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UCombatSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	ComboSystem = UComboSystem::GetInstance();
}

bool UCombatSystemComponent::CheckIfCanAttack()
{
	return !bIsAttacking && (bInterputedByItself || CurrentAttackData.AttackMontage != playerCharacter->GetCurrentMontage() || !CurrentAttackData.AttackMontage);
}

const FAttackAnimData& UCombatSystemComponent::DetermineNextAttackData()
{
	static int index = 0;
	return AttackMontages[index++ % AttackMontages.Num()];
}

const FAttackAnimData& UCombatSystemComponent::DetermineNextCounterAttackData()
{
	static int index = 0;
	return CounterAttackMontages[index++ % CounterAttackMontages.Num()];
}

void UCombatSystemComponent::HandleAttackEnd(bool bShouldPerformFinalTraceCheck)
{
	GetWorld()->GetTimerManager().ClearTimer(EnemiesTraceTimerHandle);

	bIsAttacking = false;
	bInCombat = false;

	HitTracer->ToggleTraceCheck(false);

	if (!bShouldPerformFinalTraceCheck)
		return;

	for (auto result : HitTracer->HitArray)
		ProcessHitResult(result);
}

void UCombatSystemComponent::ProcessHitResult(const FHitResult& HitResult)
{
	//slice plane code
	FVector HandVelocity = playerCharacter->GetMesh()->GetBoneLinearVelocity("hand_r");
	FVector PlaneNormal = Katana->GetBladeWorldVector().Cross(HandVelocity);
	PlaneNormal.Normalize();

	FVector TrueImpactPoint = Katana->KatanaMesh->GetSocketLocation("Middle");

	AActor* HitActor = HitResult.GetActor();
	if (auto Enemy = Cast<ABaseEnemy>(HitActor))
	{
		if (!CurrentAttackData.bIsTeleportAttack && Enemy->bOwnsShield)
		{
			auto ToPlayerNormalized = (playerCharacter->GetActorLocation() - HitActor->GetActorLocation()).GetSafeNormal();
			if (CheckIsShieldProtected(ToPlayerNormalized, HitActor->GetActorForwardVector()))
			{
				ProcessHitResponse(ShieldHitImpulse, HitResult.ImpactPoint);
				return;
			}
		}

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), 
			BloodParticles, Enemy->GetMesh()->GetBoneLocation("head"), FRotator(0), BloodScale);

		FCombatHitData KatanaHitResult;
		KatanaHitResult.ActorCauser = playerCharacter;
		KatanaHitResult.ImpactLocation = HitResult.ImpactPoint;
		KatanaHitResult.CutPlaneNormal = PlaneNormal;
		KatanaHitResult.CutVelocity = HandVelocity;
		KatanaHitResult.bSuperAbilityKill = SuperAbilityTargetsLeft >= 0;

		if (!Enemy->HandleHitReaction(KatanaHitResult))
		{
			PlayerCameraManager->StopAllCameraShakes();

			PlayerCameraManager->PlayWorldCameraShake(GetWorld(),
				HitCameraShake,
				playerCharacter->GetActorLocation(),
				1, 500, 1);
		}
	}
	else if (auto SlicableActor = Cast<ASlicableActor>(HitActor))
	{
		SlicableActor->SliceActor(PlaneNormal, HitResult.ImpactPoint);
	}
}

void UCombatSystemComponent::ProcessHitResponse(float ImpulseStrength, const FVector& ImpactPoint)
{
	if (ImpulseStrength)
	{
		auto LaunchVelocity = playerCharacter->GetActorForwardVector() * -ImpulseStrength;
		playerCharacter->LaunchCharacter(LaunchVelocity, false, false);
	}

	AnimInstance->Montage_Stop(OnHitAnimationBlendTime, CurrentAttackData.AttackMontage);

	HandleAttackEnd(false);

	PlayerCameraManager->StopAllCameraShakes();

	PlayerCameraManager->PlayWorldCameraShake(GetWorld(),
		ShieldHitCameraShake,
		playerCharacter->GetActorLocation(),
		0, 500, 1);

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShieldHitParticle, ImpactPoint, FRotator(0.f), FVector(UniformShieldHitParticleSize));
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
		ProcessHitResult(result);
		result.Reset();
	}

	if (bInTeleport)
		return;

	auto CapsuleComponent = playerCharacter->GetCapsuleComponent();

	FVector HalfSize;
	HalfSize.X = 1.f;
	HalfSize.Y = CapsuleComponent->GetScaledCapsuleRadius() * 4.;
	HalfSize.Z = CapsuleComponent->GetScaledCapsuleHalfHeight() * 1.5;

	FVector Start = playerCharacter->GetActorLocation();
	FVector End = Start + playerCharacter->GetControlRotation().Vector() * TeleportTriggerLength;


	FRotator BoxRotation = playerCharacter->GetControlRotation();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjToTrace;
	ObjToTrace.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	TArray<AActor*> Ignore;
	Ignore.Add(playerCharacter);

	TArray<FHitResult> HitResults;

	UKismetSystemLibrary::BoxTraceMultiForObjects(GetWorld(), Start, End, HalfSize, BoxRotation,
		ObjToTrace, true, Ignore, EDrawDebugTrace::None, HitResults, true, FColor::Red, FColor::Green, 1.5f);

	float MinDistance = TeleportTriggerLength + 100;
	ABaseEnemy* Closest = nullptr;
	for (auto HitResult : HitResults)
	{
		auto Enemy = Cast<ABaseEnemy>(HitResult.GetActor());
		if (!Enemy)
			continue;

		float CurrentDistance = Enemy->GetDistanceTo(playerCharacter);

		if (CurrentDistance < MinDistance)
		{
			MinDistance = CurrentDistance;
			Closest = Enemy;
		}
	}

	if (Closest)
	{
		FVector NextTargetPointOffset = GetAutoAimOffset(playerCharacter->GetMesh()->GetBoneLocation("head"), Closest->GetActorLocation(), 
			playerCharacter->GetActorForwardVector(), playerCharacter->GetActorUpVector());

		TargetPointOffset = UKismetMathLibrary::VInterpTo(TargetPointOffset, NextTargetPointOffset, GetWorld()->GetDeltaSeconds(), 25.f);

		if (!bShouldIgnoreTeleport && MinDistance > KatanaTriggerLenSquared)
		{
			FValidationRules ValidationRules{};
			ValidationRules.DrawDebugTrace = EDrawDebugTrace::ForDuration;


			bool bIsTargetValid = ValidateTeleportTarget(Closest, ValidationRules);
			if (bIsTargetValid)
				TeleportToEnemy(Closest->GetDistanceTo(playerCharacter));
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

bool UCombatSystemComponent::CheckIsTeleportTargetObscured(ABaseEnemy* Enemy)
{
	FVector EyeStart = playerCharacter->GetMesh()->GetBoneLocation("head");
	FVector EyeEnd = Enemy->GetActorLocation();
	FHitResult EyeOutHit;

	FCollisionQueryParams CollisionParams;
	CollisionParams.bTraceComplex = true;
	CollisionParams.AddIgnoredActor(playerCharacter);
	CollisionParams.AddIgnoredActor(Katana);
	CollisionParams.AddIgnoredActor(Enemy);

	bool bEyeToCenterHit = GetWorld()->LineTraceSingleByChannel(EyeOutHit, EyeStart, EyeEnd, ECollisionChannel::ECC_Visibility, CollisionParams);

	EyeEnd.Z += Enemy->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	bool bEyeToEyeHit = GetWorld()->LineTraceSingleByChannel(EyeOutHit, EyeStart, EyeEnd, ECollisionChannel::ECC_Visibility, CollisionParams);

	return bEyeToCenterHit && bEyeToEyeHit;
}

bool UCombatSystemComponent::CheckIsShieldProtected(const FVector& ToPlayerNormalized, const FVector& EnemyForward)
{
	float dot = ToPlayerNormalized.Dot(EnemyForward);
	static const float cos = FMath::Cos(FMath::DegreesToRadians(ShieldIgnoreAngle));

	return dot > cos;
}

bool UCombatSystemComponent::ValidateTeleportTarget(ABaseEnemy* Enemy, const FValidationRules& ValidationRules)
{
	auto ToPlayer = playerCharacter->GetActorLocation() - Enemy->GetActorLocation();
	auto ToPlayerNormalized = ToPlayer.GetSafeNormal(); //change to unsafe normal for perfomance?

	if (!ValidationRules.bShouldIgnoreShields)
	{
		if (Enemy->bOwnsShield)
		{
			if (CheckIsShieldProtected(ToPlayerNormalized, Enemy->GetActorForwardVector()))
			{
				if (ValidationRules.bUseDebugPrint)
					PRINT("CANT TELEPORT: [FOUND SHIELD]", 2);
				return false;
			}
		}
	}

	if (CheckIsTeleportTargetObscured(Enemy))
	{
		if (ValidationRules.bUseDebugPrint)
			PRINT("CANT TELEPORT: [EYE TRACE]", 4);
		return false;
	}

	PlayerStartForTeleport = playerCharacter->GetActorLocation();

	FVector EvaluatedDestination;

	auto playerCapsule = playerCharacter->GetCapsuleComponent();
	/*float BlockCapsuleRadius = playerCapsule->GetScaledCapsuleRadius() * .8f;
	float BlockCapsuleHalfHeight = playerCapsule->GetScaledCapsuleHalfHeight() - 3.f;*/
	float BlockCapsuleRadius = playerCapsule->GetScaledCapsuleRadius();
	float BlockCapsuleHalfHeight = playerCapsule->GetScaledCapsuleHalfHeight();
	float TraceDepth = playerCapsule->GetScaledCapsuleHalfHeight() * 2.5f;

	float TeleportOffset = KatanaTriggerLenSquared * 0.55f;

	float TeleportTime = UKismetMathLibrary::MapRangeClamped(ToPlayer.Length(), KatanaTriggerLenSquared,
		TeleportTriggerLength, MinTotalTeleportTime, MaxTotalTeleportTime);

	FVector EnemyLocationOverTime = Enemy->GetActorLocation() + Enemy->GetVelocity() * TeleportTime;
	FVector TeleportOffsetVector = ToPlayerNormalized * TeleportOffset;

	bool bCanTeleport = false;

	bCanTeleport = PerformTeleportCheck(Enemy, EnemyLocationOverTime, TeleportOffsetVector, TraceDepth,
		BlockCapsuleRadius, BlockCapsuleHalfHeight, ValidationRules);

	if (!ValidationRules.bUseLazyCheck && !bCanTeleport)
	{
		float Side = BlockCapsuleRadius * 2.f;
		float TwoR = TeleportOffset * 2.f;

		int N = FMath::DivideAndRoundNearest(180.f, FMath::RadiansToDegrees(FMath::FastAsin(Side / TwoR)));

		N *= ValidationRules.ChecksSampleScale;

		float InnerAngle = 360.f / N;
		float LeftRotation = InnerAngle;
		float RightRotation = -InnerAngle;

		for (int Checks = 1; (Checks <= N) && !bCanTeleport; Checks += 2)
		{
			auto LeftDirection = TeleportOffsetVector.RotateAngleAxis(LeftRotation, FVector(0, 0, 1));
			auto RightDirection = TeleportOffsetVector.RotateAngleAxis(RightRotation, FVector(0, 0, 1));
			LeftRotation += InnerAngle;
			RightRotation -= InnerAngle;

			bCanTeleport = PerformTeleportCheck(Enemy, EnemyLocationOverTime, LeftDirection, TraceDepth,
				BlockCapsuleRadius, BlockCapsuleHalfHeight, ValidationRules);
			
			if(!bCanTeleport)
				bCanTeleport = PerformTeleportCheck(Enemy, EnemyLocationOverTime, RightDirection, TraceDepth,
					BlockCapsuleRadius, BlockCapsuleHalfHeight, ValidationRules);
		}
	}

	return bCanTeleport;
}

bool UCombatSystemComponent::PerformTeleportCheck(ABaseEnemy* Enemy, const FVector& EnemyLocationOverTime, const FVector& Direction, float TraceDepth,
	float BlockCapsuleRadius, float BlockCapsuleHalfHeight, const FValidationRules& ValidationRules)
{
	FVector EvaluatedDestination = EnemyLocationOverTime + Direction;
	EvaluatedDestination.Z = EnemyLocationOverTime.Z;

	FVector GroundStart = EvaluatedDestination;

	FVector GroundEnd = GroundStart - (Enemy->GetActorUpVector() * TraceDepth);
	FHitResult GroundHit;

	/*FCollisionQueryParams CollisionParams;
	CollisionParams.bTraceComplex = true;*/

	/*bool bHasGround = UKismetSystemLibrary::LineTraceSingle(GetWorld(), GroundStart, GroundEnd, ETraceTypeQuery::TraceTypeQuery1, 
		true, {}, ValidationRules.DrawDebugTrace, GroundHit, true);*/

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjToTrace;
	ObjToTrace.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
	//ObjToTrace.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
	ObjToTrace.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	//FVector HalfHeightVector = playerCharacter->GetActorUpVector() * BlockCapsuleHalfHeight;
	/*FVector CapsuleStart = EvaluatedDestination + HalfHeightVector;
	FVector CapsuleEnd = EvaluatedDestination - HalfHeightVector;*/

	bool bHasGround = UKismetSystemLibrary::CapsuleTraceSingleForObjects(GetWorld(), GroundStart, GroundEnd,
		BlockCapsuleRadius, BlockCapsuleRadius,
		ObjToTrace, true, { playerCharacter, Enemy }, ValidationRules.DrawDebugTrace, GroundHit, true, FColor::Emerald, FColor::Green);

	DrawDebugBox(GetWorld(), GroundHit.ImpactPoint, FVector(5.f), FColor::Magenta, true, 10);
	//PRINT_B("penetrating hit %s", GroundHit.bStartPenetrating);

	if (!bHasGround)
	{
		if (ValidationRules.bUseDebugPrint)
			PRINT("CANT TELEPORT: [GROUND CHECK]", 4);
		return false;
	}

	PRINT_F("hit actor by ground check %s", *UKismetSystemLibrary::GetDisplayName(GroundHit.GetComponent()), 0.033f);

	//change Z so that player has perfect teleport position and collision enabling won't cause chaos
	PlayerDestinationForTeleport = GroundHit.ImpactPoint;
	PlayerDestinationForTeleport.Z += playerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	FHitResult CapsuleSpaceHit;

	FVector CapsuleCheckLocation = PlayerDestinationForTeleport + FVector(0.f, 0.f, BlockCapsuleHalfHeight / 2.f);
	bool bTeleportBlock = UKismetSystemLibrary::CapsuleTraceSingleForObjects(GetWorld(), CapsuleCheckLocation, CapsuleCheckLocation,
		BlockCapsuleRadius, BlockCapsuleHalfHeight / 2.f,
		ObjToTrace, true, { playerCharacter }, ValidationRules.DrawDebugTrace, CapsuleSpaceHit, true, FColor::Emerald, FColor::Green);

	if (bTeleportBlock)
	{
		if (ValidationRules.bUseDebugPrint)
		{
			PRINT("CANT TELEPORT: [CAPSULE CHECK] - actor : %s, component: %s", 4);
			PRINT_F("actor : %s", *UKismetSystemLibrary::GetDisplayName(CapsuleSpaceHit.GetActor()), 4);
			PRINT_F("component : %s", *UKismetSystemLibrary::GetDisplayName(CapsuleSpaceHit.GetComponent()), 4);
		}
		return false;
	}

	////change Z so that player has perfect teleport position and collision enabling won't cause chaos
	//PlayerDestinationForTeleport = GroundHit.Location;
	//PlayerDestinationForTeleport.Z += playerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	//TArray<TEnumAsByte<EObjectTypeQuery>> ObjToTrace;
	//ObjToTrace.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
	////ObjToTrace.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
	//ObjToTrace.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));


	//FVector CapsuleCheckLocation = PlayerDestinationForTeleport + FVector(0.f, 0.f, BlockCapsuleHalfHeight / 2.f);
	//bool bTeleportBlock = UKismetSystemLibrary::CapsuleTraceSingleForObjects(GetWorld(), CapsuleCheckLocation, CapsuleCheckLocation,
	//	BlockCapsuleRadius, BlockCapsuleHalfHeight / 2.f,
	//	ObjToTrace, true, { playerCharacter }, ValidationRules.DrawDebugTrace, CapsuleSpaceHit, true, FColor::Emerald, FColor::Green);

	//DrawDebugBox(GetWorld(), CapsuleSpaceHit.ImpactPoint, FVector(5.f), FColor::Magenta, true, 10);

	/*bool bTeleportBlock = UKismetSystemLibrary::CapsuleTraceSingleForObjects(GetWorld(), PlayerDestinationForTeleport, PlayerDestinationForTeleport,
		BlockCapsuleRadius, BlockCapsuleHalfHeight,
		ObjToTrace, true, { playerCharacter }, ValidationRules.DrawDebugTrace, CapsuleSpaceHit, true, FColor::Emerald, FColor::Green);*/

	//DrawDebugCapsule(GetWorld(), PlayerDestinationForTeleport, BlockCapsuleHalfHeight, BlockCapsuleRadius, FQuat::Identity, FColor::Magenta, true);	


	PlayerOnTeleportRotation = playerCharacter->GetControlRotation();
	RotationToEnemy = playerCharacter->GetControlRotation();

	FVector LookAtEnemyLocation = EnemyLocationOverTime;
	LookAtEnemyLocation.Z -= Enemy->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * .3f; // so that player looks a bit down

	FRotator FaceEnemyRotation = (LookAtEnemyLocation - PlayerDestinationForTeleport).Rotation();
	FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(FaceEnemyRotation, PlayerOnTeleportRotation);

	if (!ValidationRules.bUsePitch)
		Delta.Pitch = 0;

	RotationToEnemy += Delta;

	auto FeetToHead = playerCharacter->GetMesh()->GetBoneLocation("head") - playerCharacter->GetMesh()->GetBoneLocation("root");
	auto CombatPoint = GroundHit.Location + FeetToHead;

	TargetPointOffset = GetAutoAimOffset(PlayerDestinationForTeleport, EnemyLocationOverTime,
		RotationToEnemy.Vector(), playerCharacter->GetActorUpVector());
	CombatPoint += RotationToEnemy.Vector() * CharacterArmsLength + TargetPointOffset;

	FVector KatanaStart = CombatPoint;
	FVector KatanaEnd = KatanaStart + RotationToEnemy.Vector() * KatanaTriggerLenSquared;

	FVector EnemyBottom = EnemyLocationOverTime - Enemy->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * Enemy->GetActorUpVector();
	FVector EnemyTop = EnemyLocationOverTime + Enemy->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * Enemy->GetActorUpVector();

	if (ValidationRules.DrawDebugTrace != EDrawDebugTrace::None)
	{
		DrawDebugLine(GetWorld(), CombatPoint, CombatPoint, FColor::Cyan, true, 1, 0, 5);
		DrawDebugLine(GetWorld(), CombatPoint, KatanaEnd, FColor::Blue, true, 1, 0, 2);

		DrawDebugLine(GetWorld(), EnemyBottom, EnemyBottom, FColor::Orange, true, 1, 0, 10);
		DrawDebugLine(GetWorld(), EnemyTop, EnemyTop, FColor::Green, true, 1, 0, 10);
	}

	if (!UKismetMathLibrary::InRange_FloatFloat(KatanaStart.Z, EnemyBottom.Z, EnemyTop.Z) && !UKismetMathLibrary::InRange_FloatFloat(KatanaEnd.Z, EnemyBottom.Z, EnemyTop.Z))
	{
		if (ValidationRules.bUseDebugPrint)
			PRINT("CANT TELEPORT: Katana Blade wont cut the enemy", 4);
		return false;
	}

	return true;
}

void UCombatSystemComponent::TeleportToEnemy(float TeleportDistance)
{
	bInTeleport = true;

	CurrentAttackData.bIsTeleportAttack = true;

	PlayerCameraFOV = PlayerCameraManager->GetFOVAngle();

	float NormalizedTeleportTime = UKismetMathLibrary::MapRangeClamped(TeleportDistance, KatanaTriggerLenSquared,
			TeleportTriggerLength, MinTotalTeleportTime, MaxTotalTeleportTime);

	TeleportTimeline.SetPlayRate(1.f / NormalizedTeleportTime);

	auto CurrentAttackMontage = CurrentAttackData.AttackMontage;
	float TimeToPerfectAttack = CurrentAttackData.PerfectAttackTime - AnimInstance->Montage_GetPosition(CurrentAttackMontage);
	float AcctualPlayRate = TimeToPerfectAttack / NormalizedTeleportTime;

	AnimInstance->Montage_SetPlayRate(CurrentAttackMontage, AcctualPlayRate);

	playerCharacter->GetCharacterMovement()->DisableMovement();
	playerCharacter->GetController()->SetIgnoreLookInput(true);
	playerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	OnIFramesChanged.Broadcast(true);

	TeleportTimeline.PlayFromStart();
}

float UCombatSystemComponent::GetNotifyTimeInMontage(UAnimMontage* Montage, FName NotifyName)
{
	auto NotifyEvent = Montage->Notifies.FindByPredicate([&](const FAnimNotifyEvent& CurrentEvent) -> bool {
		return CurrentEvent.NotifyName == NotifyName;
		});

	if (NotifyEvent)
		return NotifyEvent->GetTriggerTime();
	
	return 0;
}

void UCombatSystemComponent::ExecuteSuperAbility()
{
	FVector Start = playerCharacter->GetActorLocation();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjToTrace;
	ObjToTrace.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	TArray<AActor*> Ignore;
	Ignore.Add(playerCharacter);

	TArray<FHitResult> HitResults;
	UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), Start, Start, MaxJumpRadius, ObjToTrace,
		true, Ignore, EDrawDebugTrace::None, HitResults, true);

	float MaxDot = -1;
	ABaseEnemy* Target = nullptr;

	int ObscuredCounter = HitResults.Num();

	for (auto HitResult : HitResults)
	{
		auto Enemy = Cast<ABaseEnemy>(HitResult.GetActor());
		if (!Enemy)
			continue;

		bool bIsTargetObscured = CheckIsTeleportTargetObscured(Enemy);

		if (bIsTargetObscured)
		{
			ObscuredCounter--;
			continue;
		}
		else
		{
			Enemy->GetMesh()->SetCustomDepthStencilValue(3);
			PostProcessSA_Targets.Add(Enemy);
		}


		FVector ToEnemy = Enemy->GetActorLocation() - playerCharacter->GetActorLocation();

		float dot = playerCharacter->GetControlRotation().Vector().Dot(ToEnemy.GetSafeNormal());

		if (dot >= .97f)
		{
			if (dot > MaxDot)
			{
				MaxDot = dot;
				Target = Enemy;
			}
		}
	}

	if (!ObscuredCounter)
	{
		OnSuperAbilityCalled.Broadcast(false, "No enemies nearby");

		CancelSuperAbility();
		return;
	}

	if (SA_State != SuperAbilityState::WAITING)
		OnSuperAbilityCalled.Broadcast(true, "");

	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), SuperAbilitySlowMo);
	SA_State = SuperAbilityState::WAITING;

	/*if (Target)
	{
		PRINTC_F("Target = %s", *UKismetSystemLibrary::GetDisplayName(Target), 0.33f, FColor::Cyan);
	}
	else
		PRINTC("Target = NULLPTR", FColor::Cyan);*/

	if (Target)
	{
		FValidationRules ValidationRules;
		ValidationRules.bUseLazyCheck = false;
		ValidationRules.ChecksSampleScale = 2;
		ValidationRules.bShouldIgnoreShields = true;

		ValidationRules.DrawDebugTrace = EDrawDebugTrace::ForDuration;
		//ValidationRules.bUseDebugPrint = true;


		bool bIsValidTarget = ValidateTeleportTarget(Target, ValidationRules);
		if (SuperAbilityTarget != Target)
		{
			if (SuperAbilityTarget)
				SuperAbilityTarget->SetEnableTargetWidget(false);

			if (bIsValidTarget)
			{
				Target->SetEnableTargetWidget(true);
				SuperAbilityTarget = Target;
				OnSuperAbilityTargetAcquired.Broadcast(true);
			}
			else
				SuperAbilityTarget = nullptr;
		}
	}
	else if (SuperAbilityTarget)
	{
		SuperAbilityTarget->SetEnableTargetWidget(false);
		OnSuperAbilityTargetAcquired.Broadcast(false);
		SuperAbilityTarget = nullptr;
	}
}

void UCombatSystemComponent::SwingKatana()
{
	HitTracer->ClearHitArray();

	//quickly stop perfect parry montage
	AnimInstance->Montage_Stop(0.01, PerfectParryMontage);

	SpeedUpSlowMoTimeline();

	bIsAttacking = true;
	bInterputedByItself = false;
	bShouldIgnoreTeleport = false;

	float AttackMontageStartPercent = .21f;
	AnimInstance->Montage_Play(NextAttackData.AttackMontage, AttackSpeedMultiplier, EMontagePlayReturnType::MontageLength, AttackMontageStartPercent);

	CurrentAttackData = NextAttackData;
	NextAttackData = DetermineNextAttackData();

	//start timer for auto aim
	GetWorld()->GetTimerManager().SetTimer(EnemiesTraceTimerHandle, this,
		&UCombatSystemComponent::GetEnemiesInViewportOnAttack,
		1 / 60.f, true);
}

void UCombatSystemComponent::ClearAffectedByPostProcess()
{
	for (auto& enemy : PostProcessSA_Targets)
		enemy->GetMesh()->SetCustomDepthStencilValue(enemy->bIsGettingHit ? 1 : 2);
	PostProcessSA_Targets.Empty();
}

FVector UCombatSystemComponent::GetAutoAimOffset(const FVector& PlayerLocation, const FVector& EnemyLocation, const FVector& PlayerForwardVector, const FVector& PlayerUpVector)
{
	auto ToEnemy = EnemyLocation - PlayerLocation;

	FVector PlayerRightVector = PlayerUpVector.Cross(PlayerForwardVector);


	float MaxAbsoluteYOffset = 45.f;
	float TargetPointYOffset = FMath::Clamp(ToEnemy.Dot(PlayerRightVector),
		-MaxAbsoluteYOffset, MaxAbsoluteYOffset);

	float TargetPointZOffset = FMath::Clamp(ToEnemy.Dot(PlayerUpVector), -17, 7);

	return FVector(0.f, TargetPointYOffset, TargetPointZOffset);
}

void UCombatSystemComponent::InitializeCombatSystem(ATheFallenSamuraiCharacter* player, TSubclassOf<AKatana> KatanaActor)
{
	playerCharacter = player;

	auto PlayerMesh = playerCharacter->GetMesh();
	CharacterArmsLength = FVector::Distance(PlayerMesh->GetBoneLocation("clavicle_r"), PlayerMesh->GetBoneLocation("hand_r"));

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
	 
	PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	PlayerCameraManager->ViewPitchMax = MaxViewPitchValue;
	PlayerCameraManager->ViewPitchMin = MinViewPitchValue;


	AnimInstance = playerCharacter->GetMesh()->GetAnimInstance();

	AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UCombatSystemComponent::PlayMontageNotifyBegin);
	AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &UCombatSystemComponent::PlayMontageNotifyEnd);
	AnimInstance->OnMontageBlendingOut.AddDynamic(this, &UCombatSystemComponent::PlayMontageFinished);

	for (auto& AttackMontageData : AttackMontages)
	{
		AttackMontageData.PerfectAttackTime = GetNotifyTimeInMontage(AttackMontageData.AttackMontage, "AN_PerfectAttack");

		if (AttackMontageData.PerfectForCounter)
			CounterAttackMontages.Add(AttackMontageData);
	}

	NextAttackData = DetermineNextAttackData();

	//bind teleport curve data
	FOnTimelineFloat TimelineProgressLocation;
	TimelineProgressLocation.BindUFunction(this, FName("TimelineProgessLocation"));
	TeleportTimeline.AddInterpFloat(LocationCurve, TimelineProgressLocation);

	FOnTimelineFloat TimelineProgressRotation;
	TimelineProgressRotation.BindUFunction(this, FName("TimelineProgessRotation"));
	TeleportTimeline.AddInterpFloat(RotationCurve, TimelineProgressRotation);

	FOnTimelineFloat TimelineProgressFOV;
	TimelineProgressFOV.BindUFunction(this, FName("TimelineProgessFOV"));
	TeleportTimeline.AddInterpFloat(FOVCurve, TimelineProgressFOV);


	FOnTimelineEvent TeleportTimelineFinished;
	TeleportTimelineFinished.BindUFunction(this, FName("TeleportTimelineFinish"));
	TeleportTimeline.SetTimelineFinishedFunc(TeleportTimelineFinished);

	TeleportTimeline.SetLooping(false);

	//bind slow-mo after parry curve data
	FOnTimelineFloat TimelineProgressSlowMo;
	TimelineProgressSlowMo.BindUFunction(this, FName("TimelineProgessSlowMo"));
	ParrySlowMoTimeline.AddInterpFloat(SlowMoCurve, TimelineProgressSlowMo);

	FOnTimelineEvent SlowMoTimelineFinished;
	SlowMoTimelineFinished.BindUFunction(this, FName("SlowMoTimelineFinish"));
	ParrySlowMoTimeline.SetTimelineFinishedFunc(SlowMoTimelineFinished);

	ParrySlowMoTimeline.SetLooping(false);
}

void UCombatSystemComponent::Attack()
{
	switch (SA_State)
	{
		case SuperAbilityState::NONE:
		{
			if (CheckIfCanAttack())
				SwingKatana();
		} break;
		case SuperAbilityState::WAITING:
		{
			if (SuperAbilityTarget)
			{
				GetWorld()->GetTimerManager().ClearTimer(SuperAbilityTimerHandle);

				UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);

				SuperAbilityTarget->SetEnableTargetWidget(false);
				OnSuperAbilityTargetAcquired.Broadcast(false);

				SwingKatana();

				SA_State = SuperAbilityState::TELEPORTING;
				TeleportToEnemy(SuperAbilityTarget->GetDistanceTo(playerCharacter));

				if (ComboSystem->AbilityComboPoints >= ComboSystem->SuperAbilityCost)
				{
					ComboSystem->AbilityComboPoints -= ComboSystem->SuperAbilityCost;
				}

				OnEnemiesLeftChanged.Broadcast(SuperAbilityTargetsLeft);
				SuperAbilityTargetsLeft--;
			}
		} break;
	}
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
	if (bInTeleport || bInParry || IsSuperAbilityActive())
		return;

	bInParry = true;
	bCanRigUpdate = false;
	bInCombat = true;

	SpeedUpSlowMoTimeline();

	AnimInstance->Montage_Play(PerfectParryMontage, PerfectParryMontageSpeed, EMontagePlayReturnType::MontageLength);
}

void UCombatSystemComponent::PerfectParryResponse(bool bEnableSlowMo)
{
	if (bEnableSlowMo)
	{
		bShouldSpeedUpSlowMoTimeline = false;
		ParrySlowMoTimeline.PlayFromStart();
	}

	AnimInstance->Montage_Play(ParryImpactMontage, ParryImpactMontageSpeed);

	NextAttackData = DetermineNextCounterAttackData();

	auto LaunchVelocity = playerCharacter->GetActorForwardVector() * -250.f;
	playerCharacter->LaunchCharacter(LaunchVelocity, false, false);

	PlayerCameraManager->StopAllCameraShakes();
	PlayerCameraManager->PlayWorldCameraShake(GetWorld(),
		ParryCameraShake,
		playerCharacter->GetActorLocation(),
		0, 500, 1);
}

void UCombatSystemComponent::SuperAbility()
{
	if (SA_State == SuperAbilityState::WAITING)
	{
		CancelSuperAbility();
		return;
	}
	else if (SA_State != SuperAbilityState::NONE)
		return;

	PRINT("NO CHECK FOR COMBO POINTS IN SUPERABILITY CODE", 4);

	/*if (ComboSystem->AbilityComboPoints < ComboSystem->SuperAbilityCost)
	{
		OnSuperAbilityCalled.Broadcast(false, "Not enough Combo Points");
		return;
	}*/

	SuperAbilityTargetsLeft = SuperAbilityTargetLimit;

	GetWorld()->GetTimerManager().SetTimer(SuperAbilityTimerHandle, this, &UCombatSystemComponent::ExecuteSuperAbility, 1 / 60.f, true);
}

void UCombatSystemComponent::CancelSuperAbility()
{
	SuperAbilityTargetsLeft = -1;

	ClearAffectedByPostProcess();

	GetWorld()->GetTimerManager().ClearTimer(SuperAbilityTimerHandle);

	if (SuperAbilityTarget)
	{
		SuperAbilityTarget->SetEnableTargetWidget(false);
		SuperAbilityTarget = nullptr;
	}

	SA_State = SuperAbilityState::NONE;
	OnSuperAbilityCancelled.Broadcast();
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
	playerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

bool UCombatSystemComponent::IsSuperAbilityActive()
{
	return SA_State != SuperAbilityState::NONE;
}

float UCombatSystemComponent::GetLookRate()
{
	return SA_State == SuperAbilityState::WAITING ? LookRateScale : 1.f;
}

void UCombatSystemComponent::SpeedUpSlowMoTimeline(float SpeedUpValue)
{
	bShouldSpeedUpSlowMoTimeline = true;
	ParrySlowMoTimeline.SetPlayRate(SpeedUpValue);
}

void UCombatSystemComponent::PlayMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	if (NotifyName.IsEqual("TraceWindow"))
	{
		bInCombat = true;
		HitTracer->ToggleTraceCheck(true);
		
		PlayerCameraManager->PlayWorldCameraShake(GetWorld(), 
			CurrentAttackData.AttackShake,
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
		bShouldIgnoreTeleport = true;
	}
	else if (NotifyName.IsEqual("SuccessfulParryEffect"))
	{
		UGameplayStatics::SpawnEmitterAttached(PerfectParrySparks, Katana->KatanaMesh,
			"ParryEffect", FVector::Zero(), FRotator::ZeroRotator, PerfectParrySparksSize, EAttachLocation::SnapToTargetIncludingScale);
		
		auto SpawnedShockwave = UGameplayStatics::SpawnEmitterAttached(PerfectParryShockwave, Katana->KatanaMesh,
			"ParryEffect", FVector::Zero(), FRotator::ZeroRotator, PerfectParryShockwaveSize, EAttachLocation::SnapToTargetIncludingScale);

		//so that shockwave is not affected by slow mo after parry
		SpawnedShockwave->CustomTimeDilation = 1.f;
	}
}

void UCombatSystemComponent::PlayMontageNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	if (NotifyName.IsEqual("TraceWindow"))
	{
		bInterputedByItself = true;
		HandleAttackEnd();
		OnIFramesChanged.Broadcast(false);

		if (SA_State == SuperAbilityState::TELEPORTING)
		{
			if (SuperAbilityTargetsLeft <= 0)
			{
				CancelSuperAbility();
				return;
			}

			SuperAbilityTarget = nullptr;
			ClearAffectedByPostProcess();
			GetWorld()->GetTimerManager().SetTimer(SuperAbilityTimerHandle, this, &UCombatSystemComponent::ExecuteSuperAbility, 1 / 120.f, true);
		}
	}
}

void UCombatSystemComponent::PlayMontageFinished(UAnimMontage* MontagePlayed, bool bWasInterrupted)
{
	if (MontagePlayed == PerfectParryMontage)
	{
		if(!bWasInterrupted)
			bInCombat = false;
		bInParry = false;
	} 
	else if (CurrentAttackData.AttackMontage == MontagePlayed)
	{
		HandleAttackEnd();
	}
	else if (MontagePlayed == ParryImpactMontage)
	{
		bInCombat = false;
	}
}

void UCombatSystemComponent::TimelineProgessLocation(float Value)
{
	auto NewLocation = FMath::Lerp(PlayerStartForTeleport, PlayerDestinationForTeleport, Value);
	playerCharacter->SetActorLocation(NewLocation);
}

void UCombatSystemComponent::TimelineProgessRotation(float Value)
{
	auto NewRotation = FMath::Lerp(PlayerOnTeleportRotation, RotationToEnemy, Value);
	playerCharacter->GetController()->SetControlRotation(NewRotation);
}

void UCombatSystemComponent::TimelineProgessFOV(float Value)
{
	float NewFOV = FMath::Lerp(PlayerCameraFOV, TeleportFOVChange, Value);
	PlayerCameraManager->SetFOV(NewFOV);
}

void UCombatSystemComponent::TimelineProgessSlowMo(float Value)
{
	float SlowMoValue = FMath::Lerp(1.f, MinTimeDilation, Value);

	if(!bShouldSpeedUpSlowMoTimeline)
		ParrySlowMoTimeline.SetPlayRate(1.f / SlowMoValue);
	
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), SlowMoValue);
}

void UCombatSystemComponent::TeleportTimelineFinish()
{
	playerCharacter->GetController()->SetIgnoreLookInput(false);
	playerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	playerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	bInTeleport = false;

	if (CurrentAttackData.bIsTeleportAttack)
		AnimInstance->Montage_SetPlayRate(CurrentAttackData.AttackMontage, AttackSpeedMultiplier);
}

void UCombatSystemComponent::SlowMoTimelineFinish()
{
	bShouldSpeedUpSlowMoTimeline = false;
}

// Called every frame
void UCombatSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TeleportTimeline.TickTimeline(DeltaTime);
	ParrySlowMoTimeline.TickTimeline(DeltaTime);
	
	if (!bInCombat)
	{
		GetLookInputVariables(CurrentCameraRotation);
		GetVelocityVariables();

		HandTotalOffset = HandSwayLookOffset + LocationLagPosition;
	}

	//SUPER ABILITY DEBUG PRINT
	/*PRINT_F("Super Ability State = %s", *UEnum::GetValueAsString(SA_State), 0);

	if (SuperAbilityTarget)
	{
		PRINT_F("Super Ability Target = %s", *UKismetSystemLibrary::GetDisplayName(SuperAbilityTarget), 0);
	}
	else
		PRINT("Super Ability Target = NULLPTR", 0);*/
}

void UCombatSystemComponent::OnComboPointsChanged(int32 NewComboPoints)
{
	CurrentComboPoints += NewComboPoints;
}