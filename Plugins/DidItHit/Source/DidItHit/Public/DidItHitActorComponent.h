// Copyright 2021, Project Zero. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include "DidItHitActorComponent.generated.h"

/*There are 10^11 stars in the galaxy.
That used to be a huge number.
But it's only a hundred billion.
It's less than the national deficit!
We used to call them astronomical numbers.
Now we should call them economical numbers.*/

UENUM(BlueprintType)
enum class EKismetTraceType : uint8
{
	LineTrace			UMETA(DisplayName = "Line Trace"),
	BoxTrace			UMETA(DisplayName = "Box Trace"),
	CapsuleTrace		UMETA(DisplayName = "Capsule Trace"),
	SphereTrace			UMETA(DisplayName = "Sphere Trace")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemAdded, FHitResult, LastItem);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent, DisplayName = "AC_DidItHit"))
class DIDITHIT_API UDidItHitActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "AC_DidItHit")
		FOnItemAdded OnItemAdded;


	// Sets default values for this component's properties
	UDidItHitActorComponent();

	//Required to determine sockets locations
	UPROPERTY()
		class UStaticMeshComponent* MyStaticMeshComponent;

	//Required to determine sockets locations
	UPROPERTY()
		class UPrimitiveComponent* MyPrimitive;

	//Fallback in case no primitive or staticmesh comp is available
	UPROPERTY()
		class AActor* MyActor;

	//The whole point of the plugin
	UPROPERTY()
		TArray<FName> MySockets;

	//To keep sockets in memory and their previous locations
	UPROPERTY()
		TMap<FName, FVector> LastKnownSocketLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Method, meta = (ToolTip = "If true, will trace between the same socket at different times"))
		bool bHitSameSocketAtDifferentTimes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Method, meta = (ToolTip = "If true, will trace between the different sockets at the same time"))
		bool bHitOtherSocketsAtSameTime = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Method, meta = (ToolTip = "If true, will trace between the different socket at different times"))
		bool bHitOtherSocketsAtDifferentTime = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Method, meta = (ToolTip = "Modulo skip factor. Can be used to skip ticks."))
		int ModuloSkip = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Method, meta = (ToolTip = "String filter to skip socket names containing this string"))
		FString SkipStringFilter = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Method, meta = (ToolTip = "String filter to only include socket names containing this string."))
		FString InclusionStringFilter = "";

	UPROPERTY()
		int ModuloNumber = 0;

	//Return value of the plugin
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Result, meta = (ToolTip = "Return Value for the plugin. It is a raw hit struct and can be broken down further."))
		TArray<FHitResult> HitArray;

	//Boolean used during Tick
	UPROPERTY()
		bool CanTrace = false;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	//	bool bDebugMode = false;

#pragma region WorldTrace Variables

	////This space is reserved for future enhancement using the GetWorld Trace method.

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WorldTraceSettings)
	//	FQuat SweepQuaternion;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WorldTraceSettings)
	//	FCollisionShape	MyCollisionShape;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WorldTraceSettings)
	//	FCollisionQueryParams MyTraceParams;
#pragma endregion

#pragma region Kismet Variables

//Trace Settings Kismet
	//UPROPERTY(Category = KismetTraceSettings, meta = (Tooltip = "Kismet will use the regular nodes in blueprints however may be more costly. Great for debugging though!"))
	//	bool UseKismet = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KismetTraceSettings, meta = (Tooltip = "If True, will trace by channel; otherwise will trace for objects."))
		bool TraceByChannelOrObjects = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KismetTraceSettings, meta = (Tooltip = "In case you use BoxTrace."))
		FVector BoxHalfSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KismetTraceSettings, meta = (Tooltip = "In case you use BoxTrace."))
		FRotator BoxOrientation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KismetTraceSettings, meta = (Tooltip = "In case you use CapsuleTrace."))
		float CapsuleRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KismetTraceSettings, meta = (Tooltip = "In case you use CapsuleTrace."))
		float CapsuleHalfHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KismetTraceSettings, meta = (Tooltip = "In case you use SphereTrace"))
		float SphereRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KismetTraceSettings, meta = (Tooltip = "Allows to change from the regular line trace."))
		EKismetTraceType MyKismetTraceType = EKismetTraceType::LineTrace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KismetTraceSettings, meta = (Tooltip = "In case you trace by channel."))
		TEnumAsByte<ETraceTypeQuery> MyTraceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KismetTraceSettings, meta = (Tooltip = "In case you trace for objects (array)."))
		TArray<TEnumAsByte<EObjectTypeQuery>> MyObjectTypesToHit = { UEngineTypes::ConvertToObjectType(ECC_WorldStatic) , UEngineTypes::ConvertToObjectType(ECC_WorldDynamic) };

	//Not required yet.
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TraceSettings, meta = (Tooltip = "In case you trace for objects (single)."))
	//	TEnumAsByte<EObjectTypeQuery> MyObjectTypeToHit = UEngineTypes::ConvertToObjectType(ECC_WorldDynamic);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KismetTraceSettings, meta = (Tooltip = "True means trace complex, false means trace simple collisions."))
		bool ShouldTraceComplex = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KismetTraceSettings, meta = (Tooltip = "Actors to ignore (array)."))
		TArray<AActor*> MyActorsToIgnore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KismetTraceSettings, meta = (Tooltip = "True means it will ignore self."))
		bool ShouldIgnoreSelf = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KismetTraceSettings, meta = (Tooltip = "Custom duration for draw time."))
		float MyDrawTime = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KismetTraceSettings, meta = (Tooltip = "Color when tracing but not hitting."))
		FLinearColor MyTraceColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KismetTraceSettings, meta = (Tooltip = "Color when hitting."))
		FLinearColor MyTraceHitColor = FLinearColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KismetTraceSettings, meta = (Tooltip = "How you want to debug the trace if at all."))
		TEnumAsByte<EDrawDebugTrace::Type> MyDrawDebugType = EDrawDebugTrace::Type::ForDuration;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KismetTraceSettings, meta = (Tooltip = "Set by default to this. Do not change unless you know what it does."))
	//	UObject* MyWorldContextObject = this; //this can be anything according to the documentation. best proposed solution is either "this" as in the actor component, or "getowner" as in the container.
#pragma endregion


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/////////////
	//Does a trace check with the same socket at different frames
	virtual void HitSameSocketAtDifferentTimes();

	//Does a trace check between all sockets at the same time
	virtual void HitOtherSocketsAtSameTime();

	//Does a trace check between all sockets at different times
	virtual void HitOtherSocketsAtDifferentTime();
	//////////////


	////////////
	//A Blueprint callable event to be set in the constructor of the owner to setup the actor or weapon to derive the primitive and therefore the sockets
	UFUNCTION(BlueprintCallable, Category = "Did It Hit?")
		TArray<FName> SetupVariables(UPrimitiveComponent* MyPrimitiveTarget, AActor* MyActorTarget);
	////////////

	////////////
	//Updates a map of the socket names and the vector location
	UFUNCTION()
		virtual void UpdateLastSocketLocation();

	//Gets the primitive component
	UFUNCTION()
		virtual void GetPrimaryComponent(AActor* ActorTarget);

	//Gets the sockets
	UFUNCTION()
		TArray<FName> GetSockets();

	UFUNCTION()
		virtual void AddHitToHitArray(TArray<FHitResult> HitArrayToAdd);
	////////////

	////////////
	//Turns on or off the trace on tick and resets variables when toggled back on
public:
	UFUNCTION(BlueprintCallable, Category = "Did It Hit?")
		void ToggleTraceCheck(bool CanTraceInput);

	//A function to clear the hit array
public:
	UFUNCTION(BlueprintCallable, Category = "Did It Hit?")
		virtual void ClearHitArray();

	//A function to clear the socket and location map
public:
	UFUNCTION(BlueprintCallable, Category = "Did It Hit?")
		virtual void ClearSocketLocationMap();

	//A function to get the sockets' positions at t=0. Required prior to calling any delta T trace.
public:
	UFUNCTION()
		virtual void GetSocket_t0();
	////////////

};

/*I don't know what's the matter with people:
they don't learn by understanding,
they learn by some other way —
by rote or something.
Their knowledge is so fragile!*/