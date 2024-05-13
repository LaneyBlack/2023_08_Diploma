// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "Templates/Tuple.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "CombatSystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIFramesChanged, bool, bIsImmortal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStolenTokensChanged, int, CurrentAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSuperAbilityCalled, bool, bWasSuccess, FString, FailReason); 
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSuperAbilityCancelled);

UENUM(BlueprintType)
enum class SuperAbilityState : uint8 
{
	NONE = 0 UMETA(DisplayName = "NONE"),
	WAITING = 1  UMETA(DisplayName = "WAITING"),
	GOTTARGET = 2  UMETA(DisplayName = "GOTTARGET"),
	TELEPORTING = 3  UMETA(DisplayName = "TELEPORTING")
};


USTRUCT(BlueprintType)
struct FAttackAnimData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Chance;				//how often should this montage be player

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool PerfectForCounter = false;				

	float NormalizedChance;		//direct probabilty of this montage being fired(relative to all montages present in the array)

	//float PlayRate;
	float PerfectAttackTime;

	//for later 
	//hand offset of crig
};

class AKatana;
class UCameraShakeBase;
template<typename... Types>
struct TTuple;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THEFALLENSAMURAI_API UCombatSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatSystemComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:	

	UPROPERTY()
	class ACharacter* playerCharacter;

	UPROPERTY()
	AKatana* Katana;

	UPROPERTY()
	class UDidItHitActorComponent* HitTracer;

	float MaxViewPitchValue = 35.f;

	float MinViewPitchValue = -70.f;

	bool bIsAttacking;

	bool bInterputedByItself;

	class UAnimInstance* AnimInstance;

	FAttackAnimData CurrentAttackData;

	FAttackAnimData NextAttackData;

	TArray<FAttackAnimData> CounterAttackMontages;

	class APlayerCameraManager* PlayerCameraManager;

	FVector KatanaPreviousPosition;

	FName KatanaSocketForDirection = "TraceEnd";

	FTimerHandle EnemiesTraceTimerHandle = FTimerHandle();

	FRotator CameraRotationRate;

	FRotator CurrentCameraRotation;

	FVector HandSwayLookOffset;

	FVector LocationLagPosition;

	float KatanaTriggerLenSquared;

	float TeleportTriggerLength;

	float CharacterArmsLength;

	bool bShouldIgnoreTeleport = false;

	FTimeline TeleportTimeline;

	FVector PlayerStartForTeleport;

	FVector PlayerDestinationForTeleport;

	FRotator PlayerOnTeleportRotation;

	FRotator RotationToEnemy;

	float PlayerCameraFOV;

	int StolenTokens = 0;

	FTimeline ParrySlowMoTimeline;

	float TimeDilationBeforeParry = 1.f;

	bool bShouldSpeedUpSlowMoTimeline = false;

	float DebugTimeStamp;

	//FVector TargetPointInitialPosition;

	FVector GetAutoAimOffset(FVector PlayerLocation, FVector EnemyLocation);

	SuperAbilityState SA_State = SuperAbilityState::NONE;

	ABaseEnemy* SuperAbilityTarget = nullptr;

	FTimerHandle SuperAbilityTimerHandle = FTimerHandle();

	UFUNCTION()
	bool CheckIfCanAttack();

	UFUNCTION()
	const FAttackAnimData& DetermineNextAttackData();

	UFUNCTION()
	const FAttackAnimData& DetermineNextCounterAttackData();

	UFUNCTION()
	void HandleAttackEnd();

	UFUNCTION()
	void ProcessHitReaction(AActor* HitActor, FVector ImpactPoint);

	UFUNCTION()
	FVector DetermineKatanaDirection();

	UFUNCTION()
	FVector GetKatanaSocketWorldPosition(FName SocketName);

	UFUNCTION()
	void GetEnemiesInViewportOnAttack();

	UFUNCTION()
	void GetLookInputVariables(FRotator PreviousCameraRotation);

	UFUNCTION()
	void GetVelocityVariables();

	/*UFUNCTION()
	void TraceForEnemiesToTeleport();*/

	UFUNCTION()
	void TeleportToClosestEnemy(ABaseEnemy* Enemy);

	UFUNCTION()
	float GetNotifyTimeInMontage(UAnimMontage* Montage, FName NotifyName, FName TrackName);

	UFUNCTION()
	void ExecuteSuperAbility();

	UFUNCTION()
	void SwingKatana();

	/*UFUNCTION()
	void WaitForTargets();*/

	/*UFUNCTION()
	bool ScanSurroundingForSuperAbility();*/
public:

	UPROPERTY(EditAnywhere, Category = "Attack Data|Animation")
	TArray<FAttackAnimData> AttackMontages;
	//TArray<UAnimMontage*> AttackMontages;

	UPROPERTY(EditAnywhere, Category = "Attack Data|Animation")
	float AttackSpeedMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Attack Data|Camera Shakes")
	TSubclassOf<UCameraShakeBase> AttackCameraShake;

	UPROPERTY(EditAnywhere, Category = "Attack Data|VFX")
	class UParticleSystem* BloodParticles;

	UPROPERTY(EditAnywhere, Category = "Attack Data|VFX")
	FVector BloodScale = FVector(.6f, .6f, .8f);

	UPROPERTY(EditAnywhere, Category = "Attack Data|Katana Collider")
	float KatanaBladeTriggerScale = 2.f;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|Animation")
	UAnimMontage* PerfectParryMontage;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|Animation")
	float PerfectParryMontageSpeed = 1.4f;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|Animation")
	UAnimMontage* ParryImpactMontage;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|Animation")
	float ParryImpactMontageSpeed = 1.3f;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|Camera Shakes")
	TSubclassOf<UCameraShakeBase> ParryCameraShake;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|Slow-mo")
	UCurveFloat* SlowMoCurve;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|Slow-mo")
	float MinTimeDilation = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|VFX")
	class UParticleSystem* PerfectParrySparks;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|VFX")
	FVector PerfectParrySparksSize = FVector(3.f, 3.f, 3.f);

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|VFX")
	class UParticleSystem* PerfectParryShockwave;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|VFX")
	FVector PerfectParryShockwaveSize = FVector(1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, Category = "Teleport Data")
	float TeleportTriggerScale = 3.f;

	UPROPERTY(EditAnywhere, Category = "Teleport Data")
	float MinTotalTeleportTime = .3f;

	UPROPERTY(EditAnywhere, Category = "Teleport Data")
	float MaxTotalTeleportTime = .3f;

	UPROPERTY(EditAnywhere, Category = "Teleport Data")
	float MinFOVValue = 70.f;

	UPROPERTY(EditAnywhere, Category = "Teleport Data|Interpolation Curves")
	UCurveFloat* LocationCurve;

	UPROPERTY(EditAnywhere, Category = "Teleport Data|Interpolation Curves")
	UCurveFloat* RotationCurve;

	UPROPERTY(EditAnywhere, Category = "Teleport Data|Interpolation Curves")
	UCurveFloat* FOVCurve;

	UPROPERTY(EditAnywhere, Category = "Super Ability")
	int MaxStolenTokens = 3;

	UPROPERTY(EditAnywhere, Category = "Super Ability")
	float MaxJumpRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Super Ability")
	int EnemyTargetLimit = 4;

	UPROPERTY(EditAnywhere, Category = "Super Ability")
	float SuperAbilitySlowMo = 0.1f;

	UPROPERTY(BlueprintAssignable)
	FOnStolenTokensChanged OnStolenTokensChanged;

	UPROPERTY(BlueprintAssignable)
	FOnIFramesChanged OnIFramesChanged;

	UPROPERTY(BlueprintAssignable)
	FOnSuperAbilityCalled OnSuperAbilityCalled;

	UPROPERTY(BlueprintAssignable)
	FOnSuperAbilityCancelled OnSuperAbilityCancelled;

	UPROPERTY(BlueprintReadOnly)
	bool bCanRigUpdate;
	
	UPROPERTY(BlueprintReadOnly)
	bool bInCombat;

	UPROPERTY(BlueprintReadOnly)
	bool bInParry;

	UPROPERTY(BlueprintReadOnly)
	FVector HandTotalOffset;

	UPROPERTY(BlueprintReadOnly)
	float YawSwayValue;

	UPROPERTY(BlueprintReadOnly)
	FVector TargetPointOffset;

	/*UPROPERTY(BlueprintReadOnly)
	FVector TargetPointPosition;*/

	UPROPERTY(BlueprintReadOnly)
	bool bInTeleport;

	UFUNCTION(BlueprintCallable)
	void InitializeCombatSystem(ACharacter* player, TSubclassOf<AKatana> KatanaActor);

	UFUNCTION(BlueprintCallable)
	void Attack();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	void GetLeftTransforms(FTransform& KatanaGripWorldTransform, FTransform& LeftHandSocket, FTransform& RightHandSocket);

	UFUNCTION(BlueprintCallable)
	void PerfectParry();

	UFUNCTION(BlueprintCallable)
	void InterruptPerfectParry();

	UFUNCTION(BlueprintCallable)
	void PerfectParryResponse(int InTokens, bool bEnableSlowMo);

	UFUNCTION(BlueprintCallable)
	void SuperAbility();

	UFUNCTION(BlueprintCallable)
	void CancelSuperAbility();

	UFUNCTION(BlueprintCallable)
	void SpeedUpSlowMoTimeline(float SpeedUpValue = 60.f);

	UFUNCTION()
	void PlayMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void PlayMontageNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void PlayMontageFinished(class UAnimMontage* MontagePlayed, bool bWasInterrupted);

	UFUNCTION()
	void TimelineProgessLocation(float Value);

	UFUNCTION()
	void TimelineProgessRotation(float Value);

	UFUNCTION()
	void TimelineProgessFOV(float Value);

	UFUNCTION()
	void TimelineProgessSlowMo(float Value);

	UFUNCTION()
	void TeleportTimelineFinish();

	UFUNCTION()
	void SlowMoTimelineFinish();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	
};
