// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatSystemComponent.h"
#include "GameFramework/Character.h"
#include "TheFallenSamurai/KatanaSource/Katana.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "DidItHitActorComponent.h"

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

	//clear time handle for auto aim function

	bIsAttacking = false;
	bInCombat = false;

	HitTracer->ToggleTraceCheck(false);

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
}

void UCombatSystemComponent::PlayMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Green, TEXT("Notify Begin: ") + NotifyName.ToString());
	if (NotifyName.IsEqual("TraceWindow"))
	{
		bInCombat = true;
		HitTracer->ToggleTraceCheck(true);
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Orange, TEXT("HANDLE KATANA PREVIOUS POSITION CALS!"));
		
		PlayerCameraManager->PlayWorldCameraShake(GetWorld(), 
			AttackCamShake,
			playerCharacter->GetActorLocation(), 
			0, 500, 1);
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
	GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, TEXT("Notify End: ") + NotifyName.ToString());

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
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Emerald, MontagePlayed->GetName() + TEXT("MONTAGE WAS INTERUPTED!"));
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