// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseEnemy.generated.h"

UCLASS()
class THEFALLENSAMURAI_API ABaseEnemy : public ACharacter
{
	GENERATED_BODY()

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsGettingHit;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	bool HandleHitReaction(const FVector& Impulse);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ApplyDamage();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetDebugTextValue(const FString& value);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetEnableTargetWidget(bool bIsSelected);

	// Called to bind functionality to input
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
private:
	UStaticMesh* CreateStaticMeshFromSkeletalMeshPose(USkeletalMeshComponent* SkeletalMeshComponent);

	void ConvertAndSpawnStaticMeshFromPose(UWorld* World, USkeletalMeshComponent* SkeletalMeshComponent, const FTransform& Transform);
};
