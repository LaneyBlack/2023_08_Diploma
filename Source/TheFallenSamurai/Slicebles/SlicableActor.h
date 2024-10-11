// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SlicableActor.generated.h"

UCLASS()
class THEFALLENSAMURAI_API ASlicableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASlicableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UFUNCTION(BlueprintImplementableEvent)
	void SliceActor(const FVector& SliceVector, const FVector& CollisionPoint);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
