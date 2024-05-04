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
	return *CounterAttackMontages[index++ % CounterAttackMontages.Num()];
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

	TArray<FHitResult> HitResults;

	UKismetSystemLibrary::BoxTraceMultiForObjects(GetWorld(), StartEnd, StartEnd, HalfSize, BoxRotation,
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
		/*FVector ToEnemy = Enemy->GetActorLocation() - playerCharacter->GetActorLocation();
		float Dot = playerCharacter->GetActorForwardVector().Dot(ToEnemy);*/
		//float Dot = playerCharacter->GetActorForwardVector().Dot(ToEnemy.GetSafeNormal());

		//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Green, FString::Printf(TEXT("Dot = %f"), Dot));

		if (CurrentDistance < MinDistance)
		{
			MinDistance = CurrentDistance;
			Closest = Enemy;
			//MinDot = Dot;
		}
	}

	if (Closest)
	{
		auto ToEnemy = Closest->GetActorLocation() - playerCharacter->GetActorLocation();

		float MaxAbsoluteYOffset = 45.f;
		float TargetPointYOffset = FMath::Clamp(ToEnemy.Dot(playerCharacter->GetActorRightVector()),
			-MaxAbsoluteYOffset, MaxAbsoluteYOffset);

		TargetPointOffset = UKismetMathLibrary::VInterpTo(TargetPointOffset,
			FVector(0.f, TargetPointYOffset, 0.f),
			GetWorld()->GetDeltaSeconds(),
			25.f);

		if (!bShouldIgnoreTeleport && MinDistance > KatanaTriggerLenSquared)
		{
			TeleportToClosestEnemy(Closest);
			GetWorld()->GetTimerManager().ClearTimer(EnemiesTraceTimerHandle);
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
		EDrawDebugTrace::None, EyeOutHit, true, FColor::Red, FColor::Green, 5.f);

	//we hit a static object on the teleport path -> dont teleport
	if (bEyeHit)
		return;

	PlayerStartForTeleport = playerCharacter->GetActorLocation();
	PlayerDestinationForTeleport = Enemy->GetActorLocation() + ToPlayer.GetSafeNormal() * KatanaTriggerLenSquared * 0.7f; //change to unsafe normal for perfomance?
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
		TEnumAsByte<ETraceTypeQuery>(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic)),
		true, TArray<AActor*>(), EDrawDebugTrace::None, OutHit, true, FColor::Red, FColor::Green, 5.f);

	/*if (bHit)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Emerald, FString::Printf(TEXT("Allowed Trace Depth = %f"), TraceDepth * .4));
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Emerald, FString::Printf(TEXT("Distance = %f"), OutHit.Distance));
	}*/

	if (bHit && OutHit.Distance >= (TraceDepth * 0.4f)) //must be change to be relatve to players capsule
	{
		//DrawDebugLine(GetWorld(), Start, Start - (Enemy->GetActorUpVector() * OutHit.Distance), FColor::Cyan, false, 5.f, 0, 1.5);

		//to do better collision check

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

		PlayerCameraFOV = PlayerCameraManager->GetFOVAngle();

		float NormalizedTeleportTime = UKismetMathLibrary::MapRangeClamped(ToPlayer.Length(), KatanaTriggerLenSquared, 
			TeleportTriggerLength, MinTotalTeleportTime, MaxTotalTeleportTime);

		TeleportTimeline.SetPlayRate(1.f / NormalizedTeleportTime);

		bInTeleport = true;

		//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, TEXT("Teleport Started"));

		auto CurrentAttackMontage = CurrentAttackData.AttackMontage;
		float TimeToPerfectAttack = CurrentAttackData.PerfectAttackTime - AnimInstance->Montage_GetPosition(CurrentAttackMontage);
		float AcctualPlayRate = TimeToPerfectAttack / NormalizedTeleportTime * AttackSpeedMultiplier;

		//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::Printf(TEXT("Montage Speed up  = %f"), AcctualPlayRate));

		AnimInstance->Montage_SetPlayRate(CurrentAttackMontage, AcctualPlayRate);

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


	//if (NotifyTrack)
	//{
	//	// Iterate through the notifies in the notify track
	//	for (const FAnimNotifyEvent& NotifyEvent : NotifyTrack->Notifies)
	//	{
	//		// Check if this is the notify we are looking for
	//		if (NotifyEvent.NotifyName == NotifyName)
	//		{
	//			// Calculate the remaining time before the notify is fired
	//			float TimeToNotify = NotifyEvent.GetTime() - MontagePosition;

	//			// Now TimeToNotify contains the time remaining before the notify fires
	//			// You can use this value as needed
	//			break;
	//		}
	//	}
	//}
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

	NextAttackData = DetermineNextAttackData();

	for (auto& anim : AttackMontages)
	{
		if (anim.PerfectForCounter)
			CounterAttackMontages.Add(&anim);
	}

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

	for (auto& AttackMontageData : AttackMontages)
	{
		AttackMontageData.PerfectAttackTime = GetNotifyTimeInMontage(AttackMontageData.AttackMontage, "", "PerfectAttackTrack");
		//AttackMontageData.NormalizedChance = AttackMontageData.Chance / AttackMontages.Num();
	}
}

void UCombatSystemComponent::Attack()
{
	if (!CheckIfCanAttack())
		return;

	//reset this cock-sucking plugin that barely works
	HitTracer->ClearHitArray();

	//quickly stop perfect parry montage
	AnimInstance->Montage_Stop(0.01, PerfectParryMontage);

	SpeedUpSlowMoTimeline();

	bIsAttacking = true;
	bInterputedByItself = false;
	bShouldIgnoreTeleport = false;

	CurrentAttackData = NextAttackData;

	float AttackMontageStartPercent = .21f;
	AnimInstance->Montage_Play(CurrentAttackData.AttackMontage, AttackSpeedMultiplier, EMontagePlayReturnType::MontageLength, AttackMontageStartPercent);

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
	if(StolenTokens < MaxStolenTokens)
		StolenTokens += InTokens;

	if (bEnableSlowMo)
	{
		float part;
		int sec;
		UGameplayStatics::GetAccurateRealTime(sec, part);
		DebugTimeStamp = sec + part;
		bShouldSpeedUpSlowMoTimeline = false;
		ParrySlowMoTimeline.PlayFromStart();
		//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, TEXT("Timeline start"));
	}

	//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::Printf(TEXT("Total Stolen Tokens = %i"), StolenTokens));

	//AnimInstance->Montage_Stop(0.1, PerfectParryMontage);
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
}