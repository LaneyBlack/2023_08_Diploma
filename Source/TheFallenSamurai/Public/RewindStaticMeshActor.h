// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "RewindStaticMeshActor.generated.h"

class URewindComponent;

UCLASS()
class THEFALLENSAMURAI_API ARewindStaticMeshActor : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rewind")
	URewindComponent* RewindComponent;

	ARewindStaticMeshActor();
};