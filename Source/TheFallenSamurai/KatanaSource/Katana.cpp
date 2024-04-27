// Fill out your copyright notice in the Description page of Project Settings.


#include "Katana.h"
#include "Components/StaticMeshComponent.h"
#include "DidItHitActorComponent.h"
#include "Engine/StaticMeshSocket.h"

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

	HitTracer->SetupVariables(KatanaMesh, nullptr);

	//DEBUG:
	/*auto names = HitTracer->SetupVariables(KatanaMesh, nullptr);
	for (auto name : names)
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Cyan, name.ToString());*/
	
}

void AKatana::OffsetTraceEndSocket(float OffsetScale)
{
	const UStaticMeshSocket* TraceStartSocket = KatanaMesh->GetSocketByName("TraceStart");
	const UStaticMeshSocket* TraceEndSocket = KatanaMesh->GetSocketByName("TraceEnd");

	const UStaticMeshSocket* BladeEndSocket = KatanaMesh->GetSocketByName("BladeEnd");

	auto TraceStart = TraceStartSocket->RelativeLocation;
	auto BladeEnd = BladeEndSocket->RelativeLocation;

	FVector ColliderVector = (BladeEnd - TraceStart) * OffsetScale;
	FVector TraceEndFinalPosition = TraceStart + ColliderVector;
	const_cast<UStaticMeshSocket*>(TraceEndSocket)->RelativeLocation = TraceEndFinalPosition;

	ColliderMaxDistanceSquared = ColliderVector.SquaredLength();
}

FVector AKatana::GetBladeWorldVector()
{
	/*auto TraceStart = KatanaMesh->GetSocketByName("TraceStart")->RelativeLocation;
	auto BladeEnd = KatanaMesh->GetSocketByName("BladeEnd")->RelativeLocation;
	auto Direction = BladeEnd - TraceStart;*/

	auto TraceStart = KatanaMesh->GetSocketLocation("TraceStart");
	auto BladeEnd = KatanaMesh->GetSocketLocation("BladeEnd");
	auto Direction = BladeEnd - TraceStart;

	return Direction;
}

// Called every frame
void AKatana::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

