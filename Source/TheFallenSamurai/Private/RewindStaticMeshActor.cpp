// Fill out your copyright notice in the Description page of Project Settings.

#include "RewindStaticMeshActor.h"

#include "Components/StaticMeshComponent.h"
#include "RewindComponent.h"

ARewindStaticMeshActor::ARewindStaticMeshActor()
{
	PrimaryActorTick.bCanEverTick = false;
	GetStaticMeshComponent()->Mobility = EComponentMobility::Movable;
	GetStaticMeshComponent()->SetSimulatePhysics(true);
	
	RewindComponent = CreateDefaultSubobject<URewindComponent>(TEXT("RewindComponent"));
	RewindComponent->SnapshotFrequencySeconds = 1.0f / 30.0f;
}
