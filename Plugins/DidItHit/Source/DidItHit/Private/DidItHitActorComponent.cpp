// Copyright 2021, Project Zero. All Rights Reserved.


/*All the time you're saying to yourself,
'I could do that, but I won't,' —
which is just another way of saying that you can't.*/

#include "DidItHitActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"




// Sets default values for this component's properties
UDidItHitActorComponent::UDidItHitActorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	

	// ...
}


// Called when the game starts
void UDidItHitActorComponent::BeginPlay()
{
	Super::BeginPlay();
}

/*To every man is given the key to the gates of heaven.
The same key opens the gates of hell.
And so it is with science.*/

// Called every frame
void UDidItHitActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	int privateModuloSkip = 1;
	if (ModuloSkip >= 1)
		privateModuloSkip = ModuloSkip;


	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (CanTrace)
	{
		if (ModuloNumber % privateModuloSkip == 0)
		{
			if (bHitSameSocketAtDifferentTimes)
			{
				HitSameSocketAtDifferentTimes();
			}

			if (bHitOtherSocketsAtSameTime)
			{
				HitOtherSocketsAtSameTime();
			}

			if (bHitOtherSocketsAtDifferentTime)
			{
				HitOtherSocketsAtDifferentTime();
			}
		}

		UpdateLastSocketLocation();
		ModuloNumber++;
	}
}

void UDidItHitActorComponent::HitSameSocketAtDifferentTimes()
{
	/*if (UseKismet)
	{*/
		for (auto Socket : MySockets)
		{
			//Private Variables
			const FVector* Start = LastKnownSocketLocation.Find(Socket);
			const FVector End = MyPrimitive->GetSocketLocation(Socket);
			TArray<FHitResult> OutHits;

			if (Start == nullptr)
			{
				return;
			}

			if (TraceByChannelOrObjects)
			{
				switch (MyKismetTraceType)
				{
				case EKismetTraceType::LineTrace:
				{
					UKismetSystemLibrary::LineTraceMulti(MyActor, *Start, End, MyTraceChannel, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
					AddHitToHitArray(OutHits);  break;
				}

				case EKismetTraceType::BoxTrace:
				{
					UKismetSystemLibrary::BoxTraceMulti(MyActor, *Start, End, BoxHalfSize, BoxOrientation, MyTraceChannel, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
					break;
				}

				case EKismetTraceType::CapsuleTrace:
				{
					UKismetSystemLibrary::CapsuleTraceMulti(MyActor, *Start, End, CapsuleHalfHeight, CapsuleRadius, MyTraceChannel, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
					break;
				}

				case EKismetTraceType::SphereTrace:
				{
					UKismetSystemLibrary::SphereTraceMulti(MyActor, *Start, End, SphereRadius, MyTraceChannel, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
					break;
				}
				}
			}
			else if (!TraceByChannelOrObjects)
			{
				switch (MyKismetTraceType)
				{
				case EKismetTraceType::LineTrace:
				{
					UKismetSystemLibrary::LineTraceMultiForObjects(MyActor, *Start, End, MyObjectTypesToHit, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
					AddHitToHitArray(OutHits);  break;
				}

				case EKismetTraceType::BoxTrace:
				{
					UKismetSystemLibrary::BoxTraceMultiForObjects(MyActor, *Start, End, BoxHalfSize, BoxOrientation, MyObjectTypesToHit, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
					break;
				}

				case EKismetTraceType::CapsuleTrace:
				{
					UKismetSystemLibrary::CapsuleTraceMultiForObjects(MyActor, *Start, End, CapsuleHalfHeight, CapsuleRadius, MyObjectTypesToHit, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
					break;
				}

				case EKismetTraceType::SphereTrace:
				{
					UKismetSystemLibrary::SphereTraceMultiForObjects(MyActor, *Start, End, SphereRadius, MyObjectTypesToHit, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
					break;
				}
				}
			}
			AddHitToHitArray(OutHits);
		}
	//}
	/*else if (!UseKismet)
	{
		//Unfinished - requires more tweaking to optimize
		//for (auto Socket : MySockets)
		//{
		//	TArray<FHitResult> OutHits;
		//	FCollisionQueryParams TraceParams;
		//	FVector* TraceStart = LastKnownSocketLocation.Find(Socket);
		//	FVector TraceEnd = MyPrimitive->GetSocketLocation(Socket);

		//	GetWorld()->LineTraceMultiByObjectType(OutHits, *TraceStart, TraceEnd, MyObjectTypesToHit, TraceParams);
		//	
		//	GetWorld()->SweepMultiByObjectType(OutHits, *TraceStart, TraceEnd, SweepQuaternion, MyObjectTypesToHit, MyCollisionShape, TraceParams);
		//	AddHitToHitArray(OutHits);
		//}
	}*/
}

void UDidItHitActorComponent::HitOtherSocketsAtSameTime()
{
	//UE_LOG(LogTemp, Warning, TEXT("7"));
	/*if (UseKismet)
	{*/
		for (auto Socket1 : MySockets)
		{
			for (auto Socket2 : MySockets)
			{
				//Private Variables
				const FVector Start = MyPrimitive->GetSocketLocation(Socket1);
				const FVector End = MyPrimitive->GetSocketLocation(Socket2);
				TArray<FHitResult> OutHits;

				if (TraceByChannelOrObjects)
				{
					switch (MyKismetTraceType)
					{
					case EKismetTraceType::LineTrace:
					{
						UKismetSystemLibrary::LineTraceMulti(MyActor, Start, End, MyTraceChannel, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						//UKismetSystemLibrary::LineTraceMulti(MyWorldContextObject, Start, End, MyTraceChannel, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						AddHitToHitArray(OutHits); break;

					}

					case EKismetTraceType::BoxTrace:
					{
						UKismetSystemLibrary::BoxTraceMulti(MyActor, Start, End, BoxHalfSize, BoxOrientation, MyTraceChannel, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}

					case EKismetTraceType::CapsuleTrace:
					{
						UKismetSystemLibrary::CapsuleTraceMulti(MyActor, Start, End, CapsuleHalfHeight, CapsuleRadius, MyTraceChannel, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}

					case EKismetTraceType::SphereTrace:
					{
						UKismetSystemLibrary::SphereTraceMulti(MyActor, Start, End, SphereRadius, MyTraceChannel, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}
					}
				}
				else if (!TraceByChannelOrObjects)
				{
					switch (MyKismetTraceType)
					{
					case EKismetTraceType::LineTrace:
					{
						UKismetSystemLibrary::LineTraceMultiForObjects(MyActor, Start, End, MyObjectTypesToHit, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						AddHitToHitArray(OutHits); break;
					}

					case EKismetTraceType::BoxTrace:
					{
						UKismetSystemLibrary::BoxTraceMultiForObjects(MyActor, Start, End, BoxHalfSize, BoxOrientation, MyObjectTypesToHit, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}

					case EKismetTraceType::CapsuleTrace:
					{
						UKismetSystemLibrary::CapsuleTraceMultiForObjects(MyActor, Start, End, CapsuleHalfHeight, CapsuleRadius, MyObjectTypesToHit, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}

					case EKismetTraceType::SphereTrace:
					{
						UKismetSystemLibrary::SphereTraceMultiForObjects(MyActor, Start, End, SphereRadius, MyObjectTypesToHit, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}
					}
				}
				AddHitToHitArray(OutHits);
			}
		}
	//}
	/*else if (!UseKismet) {
		for (auto Socket1 : MySockets)
		{
			for (auto Socket2 : MySockets)
			{

				//Not optimized:
				//TArray<FHitResult> OutHits;
				//FCollisionQueryParams TraceParams = MyTraceParams;
				//FVector TraceStart = MyPrimitive->GetSocketLocation(Socket1);
				//FVector TraceEnd = MyPrimitive->GetSocketLocation(Socket2);
				//TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesToHit = MyObjectTypesToHit;

				//GetWorld()->LineTraceMultiByObjectType(OutHits, TraceStart, TraceEnd, ObjectTypesToHit, TraceParams);

				//AddHitToHitArray(OutHits);
			}
		}
	}*/
}

void UDidItHitActorComponent::HitOtherSocketsAtDifferentTime()
{
	/*if (UseKismet)
	{*/
		for (auto Socket1 : MySockets)
		{
			for (auto Socket2 : MySockets)
			{
				TArray<FHitResult> OutHits1;
				FVector Start = MyPrimitive->GetSocketLocation(Socket1);
				FVector* End = LastKnownSocketLocation.Find(Socket2);

				if (End == nullptr)
				{
					return;
				}

				if (TraceByChannelOrObjects)
				{
					switch (MyKismetTraceType)
					{
					case EKismetTraceType::LineTrace:
					{
						UKismetSystemLibrary::LineTraceMulti(MyActor, Start, *End, MyTraceChannel, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits1, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						AddHitToHitArray(OutHits1); break;
					}

					case EKismetTraceType::BoxTrace:
					{
						UKismetSystemLibrary::BoxTraceMulti(MyActor, Start, *End, BoxHalfSize, BoxOrientation, MyTraceChannel, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits1, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}

					case EKismetTraceType::CapsuleTrace:
					{
						UKismetSystemLibrary::CapsuleTraceMulti(MyActor, Start, *End, CapsuleHalfHeight, CapsuleRadius, MyTraceChannel, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits1, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}

					case EKismetTraceType::SphereTrace:
					{
						UKismetSystemLibrary::SphereTraceMulti(MyActor, Start, *End, SphereRadius, MyTraceChannel, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits1, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}
					}
				}
				else if (!TraceByChannelOrObjects)
				{
					switch (MyKismetTraceType)
					{
					case EKismetTraceType::LineTrace:
					{
						UKismetSystemLibrary::LineTraceMultiForObjects(MyActor, Start, *End, MyObjectTypesToHit, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits1, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						AddHitToHitArray(OutHits1); break;
					}

					case EKismetTraceType::BoxTrace:
					{
						UKismetSystemLibrary::BoxTraceMultiForObjects(MyActor, Start, *End, BoxHalfSize, BoxOrientation, MyObjectTypesToHit, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits1, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}

					case EKismetTraceType::CapsuleTrace:
					{
						UKismetSystemLibrary::CapsuleTraceMultiForObjects(MyActor, Start, *End, CapsuleHalfHeight, CapsuleRadius, MyObjectTypesToHit, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits1, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}

					case EKismetTraceType::SphereTrace:
					{
						UKismetSystemLibrary::SphereTraceMultiForObjects(MyActor, Start, *End, SphereRadius, MyObjectTypesToHit, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits1, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}
					}
				}

				AddHitToHitArray(OutHits1);
			}

			for (auto Socket3 : MySockets)
			{
				TArray<FHitResult> OutHits2;
				FVector* Start = LastKnownSocketLocation.Find(Socket3);
				FVector End = MyPrimitive->GetSocketLocation(Socket1);

				if (TraceByChannelOrObjects)
				{
					switch (MyKismetTraceType)
					{
					case EKismetTraceType::LineTrace:
					{
						UKismetSystemLibrary::LineTraceMulti(MyActor, *Start, End, MyTraceChannel, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits2, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						AddHitToHitArray(OutHits2); break;
					}

					case EKismetTraceType::BoxTrace:
					{
						UKismetSystemLibrary::BoxTraceMulti(MyActor, *Start, End, BoxHalfSize, BoxOrientation, MyTraceChannel, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits2, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}

					case EKismetTraceType::CapsuleTrace:
					{
						UKismetSystemLibrary::CapsuleTraceMulti(MyActor, *Start, End, CapsuleHalfHeight, CapsuleRadius, MyTraceChannel, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits2, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}

					case EKismetTraceType::SphereTrace:
					{
						UKismetSystemLibrary::SphereTraceMulti(MyActor, *Start, End, SphereRadius, MyTraceChannel, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits2, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}
					}
				}
				else if (!TraceByChannelOrObjects)
				{
					switch (MyKismetTraceType)
					{
					case EKismetTraceType::LineTrace:
					{
						UKismetSystemLibrary::LineTraceMultiForObjects(MyActor, *Start, End, MyObjectTypesToHit, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits2, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						AddHitToHitArray(OutHits2); break;
					}

					case EKismetTraceType::BoxTrace:
					{
						UKismetSystemLibrary::BoxTraceMultiForObjects(MyActor, *Start, End, BoxHalfSize, BoxOrientation, MyObjectTypesToHit, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits2, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}

					case EKismetTraceType::CapsuleTrace:
					{
						UKismetSystemLibrary::CapsuleTraceMultiForObjects(MyActor, *Start, End, CapsuleHalfHeight, CapsuleRadius, MyObjectTypesToHit, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits2, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}

					case EKismetTraceType::SphereTrace:
					{
						UKismetSystemLibrary::SphereTraceMultiForObjects(MyActor, *Start, End, SphereRadius, MyObjectTypesToHit, ShouldTraceComplex, MyActorsToIgnore, MyDrawDebugType, OutHits2, ShouldIgnoreSelf, MyTraceColor, MyTraceHitColor, MyDrawTime);
						break;
					}
					}
				}

				AddHitToHitArray(OutHits2);
			}
		}
	//}
	/*else if (!UseKismet)
	{
		for (auto Socket1 : MySockets)
		{
			for (auto Socket2 : MySockets)
			{
				TArray<FHitResult> OutHits1;
				FCollisionQueryParams TraceParams;
				FVector TraceStart = MyPrimitive->GetSocketLocation(Socket1);
				FVector* TraceEnd = LastKnownSocketLocation.Find(Socket2);

				GetWorld()->LineTraceMultiByObjectType(OutHits1, TraceStart, *TraceEnd, MyObjectTypesToHit, TraceParams);

				AddHitToHitArray(OutHits1);
			}

			for (auto Socket3 : MySockets)
			{
				TArray<FHitResult> OutHits2;
				FCollisionQueryParams TraceParams;
				FVector* TraceStart = LastKnownSocketLocation.Find(Socket3);
				FVector TraceEnd = MyPrimitive->GetSocketLocation(Socket1);

				GetWorld()->LineTraceMultiByObjectType(OutHits2, *TraceStart, TraceEnd, MyObjectTypesToHit, TraceParams);

				AddHitToHitArray(OutHits2);
			}
		}
	}*/
}

void UDidItHitActorComponent::GetPrimaryComponent(AActor* ActorTarget)
{
	if (MyStaticMeshComponent)
	{
		MyPrimitive = MyStaticMeshComponent;
		return;
	}

	auto taggedComponents = ActorTarget->GetComponentsByTag(UStaticMeshComponent::StaticClass(), "DidItHit?");
	if (taggedComponents.Num() > 0)
	{
		MyPrimitive = Cast<UPrimitiveComponent>(taggedComponents[0]);
		return;
	}

	auto meshComponent = ActorTarget->GetComponentByClass(UStaticMeshComponent::StaticClass());
	if (meshComponent)
	{
		MyPrimitive = Cast<UPrimitiveComponent>(meshComponent);
		return;
	}
}

TArray<FName> UDidItHitActorComponent::SetupVariables(UPrimitiveComponent* MyPrimitiveTarget, AActor* MyActorTarget)
{
	TArray<FName> MyFoundSockets;
	MyPrimitive = MyPrimitiveTarget;
	MyActor = MyActorTarget;

	if (!MyActor)
	{
		MyActor = GetOwner();
	}

	GetSockets();
	MyFoundSockets = MySockets;
	return MyFoundSockets;
}

TArray<FName> UDidItHitActorComponent::GetSockets()
{
	//TArray<FName> MySockets = MySockets;
	if (MyPrimitive)
	{
		MySockets = MyPrimitive->GetAllSocketNames();

		//remove sockets you don't want
		//if (SkipStringFilter != "")
		//{
		//	for (auto Socket : MySockets)
		//	{
		//		if (Socket.ToString().Contains(SkipStringFilter))
		//		{
		//			MySockets.Remove(Socket);
		//		}
		//	}
		//}
		//the following was a more reliable solution provided by Godman on October 13th, 2023
		if (SkipStringFilter != "")
		{
			for (int32 Index = MySockets.Num() - 1; Index >= 0; --Index)
			{
				if (MySockets[Index].ToString().Contains(SkipStringFilter))
				{
					MySockets.RemoveAt(Index);
				}
			}
		}

		//remove sockets that aren't included
		//if (InclusionStringFilter != "")
		//{
		//	for (auto Socket : MySockets)
		//	{
		//		if (!Socket.ToString().Contains(InclusionStringFilter))
		//		{
		//			MySockets.Remove(Socket);
		//		}
		//	}
		//}
		//the following was a more reliable solution provided by Godman on October 13th, 2023
		if (InclusionStringFilter != "")
		{
			for (int32 Index = MySockets.Num() - 1; Index >= 0; --Index)
			{
				if (!MySockets[Index].ToString().Contains(InclusionStringFilter))
				{
					MySockets.RemoveAt(Index);
				}
			}
		}
	}

	return MySockets;
}

void UDidItHitActorComponent::ToggleTraceCheck(bool bTrace)
{
	if (bTrace)
	{
		ClearHitArray();
		ClearSocketLocationMap();
		GetSocket_t0();
		ModuloNumber = 0;
	}

	CanTrace = bTrace;
}

void UDidItHitActorComponent::UpdateLastSocketLocation()
{
	if (MySockets.Num() > 0) {

		for (auto Socket : MySockets)
		{
			LastKnownSocketLocation.Add(Socket, MyPrimitive->GetSocketLocation(Socket));
		}
	}
}

void UDidItHitActorComponent::ClearHitArray()
{
	HitArray.Empty();
}

void UDidItHitActorComponent::ClearSocketLocationMap()
{
	LastKnownSocketLocation.Empty();
}

void UDidItHitActorComponent::GetSocket_t0()
{
	for (auto Socket : MySockets)
	{
		LastKnownSocketLocation.Add(Socket, MyPrimitive->GetSocketLocation(Socket));
	}
}

void UDidItHitActorComponent::AddHitToHitArray(TArray<FHitResult> HitArrayToAdd)
{
	for (const auto& Hit : HitArrayToAdd)
	{
		if (!HitArray.ContainsByPredicate([&](const FHitResult& Inner) {  return Inner.GetActor() == Hit.GetActor(); }))
		{
			HitArray.Add(Hit);

			OnItemAdded.Broadcast(Hit);
		}
	}
}

//FHitResult OnHitAdded(FHitResult LastHit)
//{
//	return LastHit;
//}

/*
https://docs.unrealengine.com/4.26/en-US/API/Runtime/Engine/Kismet/UKismetSystemLibrary/LineTraceSingle/
*/

/*You can know the name of a bird in all the languages of the world,
but when you're finished,
you'll know absolutely nothing whatever about the bird...
So let's look at the bird and see what it's doing —
that's what counts.
I learned very early the difference between knowing the name of something and knowing something.
*/