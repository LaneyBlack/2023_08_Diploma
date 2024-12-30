// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "Templates/Tuple.h"
#include "CoreMinimal.h"
#include "ComboSystem.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CombatSystemComponent.generated.h"


class AKatana;
class UCameraShakeBase;
class ABaseEnemy;
class UParticleSystem;
template<typename... Types>
struct TTuple;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIFramesChanged, bool, bIsImmortal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemiesLeftChanged, int, EnemiesLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSuperAbilityTargetAcquired, bool, bFoundNewTarget); 
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

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//float Chance;				//how often should this montage be played

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool PerfectForCounter = false;				

	//float NormalizedChance;		//direct probabilty of this montage being fired(relative to all montages present in the array)

	float PerfectAttackTime;

	float StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UCameraShakeBase> AttackShake;

	bool bIsTeleportAttack = false;

	//for later 
	//hand offset of crig
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector AttackVector;

	FVector AttackWorldVector;
};

USTRUCT(BlueprintType)
struct FCombatHitData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector  CutPlaneNormal;

	UPROPERTY(BlueprintReadWrite)
	FVector  CutVelocity;

	UPROPERTY(BlueprintReadWrite)
	FVector  ImpactLocation;

	UPROPERTY(BlueprintReadWrite)
	AActor*  ActorCauser;

	UPROPERTY(BlueprintReadWrite)
	bool bSuperAbilityKill;
};

USTRUCT(BlueprintType)
struct FTeleportProperties
{
	GENERATED_BODY()

	float TeleportDistance;

	FTimeline Timeline;

	float FOVChange;

	bool bPickTimeFromRange = true;

	float MinTime;
	float MaxTime;
};

USTRUCT(BlueprintType)
struct FValidationRules
{
	GENERATED_BODY()

	bool bUseDebugPrint = false;
	EDrawDebugTrace::Type DrawDebugTrace = EDrawDebugTrace::None;

	float GroundTraceDepth;
	bool bUsePitch = true;
	bool bUseLazyCheck = true;
	int ChecksSampleScale = 1; //how granular the checks are placed: bigger number -> they are more "packed"
	bool bShouldIgnoreShields = false;
};

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
	class ATheFallenSamuraiCharacter* playerCharacter;

	UPROPERTY()
	AKatana* Katana;

	UPROPERTY()
	class UDidItHitActorComponent* HitTracer;

	float MaxViewPitchValue = 70.f;

	float MinViewPitchValue = -70.f;

	bool bIsAttacking;

	bool bInterputedByItself;

	class UAnimInstance* AnimInstance;

	FAttackAnimData NextAttackData;

	TArray<FAttackAnimData> CounterAttackMontages;

	TSet<AActor*> HitActorsOnSwing;

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

	FTimeline ParrySlowMoTimeline;

	float TimeDilationBeforeParry = 1.f;

	bool bShouldSpeedUpSlowMoTimeline = false;

	SuperAbilityState SA_State = SuperAbilityState::NONE;

	ABaseEnemy* SuperAbilityTarget = nullptr;

	UComboSystem* ComboSystem;

	FTimerHandle SuperAbilityTimerHandle = FTimerHandle();
	
	int SuperAbilityTargetsLeft = -1;

	TSet<ABaseEnemy*> PostProcessSA_Targets;

	UFUNCTION()
	void ClearAffectedByPostProcess();

	UFUNCTION()
	FVector GetAutoAimOffset(const FVector& PlayerLocation, const FVector& EnemyLocation, const FVector& PlayerForwardVector, const FVector& PlayerUpVector);

	UFUNCTION()
	bool CheckIfCanAttack();

	UFUNCTION()
	const FAttackAnimData& DetermineNextAttackData();

	UFUNCTION()
	const FAttackAnimData& DetermineNextCounterAttackData();

	UFUNCTION()
	void HandleAttackEnd(bool bShouldPerformFinalTraceCheck = true);

	UFUNCTION()
	void ProcessHitResult(const FHitResult& HitResult);

	UFUNCTION()
	void ProcessHitResponse(float ImpulseStrength, const FVector& ImapctPoint);

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

	UFUNCTION()
	bool CheckIsTeleportTargetObscured(ABaseEnemy* Enemy);

	UFUNCTION()
	bool CheckIsShieldProtected(const FVector& ToPlayerNormalized, const FVector& EnemyForward);

	UFUNCTION()
	bool ValidateTeleportTarget(ABaseEnemy* Enemy, const FValidationRules& ValidationRules);

	UFUNCTION()
	bool PerformTeleportCheck(ABaseEnemy* Enemy, const FVector& EnemyLocationOverTime, const FVector& Direction, float TraceDepth,
		 float BlockCapsuleRadius, float BlockCapsuleHalfHeight, const FValidationRules& ValidationRules);

	UFUNCTION()
	void TeleportToEnemy(float TeleportDistance);

	UFUNCTION()
	float GetNotifyTimeInMontage(UAnimMontage* Montage, FName NotifyName);

	UFUNCTION()
	void ExecuteSuperAbility();

	UFUNCTION()
	void SwingKatana();

public:

	UPROPERTY(BlueprintReadOnly, Category = "Attack Data|Animation")
	FAttackAnimData CurrentAttackData;

	UPROPERTY(EditAnywhere, Category = "Attack Data|Animation")
	TArray<FAttackAnimData> AttackMontages;

	UPROPERTY(EditAnywhere, Category = "Attack Data|Animation")
	float AttackSpeedMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Attack Data|Hit Reaction")
	TSubclassOf<UCameraShakeBase> HitCameraShake;

	UPROPERTY(EditAnywhere, Category = "Attack Data|Hit Reaction")
	UParticleSystem* DefaultHitParticles;

	UPROPERTY(EditAnywhere, Category = "Attack Data|VFX")
	UParticleSystem* BloodParticles;

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
	UParticleSystem* PerfectParryShockwave;

	UPROPERTY(EditAnywhere, Category = "Perfect Parry Data|VFX")
	FVector PerfectParryShockwaveSize = FVector(1.f, 1.f, 1.f);


	UPROPERTY(EditAnywhere, Category = "Teleport Data")
	float TeleportTriggerScale = 3.f;

	UPROPERTY(EditAnywhere, Category = "Teleport Data")
	float MinTotalTeleportTime = .3f;

	UPROPERTY(EditAnywhere, Category = "Teleport Data")
	float MaxTotalTeleportTime = .3f;

	UPROPERTY(EditAnywhere, Category = "Teleport Data")
	float TeleportFOVChange = 75.f;

	UPROPERTY(EditAnywhere, Category = "Teleport Data|Interpolation Curves")
	UCurveFloat* LocationCurve;

	UPROPERTY(EditAnywhere, Category = "Teleport Data|Interpolation Curves")
	UCurveFloat* RotationCurve;

	UPROPERTY(EditAnywhere, Category = "Teleport Data|Interpolation Curves")
	UCurveFloat* FOVCurve;

	UPROPERTY(EditAnywhere, Category = "Shield Reaction Data|Teleport")
	float ShieldIgnoreAngle = 0.f;

	UPROPERTY(EditAnywhere, Category = "Shield Reaction Data|Camera Shake")
	TSubclassOf<UCameraShakeBase> ShieldHitCameraShake;

	UPROPERTY(EditAnywhere, Category = "Shield Reaction Data|VFX")
	UParticleSystem* ShieldHitParticle;

	UPROPERTY(EditAnywhere, Category = "Shield Reaction Data|VFX")
	float UniformShieldHitParticleSize;

	UPROPERTY(EditAnywhere, Category = "Shield Reaction Data")
	float ShieldHitImpulse = 600.f;

	UPROPERTY(EditAnywhere, Category = "Shield Reaction Data")
	float OnHitAnimationBlendTime = .5f;

	UPROPERTY(EditAnywhere, Category = "Super Ability")
	float MaxJumpRadius = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Super Ability")
	int SuperAbilityTargetLimit = 4;

	UPROPERTY(EditAnywhere, Category = "Super Ability")
	float SuperAbilitySlowMo = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Super Ability")
	float LookRateScale = 0.5f;

	UPROPERTY(BlueprintAssignable)
	FOnIFramesChanged OnIFramesChanged;

	UPROPERTY(BlueprintAssignable)
	FOnSuperAbilityCalled OnSuperAbilityCalled;

	UPROPERTY(BlueprintAssignable)
	FOnSuperAbilityCancelled OnSuperAbilityCancelled;

	UPROPERTY(BlueprintAssignable)
	FOnSuperAbilityTargetAcquired OnSuperAbilityTargetAcquired;

	UPROPERTY(BlueprintAssignable)
	FOnEnemiesLeftChanged OnEnemiesLeftChanged;

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

	UPROPERTY(BlueprintReadOnly)
	bool bInTeleport;

	UFUNCTION(BlueprintCallable)
	void InitializeCombatSystem(ATheFallenSamuraiCharacter* player, TSubclassOf<AKatana> KatanaActor);

	UFUNCTION()
	void Attack();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	void GetLeftTransforms(FTransform& KatanaGripWorldTransform, FTransform& LeftHandSocket, FTransform& RightHandSocket);

	UFUNCTION()
	void PerfectParry();

	UFUNCTION(BlueprintCallable)
	void PerfectParryResponse(bool bEnableSlowMo = true);

	UFUNCTION()
	void SuperAbility();

	UFUNCTION(BlueprintCallable)
	void CancelSuperAbility();

	UFUNCTION(BlueprintCallable)
	bool IsSuperAbilityActive();

	float GetLookRate();

	UFUNCTION(BlueprintCallable)
	void SpeedUpSlowMoTimeline(float SpeedUpValue = 80.f);

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

	UFUNCTION()
	void OnComboPointsChanged(int32 NewComboPoints);

private:
	int32 CurrentComboPoints;
};
