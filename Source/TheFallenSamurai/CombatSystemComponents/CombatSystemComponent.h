// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatSystemComponent.generated.h"

class AKatana;

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

	UPROPERTY(EditDefaultsOnly)
	class ACharacter* playerCharacter;

	UPROPERTY(EditDefaultsOnly)
	AKatana* Katana;

	UPROPERTY(EditDefaultsOnly)
	class UDidItHitActorComponent* HitTracer;

public:

	UPROPERTY(EditAnywhere)
	TSubclassOf<AKatana> KatanaActor;

	UFUNCTION(BlueprintCallable)
	void InitializeComponent(ACharacter* player);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	
};
