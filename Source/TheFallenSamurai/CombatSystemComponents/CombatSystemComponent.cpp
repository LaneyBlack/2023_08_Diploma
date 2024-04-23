// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatSystemComponent.h"

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

#include "Kismet/GameplayStatics.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

#include "DidItHitActorComponent.h"

#include "TheFallenSamurai/KatanaSource/Katana.h"
#include "TheFallenSamurai/BaseEnemySource/BaseEnemy.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UCombatSystemComponent::UCombatSystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCombatSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

bool UCombatSystemComponent::CheckIfCanAttack()
{
	return !bIsAttacking && (bInterputedByItself || !AttackMontages.Contains(playerCharacter->GetCurrentMontage()));
}

UAnimMontage* UCombatSystemComponent::DetermineNextMontage()
{
	auto montage = AttackMontages[FMath::RandRange(0, AttackMontages.Num() - 1)];
	//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Cyan, FString::Printf(TEXT("Montage name = %s"), *montage->GetName()));
	return montage;
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

	//debug draw

	return Direction;
}

FVector UCombatSystemComponent::GetKatanaSocketWorldPosition(FName SocketName)
{
	return Katana->KatanaMesh->GetSocketLocation(SocketName);
}

void UCombatSystemComponent::GetEnemiesInViewportOnAttack()
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Emerald, TEXT("TRACING DURING ATTACK"));

	for (auto& result : HitTracer->HitArray)
	{
		ProcessHitReaction(result.GetActor(), result.ImpactPoint);
		result.Reset();
	}

	auto CapsuleComponent = playerCharacter->GetCapsuleComponent();
	FVector StartEnd = playerCharacter->GetActorLocation() 
		+ playerCharacter->GetActorForwardVector() * (2.5f * CapsuleComponent->GetScaledCapsuleRadius());

	FVector HalfSize;
	HalfSize.X = HalfSize.Y = CapsuleComponent->GetScaledCapsuleRadius() * 3.5f;
	HalfSize.Z = CapsuleComponent->GetScaledCapsuleHalfHeight() * 1.5f;

	FRotator BoxRotation = playerCharacter->GetActorForwardVector().Rotation();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjToTrace;
	ObjToTrace.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	TArray<AActor*> Ignore;
	Ignore.Add(playerCharacter);

	FHitResult HitResult;
	bool bHit = UKismetSystemLibrary::BoxTraceSingleForObjects(GetWorld(), StartEnd, StartEnd, HalfSize, BoxRotation, 
		ObjToTrace, true, Ignore, EDrawDebugTrace::ForDuration, HitResult, true);

	auto Enemy = Cast<ABaseEnemy>(HitResult.GetActor());
	if (bHit && Enemy)
	{
		auto VectorToEnemy = Enemy->GetActorLocation() - playerCharacter->GetActorLocation();
		float TargetPointYOffset = FMath::Clamp(VectorToEnemy.Dot(playerCharacter->GetActorRightVector()), -30, 30);
		TargetPointOffset = UKismetMathLibrary::VInterpTo(TargetPointOffset,
			FVector(0.f, TargetPointYOffset, 0.f),
			GetWorld()->GetDeltaSeconds(),
			20.f);

		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Emerald, FString::Printf(TEXT("Target point offset = %f"), TargetPointYOffset));
	}

}

void UCombatSystemComponent::InitializeCombatSystem(ACharacter* player, TSubclassOf<AKatana> KatanaActor)
{
	playerCharacter = player;

	FActorSpawnParameters KatanaSpawnParams;
	KatanaSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	KatanaSpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

	Katana = GetWorld()->SpawnActor<AKatana>(KatanaActor, player->GetTransform(), KatanaSpawnParams);
	
	HitTracer = Katana->HitTracer;
	//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Magenta, TEXT("hit tracer box extent = ") + HitTracer->BoxHalfSize.ToCompactString());

	EAttachmentRule KatanaAttachRules = EAttachmentRule::SnapToTarget;

	Katana->K2_AttachToComponent(player->GetMesh(), "KatanaSocket",
		KatanaAttachRules, KatanaAttachRules, KatanaAttachRules,
		true);
 
	PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	PlayerCameraManager->ViewPitchMax = MaxViewPitchValue;
	PlayerCameraManager->ViewPitchMin = MinViewPitchValue;

	AnimInstance = playerCharacter->GetMesh()->GetAnimInstance();

	AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UCombatSystemComponent::PlayMontageNotifyBegin);
	AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &UCombatSystemComponent::PlayMontageNotifyEnd);
	AnimInstance->OnMontageBlendingOut.AddDynamic(this, &UCombatSystemComponent::PlayMontageFinished);
}

void UCombatSystemComponent::Attack()
{
	if (!CheckIfCanAttack())
		return;

	bIsAttacking = true;
	bInterputedByItself = false;

	auto MontageToPlay = DetermineNextMontage();
	AnimInstance->Montage_Play(MontageToPlay, AttackSpeedMultiplier);

	//start timer
	GetWorld()->GetTimerManager().SetTimer(EnemiesTraceTimerHandle, this,
		&UCombatSystemComponent::GetEnemiesInViewportOnAttack, 
		1 / 120.f, true);
}

void UCombatSystemComponent::GetLeftTransforms(FTransform& KatanaGripWorldTransform, FTransform& LeftHandSocket, FTransform& RightHandSocket)
{

}

void UCombatSystemComponent::PlayMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Green, TEXT("Notify Begin: ") + NotifyName.ToString());
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
}

void UCombatSystemComponent::PlayMontageNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, TEXT("Notify End: ") + NotifyName.ToString());

	if (NotifyName.IsEqual("TraceWindow"))
	{
		bInterputedByItself = true;

		//more to be done!
		HandleAttackEnd();
	}
}

void UCombatSystemComponent::PlayMontageFinished(UAnimMontage* MontagePlayed, bool bWasInterrupted)
{
	if (bWasInterrupted)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Emerald, MontagePlayed->GetName() + TEXT("MONTAGE WAS INTERUPTED!"));
		HandleAttackEnd();
	}
}

// Called every frame
void UCombatSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/*if(!HitTracer->HitArray.IsEmpty())
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Cyan, FString::Printf(TEXT("hit array num = %i"), HitTracer->HitArray.Num()));
		*/
}