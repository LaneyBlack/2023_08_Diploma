// Fill out your copyright notice in the Description page of Project Settings.


#include "TheFallenSamurai/Slicebles/SlicableActor.h"

// Sets default values
ASlicableActor::ASlicableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASlicableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASlicableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

