// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Katana.generated.h"

UCLASS()
class THEFALLENSAMURAI_API AKatana : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKatana();

	float ColliderMaxDistanceSquared;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* KatanaMesh;

	UPROPERTY(EditAnywhere)
	class UDidItHitActorComponent* HitTracer;

	UFUNCTION()
	void OffsetTraceEndSocket(float OffsetScale);

	UFUNCTION()
	FVector GetBladeWorldVector();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
