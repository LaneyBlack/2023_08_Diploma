// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatSystemComponent.h"
#include "GameFramework/Character.h"
#include "TheFallenSamurai/KatanaSource/Katana.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UCombatSystemComponent::UCombatSystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCombatSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UCombatSystemComponent::InitializeComponent(ACharacter* player, TSubclassOf<AKatana> KatanaActor)
{
	playerCharacter = player;

	FActorSpawnParameters KatanaSpawnParams;
	KatanaSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	KatanaSpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

	Katana = GetWorld()->SpawnActor<AKatana>(KatanaActor, player->GetTransform(), KatanaSpawnParams);
	
	HitTracer = Katana->HitTracer;

	EAttachmentRule KatanaAttachRules = EAttachmentRule::SnapToTarget;

	Katana->K2_AttachToComponent(player->GetMesh(), "KatanaSocket",
		KatanaAttachRules, KatanaAttachRules, KatanaAttachRules,
		true);

	auto CamManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	CamManager->ViewPitchMax = MaxViewPitchValue;
	CamManager->ViewPitchMin = MinViewPitchValue;
}

// Called every frame
void UCombatSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

