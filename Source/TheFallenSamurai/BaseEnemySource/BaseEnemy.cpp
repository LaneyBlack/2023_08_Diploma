// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemy.h"
#include "Engine/StaticMeshActor.h"

// Sets default values
ABaseEnemy::ABaseEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ABaseEnemy::HandleHitReaction(const FVector& Impulse)
{
	if (!bIsGettingHit)
	{
		ApplyDamage();

		GetMesh()->HideBoneByName(HeadBoneName, EPhysBodyOp::PBO_None);

		AStaticMeshActor* SpawnedHead = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass());
		SpawnedHead->SetMobility(EComponentMobility::Movable);

		SpawnedHead->SetActorTransform(GetMesh()->GetBoneTransform(HeadBoneName));

		auto Location = SpawnedHead->GetActorLocation();
		Location.Z += 20.f;
		SpawnedHead->SetActorLocation(Location);

		SpawnedHead->SetActorScale3D(FVector(1.4f));

		UStaticMeshComponent* MeshComponent = SpawnedHead->GetStaticMeshComponent();
		if (MeshComponent)
		{
			MeshComponent->SetStaticMesh(HeadMesh);
			MeshComponent->SetSimulatePhysics(true);
			MeshComponent->SetEnableGravity(true);
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
			MeshComponent->SetAllMassScale(7);
			MeshComponent->AddImpulse(Impulse);
		}
		bIsGettingHit = true;
		return false;
	}

	return true;
}

// Called to bind functionality to input
//void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
//{
//	Super::SetupPlayerInputComponent(PlayerInputComponent);
//
//}

