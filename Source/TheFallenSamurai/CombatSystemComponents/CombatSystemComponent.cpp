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

//#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

#include "UObject/Class.h"

//DEBUG
#include "DrawDebugHelpers.h"

#define PRINT(mess, mtime)  GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::Green, TEXT(mess));
#define PRINTC(mess, color)  GEngine->AddOnScreenDebugMessage(-1, 3, color, TEXT(mess));
#define PRINT_F(prompt, mess, mtime) GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::Green, FString::Printf(TEXT(prompt), mess));
#define PRINT_B(prompt, mess) GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, FString::Printf(TEXT(prompt), mess ? TEXT("TRUE") : TEXT("FALSE")));


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
	//return !bIsAttacking && (bInterputedByItself || !AttackMontages.Contains(playerCharacter->GetCurrentMontage()));
	return !bIsAttacking && (bInterputedByItself || CurrentAttackData.AttackMontage != playerCharacter->GetCurrentMontage() || !CurrentAttackData.AttackMontage);
}

const FAttackAnimData& UCombatSystemComponent::DetermineNextAttackData()
{
	/*auto montage = AttackMontages[FMath::RandRange(0, AttackMontages.Num() - 1)];
	return montage;*/

	static int index = 0;
	//CurrentAttackData = AttackMontages[index++ % AttackMontages.Num()];
	return AttackMontages[index++ % AttackMontages.Num()];
}

const FAttackAnimData& UCombatSystemComponent::DetermineNextCounterAttackData()
{
	static int index = 0;
	return CounterAttackMontages[index++ % CounterAttackMontages.Num()];
}

void UCombatSystemComponent::HandleAttackEnd()
{
	//TargetPointOffset = FVector::Zero();

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

	if (bInTeleport)
		return;

	auto CapsuleComponent = playerCharacter->GetCapsuleComponent();

	FVector HalfSize;
	HalfSize.X = 1.f;
	HalfSize.Y = CapsuleComponent->GetScaledCapsuleRadius() * 4.;
	HalfSize.Z = CapsuleComponent->GetScaledCapsuleHalfHeight() * 1.5;

	FVector Start = playerCharacter->GetActorLocation();
	FVector End = Start + playerCharacter->GetControlRotation().Vector() * TeleportTriggerLength;

	//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Cyan, HalfSize.ToCompactString());

	FRotator BoxRotation = playerCharacter->GetControlRotation(); //is this ok, or revert to rotation from forward vector?
	//FRotator BoxRotation = playerCharacter->GetActorForwardVector().Rotation();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjToTrace;
	ObjToTrace.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	TArray<AActor*> Ignore;
	Ignore.Add(playerCharacter);

	TArray<FHitResult> HitResults;

	UKismetSystemLibrary::BoxTraceMultiForObjects(GetWorld(), Start, End, HalfSize, BoxRotation,
		ObjToTrace, true, Ignore, EDrawDebugTrace::None, HitResults, true, FColor::Red, FColor::Green, 1.5f);

	float MinDistance = TeleportTriggerLength + 100;
	//float MinDot = -1;
	ABaseEnemy* Closest = nullptr;
	for (auto HitResult : HitResults)
	{
		auto Enemy = Cast<ABaseEnemy>(HitResult.GetActor());
		if (!Enemy)
			continue;

		/*GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Green, FString::Printf(TEXT("MinDistance(Hit) = %f"), HitResult.Distance));
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Green, FString::Printf(TEXT("MinDistance(Math) = %f"), Enemy->GetDistanceTo(playerCharacter)));*/

		float CurrentDistance = Enemy->GetDistanceTo(playerCharacter);

		if (CurrentDistance < MinDistance)
		{
			MinDistance = CurrentDistance;
			Closest = Enemy;
			//MinDot = Dot;
		}

		FVector ToEnemy = Enemy->GetActorLocation() - playerCharacter->GetActorLocation();
		//float Dot = playerCharacter->GetActorForwardVector().Dot(ToEnemy);
		float Dot = playerCharacter->GetActorForwardVector().Dot(ToEnemy.GetSafeNormal());

		float InDot = 1.f / Dot;

		float DotWeight = 600.f;
		float distweight = .5f;

		float a = DotWeight * FMath::Abs(Dot - 0.7071) / CurrentDistance;
		Enemy->SetDebugTextValue(FString::SanitizeFloat(a));
		//Enemy->SetDebugTextValue(Dot * Dot * Dot);
	}

	if (Closest)
	{
		//bool bVerticalOverflow = false;
		FVector NextTargetPointOffset = GetAutoAimOffset(playerCharacter->GetMesh()->GetBoneLocation("head"), Closest->GetActorLocation());
		//const auto& [NextTargetPointOffset, VerticalOverflow] = GetAutoAimOffset(playerCharacter->GetActorLocation(), Closest->GetActorLocation());

		//TargetPointOffset = UKismetMathLibrary::VInterpTo(TargetPointOffset, NextTargetPointOffset, GetWorld()->GetDeltaSeconds(), .f);
		TargetPointOffset = NextTargetPointOffset;

		if (!bShouldIgnoreTeleport && MinDistance > KatanaTriggerLenSquared)
		{
			TeleportToClosestEnemy(Closest);
			//GetWorld()->GetTimerManager().ClearTimer(EnemiesTraceTimerHandle);
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
	auto ToPlayer = playerCharacter->GetActorLocation() - Enemy->GetActorLocation();

	//eye trace(move it out of the teleport function?)
	FVector EyeStart = playerCharacter->GetMesh()->GetBoneLocation("head");
	FVector EyeEnd = EyeStart + playerCharacter->GetControlRotation().Vector() * ToPlayer.Length();
	//FVector End = Start + playerCharacter->GetActorForwardVector() * MinDistance; //use forward vector or camera rotation? 
	FHitResult EyeOutHit;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjToTrace;
	ObjToTrace.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));

	bool bEyeHit = UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), EyeStart, EyeEnd,
		ObjToTrace, true, { playerCharacter, Katana },
		EDrawDebugTrace::None, EyeOutHit, true, FColor::Blue, FColor::Black, 5.f);

	//we hit a static object on the teleport path -> dont teleport
	if (bEyeHit)
	{
		//PRINT("Got obstacle between enemy and player");
		return;
	}

	PlayerStartForTeleport = playerCharacter->GetActorLocation();
	PlayerDestinationForTeleport = Enemy->GetActorLocation() + ToPlayer.GetSafeNormal() * KatanaTriggerLenSquared * 0.7f; //change to unsafe normal for perfomance?
	PlayerDestinationForTeleport.Z = Enemy->GetActorLocation().Z;

	float TraceDepth = playerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.f;
	FVector Start = PlayerDestinationForTeleport;

	FVector End = Start - (Enemy->GetActorUpVector() * TraceDepth);
	FHitResult OutHit;

	bool bHasGround = UKismetSystemLibrary::LineTraceSingle(GetWorld(), Start, End, 
		TEnumAsByte<ETraceTypeQuery>(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic)),
		true, TArray<AActor*>(), EDrawDebugTrace::ForDuration, OutHit, true, FColor::Red, FColor::Green, 5.f);

	if (bHasGround) 
	{
		//PRINT("has ground");
		//change Z so that it player has perfect teleport position and collision enabling won't cause chaos
		PlayerDestinationForTeleport = OutHit.Location;
		PlayerDestinationForTeleport.Z += playerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		ObjToTrace.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
		ObjToTrace.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

		auto playerCapsule = playerCharacter->GetCapsuleComponent();
		FHitResult CapsuleSpaceHit;

		bool bTeleportBlock = UKismetSystemLibrary::CapsuleTraceSingleForObjects(GetWorld(), PlayerDestinationForTeleport, PlayerDestinationForTeleport, 
			playerCapsule->GetScaledCapsuleRadius() * .8f, playerCapsule->GetScaledCapsuleHalfHeight() - 2.f,
			ObjToTrace, true, { playerCharacter }, EDrawDebugTrace::None, CapsuleSpaceHit, true);

		if (bTeleportBlock)
		{
			//PRINT("something blocks the spawn position");
			return;
		}

		PlayerOnTeleportRotation = playerCharacter->GetControlRotation();
		RotationToEnemy = playerCharacter->GetControlRotation();

		/*auto FakeDestination = PlayerDestinationForTeleport;
		FakeDestination.Z = playerCharacter->GetMesh()->GetBoneLocation("head").Z;*/

		FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(PlayerStartForTeleport, PlayerDestinationForTeleport); //change to enemies location?

		/*FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(playerCharacter->GetControlRotation().Vector(),
			(Enemy->GetActorLocation() - Start).GetUnsafeNormal());*/

		/*FRotator LookAt = UKismetMathLibrary::NormalizedDeltaRotator(playerCharacter->GetControlRotation(), 
			(Enemy->GetActorLocation() - PlayerDestinationForTeleport).Rotation());*/

		RotationToEnemy.Yaw = LookAt.Yaw;
		//RotationToEnemy.Pitch = LookAt.Pitch;

		auto FeetToHead = playerCharacter->GetMesh()->GetBoneLocation("head") - playerCharacter->GetMesh()->GetBoneLocation("root");
		auto CombatPoint = OutHit.Location + FeetToHead;

		CombatPoint += RotationToEnemy.Vector() * CharacterArmsLength + TargetPointOffset; //which check is better?

		//previous solution with calculating the target point 
		/*auto Dest = PlayerDestinationForTeleport;
		Dest.Z = playerCharacter->GetMesh()->GetBoneLocation("head").Z;*/
		//TargetPointOffset = GetAutoAimOffset(Dest, Enemy->GetActorLocation());
		//CombatPoint += TargetPointOffset;

		FVector EnemyBottom = Enemy->GetActorLocation() - Enemy->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * Enemy->GetActorUpVector();
		FVector EnemyTop = Enemy->GetActorLocation() + Enemy->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * Enemy->GetActorUpVector();

		/*DrawDebugLine(GetWorld(), CombatPoint, CombatPoint, FColor::Cyan, true, 1, 0, 3);

		DrawDebugLine(GetWorld(), EnemyBottom, EnemyBottom, FColor::Orange, true, 1, 0, 20);
		DrawDebugLine(GetWorld(), EnemyTop, EnemyTop, FColor::Green, true, 1, 0, 20);*/

		if (!UKismetMathLibrary::InRange_FloatFloat(CombatPoint.Z, EnemyBottom.Z, EnemyTop.Z))
			return;

		/*GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::Printf(TEXT("Length between = %f"), ToPlayer.Length()));
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::Printf(TEXT("Katana Trigger  = %f"), KatanaTriggerLenSquared));
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::Printf(TEXT("Teleport Trigger  = %f"), TeleportTriggerLength));*/

		PlayerCameraFOV = PlayerCameraManager->GetFOVAngle();

		float NormalizedTeleportTime = UKismetMathLibrary::MapRangeClamped(ToPlayer.Length(), KatanaTriggerLenSquared, 
			TeleportTriggerLength, MinTotalTeleportTime, MaxTotalTeleportTime);

		TeleportTimeline.SetPlayRate(1.f / NormalizedTeleportTime);

		bInTeleport = true;

		//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, TEXT("Teleport Started"));

		auto CurrentAttackMontage = CurrentAttackData.AttackMontage;
		float TimeToPerfectAttack = CurrentAttackData.PerfectAttackTime - AnimInstance->Montage_GetPosition(CurrentAttackMontage);
		float AcctualPlayRate = TimeToPerfectAttack / NormalizedTeleportTime * AttackSpeedMultiplier;

		//PRINT_F("Montage Speed up  = %f", AcctualPlayRate);

		AnimInstance->Montage_SetPlayRate(CurrentAttackMontage, AcctualPlayRate);

		playerCharacter->GetCharacterMovement()->DisableMovement();
		playerCharacter->GetController()->SetIgnoreLookInput(true);
		playerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		TeleportTimeline.PlayFromStart();

		OnIFramesChanged.Broadcast(true);
	}
}

float UCombatSystemComponent::GetNotifyTimeInMontage(UAnimMontage* Montage, FName NotifyName, FName TrackName)
{
	/*auto NotifyEvent = Montage->Notifies.FindByPredicate([&](const FAnimNotifyEvent& CurrentEvent) -> bool {
		return CurrentEvent.NotifyName.IsEqual(NotifyName);
		});*/

	/*auto notifies = Montage->Notifies;
	for (const auto& x : notifies)
	{
		UAnimNotify_PlayMontageNotify* MontageNotify = Cast<UAnimNotify_PlayMontageNotify>(x.Notify);
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Emerald, FString::Printf(TEXT("notify name = %f"), *MontageNotify->NotifyName.ToString()));
	}*/

	auto track = Montage->AnimNotifyTracks.FindByPredicate([&](const FAnimNotifyTrack& CurrentTrack) -> bool {
		return CurrentTrack.TrackName.IsEqual(TrackName);
		});

	if (track)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Emerald, FString::Printf(TEXT("track name = %s"), *track->TrackName.ToString()));
		//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Emerald, FString::Printf(TEXT("notify time = %f"), track->Notifies[0]->GetTriggerTime()));

		return track->Notifies[0]->GetTriggerTime();
	}
	
	return 0;
}

void UCombatSystemComponent::ExecuteSuperAbility()
{
	//PRINT("execute ability in progress", 0);

	FVector Start = playerCharacter->GetActorLocation();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjToTrace;
	ObjToTrace.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	TArray<AActor*> Ignore;
	Ignore.Add(playerCharacter);

	TArray<FHitResult> HitResults;
	UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), Start, Start, MaxJumpRadius, ObjToTrace,
		true, Ignore, EDrawDebugTrace::None, HitResults, true);

	if (HitResults.Num() == 0)
	{
		PRINT("No enemies nearby", 2);
		OnSuperAbilityCalled.Broadcast(false, "No enemies nearby");

		//GetWorld()->GetTimerManager().ClearTimer(SuperAbilityTimerHandle);
		CancelSuperAbility();
		//cancel superability?

		return;
	}

	SA_State = SuperAbilityState::WAITING;
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), SuperAbilitySlowMo);

	float MaxDot = -1;
	ABaseEnemy* Target = nullptr;

	for (auto HitResult : HitResults)
	{
		auto Enemy = Cast<ABaseEnemy>(HitResult.GetActor());
		if (!Enemy)
			continue;

		Enemy->SetDebugTextValue("-");
		//Enemy->SetEnableTargetWidget(false);

		FHitResult BlockHit;
		FVector EyeStart = playerCharacter->GetMesh()->GetBoneLocation("head");
		FVector EyeEnd = Enemy->GetActorLocation();

		FCollisionQueryParams CollisionParams;
		CollisionParams.bTraceComplex = true;
		CollisionParams.AddIgnoredActor(playerCharacter);
		CollisionParams.AddIgnoredActor(Katana);
		CollisionParams.AddIgnoredActor(Enemy);

		bool bIsBlocked = GetWorld()->LineTraceSingleByChannel(BlockHit, EyeStart, EyeEnd, ECollisionChannel::ECC_Visibility, CollisionParams);

		//DrawDebugLine(GetWorld(), EyeStart, EyeEnd, FColor::Cyan, false, 5);

		if (bIsBlocked)
		{
			Enemy->SetDebugTextValue("IM BLOCKED by " + UKismetSystemLibrary::GetDisplayName(BlockHit.GetComponent()));
			//DrawDebugBox(GetWorld(), BlockHit.ImpactPoint, FVector(5), FColor::Magenta, false, 5);
			continue;
		}


		FVector ToEnemy = Enemy->GetActorLocation() - playerCharacter->GetActorLocation();

		//auto ForwardVector2D = playerCharacter->GetActorForwardVector();
		//ForwardVector2D.Z = 0;
		////ForwardVector2D.Normalize(); // is normalization needed?
		//float dot = ForwardVector2D.Dot(ToEnemy.GetSafeNormal2D());

		float dot = playerCharacter->GetActorForwardVector().Dot(ToEnemy.GetSafeNormal2D());

		//Enemy->SetDebugTextValue(FString::SanitizeFloat(dot));

		if (dot >= .99f)
		{
			if (dot > MaxDot)
			{
				MaxDot = dot;
				Target = Enemy;
			}
		}
		/*else
			Enemy->SetEnableTargetWidget(false);*/
	}

	if (Target)
	{
		//Target->SetDebugTextValue("Current Target");
		//SA_State = SuperAbilityState::GOTTARGET;

		if (SuperAbilityTarget != Target)
		{
			Target->SetEnableTargetWidget(true);
			if (SuperAbilityTarget)
				SuperAbilityTarget->SetEnableTargetWidget(false);
		}
	}
	else if (SuperAbilityTarget)
		SuperAbilityTarget->SetEnableTargetWidget(false);

	SuperAbilityTarget = Target;
}

//void UCombatSystemComponent::WaitForTargets()
//{
//
//}

FVector UCombatSystemComponent::GetAutoAimOffset(FVector PlayerLocation, FVector EnemyLocation)
{
	auto ToEnemy = EnemyLocation - PlayerLocation;

	float MaxAbsoluteYOffset = 45.f;
	float TargetPointYOffset = FMath::Clamp(ToEnemy.Dot(playerCharacter->GetActorRightVector()),
		-MaxAbsoluteYOffset, MaxAbsoluteYOffset);

	//float MaxAbsoluteZOffset = 15.f;
	//PRINT_F("dot value = %f", ToEnemy.Dot(playerCharacter->GetActorUpVector()));
	float TargetPointZOffset = FMath::Clamp(ToEnemy.Dot(playerCharacter->GetActorUpVector()), -15, 5);

	return FVector(0.f, TargetPointYOffset, TargetPointZOffset);
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

	//TargetPointInitialPosition = PlayerCameraManager->camera;

	AnimInstance = playerCharacter->GetMesh()->GetAnimInstance();

	AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UCombatSystemComponent::PlayMontageNotifyBegin);
	AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &UCombatSystemComponent::PlayMontageNotifyEnd);
	AnimInstance->OnMontageBlendingOut.AddDynamic(this, &UCombatSystemComponent::PlayMontageFinished);

	for (auto& AttackMontageData : AttackMontages)
	{
		AttackMontageData.PerfectAttackTime = GetNotifyTimeInMontage(AttackMontageData.AttackMontage, "", "PerfectAttackTrack");
		//AttackMontageData.NormalizedChance = AttackMontageData.Chance / AttackMontages.Num();

		if (AttackMontageData.PerfectForCounter)
			CounterAttackMontages.Add(AttackMontageData);
	}

	NextAttackData = DetermineNextAttackData();


	/*CounterAttackMontages = AttackMontages.FilterByPredicate([&](const FAttackAnimData& animdata) -> bool {
		return animdata.PerfectForCounter;
		});*/

	/*for (auto x : CounterAttackMontages)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Cyan, FString::Printf(TEXT("counterattack name = %s"), *x.AttackMontage->GetName()));
	}*/

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
	if (SA_State == SuperAbilityState::WAITING && SuperAbilityTarget)
	{
		PRINTC("Super Ability attack", FColor::Red);
		GetWorld()->GetTimerManager().ClearTimer(SuperAbilityTimerHandle);

		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);

		SuperAbilityTarget->SetEnableTargetWidget(false);

		SA_State = SuperAbilityState::TELEPORTING;
		TeleportToClosestEnemy(SuperAbilityTarget);
	}
	else if (!CheckIfCanAttack())
		return;

	PRINTC("Normal attack", FColor::Cyan);

	//reset this cock-sucking plugin that barely works
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
		1 / 120.f, true);
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

	SpeedUpSlowMoTimeline();

	AnimInstance->Montage_Play(PerfectParryMontage, PerfectParryMontageSpeed, EMontagePlayReturnType::MontageLength);
}

void UCombatSystemComponent::InterruptPerfectParry()
{
	AnimInstance->Montage_Stop(0.1, PerfectParryMontage);
}

void UCombatSystemComponent::PerfectParryResponse(int InTokens = 0, bool bEnableSlowMo = true)
{
	if (InTokens != 0 && StolenTokens < MaxStolenTokens)
	{
		StolenTokens += InTokens;
		OnStolenTokensChanged.Broadcast(StolenTokens); //should be called when slow-mo timeline finishes?
	}

	if (bEnableSlowMo)
	{
		float part;
		int sec;
		UGameplayStatics::GetAccurateRealTime(sec, part);
		DebugTimeStamp = sec + part;
		bShouldSpeedUpSlowMoTimeline = false;
		ParrySlowMoTimeline.PlayFromStart();
	}


	AnimInstance->Montage_Play(ParryImpactMontage, ParryImpactMontageSpeed);
	//bInCombat = true;

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

	//PRINT("called ability", 2);

	if (StolenTokens < MaxStolenTokens)
	{
		PRINT("Not enough tokens", 2);
		OnSuperAbilityCalled.Broadcast(false, "Not enough tokens");
		return;
	}
	/*else if (!ExecuteSuperAbility())
	{
		PRINT("No enemies nearby");
		OnSuperAbilityCalled.Broadcast(false, "No enemies nearby");
		return;
	}*/

	//ExecuteSuperAbility();

	GetWorld()->GetTimerManager().SetTimer(SuperAbilityTimerHandle, this, &UCombatSystemComponent::ExecuteSuperAbility, 1 / 120.f, true);

	OnSuperAbilityCalled.Broadcast(true, "");
	StolenTokens = 0;
	OnStolenTokensChanged.Broadcast(StolenTokens);
}

void UCombatSystemComponent::CancelSuperAbility()
{
	//PRINT("canceled ability call", 2);
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
			AttackCameraShake,
			playerCharacter->GetActorLocation(), 
			0, 500, 1);

		/*auto current = playerCharacter->GetCurrentMontage();
		GetNotifyTimeInMontage(current, "TraceWindow", "PerfectAttackTrack");*/

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
	}
}

void UCombatSystemComponent::PlayMontageFinished(UAnimMontage* MontagePlayed, bool bWasInterrupted)
{
	if (MontagePlayed == PerfectParryMontage)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Cyan, playerCharacter->GetCurrentMontage()->GetName());
		if(!bWasInterrupted)
			bInCombat = false;
		bInParry = false;
	} 
	else if (/*AttackMontages.Contains(MontagePlayed)*/ CurrentAttackData.AttackMontage == MontagePlayed) //test this
	{
		HandleAttackEnd();
		/*if (bWasInterrupted)
		{
			PRINT("interupted");
		}
		else
		{
			PRINT("finished");
		}*/
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

	//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::Printf(TEXT("Timeline value = %f"), TeleportTimeline.GetPlaybackPosition()));

	//if (TeleportTimeline.GetPlaybackPosition() >= 1)
	//{
	//	//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, TEXT("playing again"));
	//	AnimInstance->Montage_Resume(CurrentAttackData.AttackMontage);
	//}
}

void UCombatSystemComponent::TimelineProgessRotation(float Value)
{
	auto NewRotation = FMath::Lerp(PlayerOnTeleportRotation, RotationToEnemy, Value);
	playerCharacter->GetController()->SetControlRotation(NewRotation);
}

void UCombatSystemComponent::TimelineProgessFOV(float Value)
{
	float NewFOV = FMath::Lerp(PlayerCameraFOV, MinFOVValue, Value);
	PlayerCameraManager->SetFOV(NewFOV);
}

void UCombatSystemComponent::TimelineProgessSlowMo(float Value)
{
	float SlowMoValue = FMath::Lerp(1.f, MinTimeDilation, Value);

	//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Green, FString::Printf(TEXT("Slow-mo value = %f"), SlowMoValue));
	if(!bShouldSpeedUpSlowMoTimeline)
		ParrySlowMoTimeline.SetPlayRate(1.f / SlowMoValue);
	
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), SlowMoValue);
}

void UCombatSystemComponent::TeleportTimelineFinish()
{
	//playerCharacter->GetCharacterMovement()->StopMovementImmediately();
	playerCharacter->GetController()->SetIgnoreLookInput(false);
	playerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	playerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	bInTeleport = false;

	if (SA_State == SuperAbilityState::TELEPORTING)
	{
		SuperAbilityTarget = nullptr;
		GetWorld()->GetTimerManager().SetTimer(SuperAbilityTimerHandle, this, &UCombatSystemComponent::ExecuteSuperAbility, 1 / 120.f, true);
	}

	//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, TEXT("TELEPORT FINISHED"));
}

void UCombatSystemComponent::SlowMoTimelineFinish()
{
	bShouldSpeedUpSlowMoTimeline = false;

	//debug:
	float part;
	int sec;
	UGameplayStatics::GetAccurateRealTime(sec, part);
	float totalseconds = sec + part;

	float elapsed = totalseconds - DebugTimeStamp;

	//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Green, FString::Printf(TEXT("Real time passed = %f"), elapsed));
}

// Called every frame
void UCombatSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TeleportTimeline.TickTimeline(DeltaTime);
	ParrySlowMoTimeline.TickTimeline(DeltaTime);
	
	if (!bInCombat) //change to not attacking??
	{
		GetLookInputVariables(CurrentCameraRotation);
		GetVelocityVariables();

		HandTotalOffset = HandSwayLookOffset + LocationLagPosition;
	}

	//TargetPointPosition = TargetPointOffset + TargetPointInitialPosition;
	PRINT_F("Super Ability State = %s", *UEnum::GetValueAsString(SA_State), 0);
	/*if (SuperAbilityTarget)
		PRINT_F("Super Ability Target = %s", *UKismetSystemLibrary::GetDisplayName(SuperAbilityTarget), 0);*/

	/*PRINT_B("Is Attacking %s", bIsAttacking);
	PRINT_B("Interputed By Itself %s", bInterputedByItself);
	PRINT_B("Can Rig Update %s", bCanRigUpdate);
	PRINT_B("In Combat %s", bInCombat);
	PRINT_B("In Parry %s", bInParry);*/
	PRINT_B("In Teleport %s", bInTeleport);

}