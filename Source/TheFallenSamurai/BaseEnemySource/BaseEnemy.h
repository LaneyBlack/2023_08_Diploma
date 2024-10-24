// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseEnemy.generated.h"

class UProceduralMeshComponent;
struct FCombatHitData;

UCLASS()
class THEFALLENSAMURAI_API ABaseEnemy : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UProceduralMeshComponent* ProceduralMesh;

public:
	// Sets default values for this character's properties
	ABaseEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismemberment")
	UStaticMesh* HeadMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismemberment")
	FName HeadBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismemberment")
	float DismembermentStrength = 5000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsGettingHit;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	bool bOwnsShield = false;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	bool HandleHitReaction(const FCombatHitData& CombatHitData);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ApplyDamage(const FCombatHitData& CombatHitData);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetDebugTextValue(const FString& value);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetEnableTargetWidget(bool bIsSelected);

	//old
	UFUNCTION(BlueprintCallable)
	void ConvertSkeletalMeshToProceduralMesh(USkeletalMeshComponent* InSkeletalMeshComponent, int32 LODIndex, UProceduralMeshComponent* InProcMeshComponent);

	//newest functino
	UFUNCTION(BlueprintCallable)
	void CopySkeletalMeshToProcedural(USkeletalMeshComponent* SkeletalMeshComponent, int32 LODIndex, UProceduralMeshComponent* ProcMeshComponent);
	
	// Called to bind functionality to input
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
