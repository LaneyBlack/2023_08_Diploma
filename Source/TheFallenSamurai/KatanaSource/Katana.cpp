// Fill out your copyright notice in the Description page of Project Settings.


#include "Katana.h"
#include "Components/StaticMeshComponent.h"
#include "DidItHitActorComponent.h"

// Sets default values
AKatana::AKatana()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	KatanaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Katana Mesh"));
	RootComponent = KatanaMesh;
	
	HitTracer = CreateDefaultSubobject<UDidItHitActorComponent>(TEXT("Hit Tracer"));
}

// Called when the game starts or when spawned
void AKatana::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AKatana::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

