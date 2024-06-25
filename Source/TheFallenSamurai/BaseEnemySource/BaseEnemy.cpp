// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemy.h"
#include "Engine/StaticMeshActor.h"

//#include "Engine/SkeletalMesh.h"
//#include "Engine/StaticMesh.h"
//#include "Engine/World.h"
//#include "Components/StaticMeshComponent.h"
//#include "Components/SkeletalMeshComponent.h"

#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Animation/AnimInstance.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#include "Rendering/SkeletalMeshModel.h"
//#include "AssetRegistryModule.h"

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

		//---------------------------------------------- previous dismemberment solution ---------------------------------------------- 
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

		//---------------------------------------------- mesh slicing solution ---------------------------------------------- 
        //ConvertAndSpawnStaticMeshFromPose(GetWorld(), GetMesh(), GetActorTransform());
        /*ConvertAndSpawnStaticMesh(GetWorld(), GetMesh()->GetSkeletalMeshAsset(), GetActorTransform());
        GetMesh()->SetVisibility(false, false);*/

		bIsGettingHit = true;
		return false;
	}

	return true;
}

void ABaseEnemy::ConvertAndSpawnStaticMesh(UWorld* World, USkeletalMesh* SkeletalMesh, const FTransform& Transform)
{
    if (!SkeletalMesh || !World) return;

    // Create a new static mesh
    UStaticMesh* NewStaticMesh = NewObject<UStaticMesh>();

    // Set up the new static mesh with the same materials as the skeletal mesh
    //NewStaticMesh->StaticMaterials = SkeletalMesh->GetMaterials();

    // Extract LOD0 data
    FMeshDescription MeshDescription;
    SkeletalMesh->GetMeshDescription(0, MeshDescription);

    NewStaticMesh->CreateMeshDescription(0, MeshDescription);

    // Register the new mesh
    NewStaticMesh->PostEditChange();

    // Spawn the static mesh actor in the game world
    AStaticMeshActor* NewActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Transform);
    if (NewActor)
    {
        UStaticMeshComponent* MeshComponent = NewActor->GetStaticMeshComponent();
        if (MeshComponent)
        {
            MeshComponent->SetStaticMesh(NewStaticMesh);
        }
    }
}


UStaticMesh* ABaseEnemy::CreateStaticMeshFromSkeletalMeshPose(USkeletalMeshComponent* SkeletalMeshComponent)
{
    if (!SkeletalMeshComponent || !SkeletalMeshComponent->SkeletalMesh) return nullptr;

    USkeletalMesh* SkeletalMesh = SkeletalMeshComponent->SkeletalMesh;
    const FSkeletalMeshRenderData* RenderData = SkeletalMesh->GetResourceForRendering();
    if (!RenderData) return nullptr;

    const FSkeletalMeshLODRenderData& LODData = RenderData->LODRenderData[0];
    const FSkeletalMeshLODModel& LODModel = SkeletalMesh->GetImportedModel()->LODModels[0];

    // Create a new static mesh
    UStaticMesh* NewStaticMesh = NewObject<UStaticMesh>();

    // Set up the new static mesh with the same materials as the skeletal mesh
    //NewStaticMesh->StaticMaterials = SkeletalMesh->GetMaterials();

    // Create a new MeshDescription
    FMeshDescription MeshDescription;
    FStaticMeshAttributes Attributes(MeshDescription);
    Attributes.Register();

    // Create vertex positions
    TVertexAttributesRef<FVector3f> VertexPositionsRef = Attributes.GetVertexPositions();
    TVertexInstanceAttributesRef<FVector3f> Normals = Attributes.GetVertexInstanceNormals();
    TVertexInstanceAttributesRef<FVector2f> UVs = Attributes.GetVertexInstanceUVs();
    UVs.SetNumChannels(1);

    // Get current bone transforms
    const TArray<FTransform>& BoneTransforms = SkeletalMeshComponent->GetBoneSpaceTransforms();

    // Map to hold created vertex instances
    TMap<int32, FVertexInstanceID> VertexInstanceMap;

    for (const FSkelMeshSection& Section : LODModel.Sections)
    {
        FPolygonGroupID PolygonGroupID = MeshDescription.CreatePolygonGroup();

        for (uint32 TriangleIndex = 0; TriangleIndex < Section.NumTriangles; ++TriangleIndex)
        {
            TArray<FVertexInstanceID> TriangleVertexInstanceIDs;

            for (int32 Corner = 0; Corner < 3; ++Corner)
            {
                int32 IndexBufferIndex = Section.BaseIndex + TriangleIndex * 3 + Corner;

                if (IndexBufferIndex >= LODModel.IndexBuffer.Num())
                    continue;

                int32 VertexIndex = LODModel.IndexBuffer[IndexBufferIndex];

                if (!VertexInstanceMap.Contains(VertexIndex))
                {
                    if (VertexIndex >= Section.SoftVertices.Num())
                        continue;

                    const FSoftSkinVertex& Vertex = Section.SoftVertices[VertexIndex];

                    // Transform vertex position by bone transform
                    FVector TransformedPosition = FVector::ZeroVector;
                    for (uint32 InfluenceIndex = 0; InfluenceIndex < MAX_TOTAL_INFLUENCES; ++InfluenceIndex)
                    {
                        int32 BoneIndex = Vertex.InfluenceBones[InfluenceIndex];
                        if (BoneIndex >= 0)
                        {
                            const FTransform& BoneTransform = BoneTransforms[BoneIndex];
                            TransformedPosition += BoneTransform.TransformPosition(FVector(Vertex.Position)) * Vertex.InfluenceWeights[InfluenceIndex] / 255.0f;
                        }
                    }

                    FVertexID VertexID = MeshDescription.CreateVertex();
                    VertexPositionsRef[VertexID] = FVector3f(TransformedPosition);

                    FVertexInstanceID VertexInstanceID = MeshDescription.CreateVertexInstance(VertexID);
                    Normals[VertexInstanceID] = Vertex.TangentZ;
                    UVs.Set(VertexInstanceID, 0, FVector2f(Vertex.UVs[0].X, Vertex.UVs[0].Y));
                    VertexInstanceMap.Add(VertexIndex, VertexInstanceID);
                }

                TriangleVertexInstanceIDs.Add(VertexInstanceMap[VertexIndex]);
            }

            MeshDescription.CreatePolygon(PolygonGroupID, TriangleVertexInstanceIDs);
        }
    }

    // Add the mesh description to the static mesh
    NewStaticMesh->CreateMeshDescription(0, MeshDescription);
    NewStaticMesh->CommitMeshDescription(0);

    // Register the new mesh
    NewStaticMesh->PostEditChange();

    return NewStaticMesh;
}

void ABaseEnemy::ConvertAndSpawnStaticMeshFromPose(UWorld* World, USkeletalMeshComponent* SkeletalMeshComponent, const FTransform& Transform)
{
    if (!SkeletalMeshComponent || !World) return;

    // Create a static mesh from the current pose
    UStaticMesh* StaticMesh = CreateStaticMeshFromSkeletalMeshPose(SkeletalMeshComponent);

    if (StaticMesh)
    {
        // Spawn the static mesh actor in the game world
        AStaticMeshActor* NewActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Transform);
        if (NewActor)
        {
            UStaticMeshComponent* MeshComponent = NewActor->GetStaticMeshComponent();
            if (MeshComponent)
            {
                MeshComponent->SetStaticMesh(StaticMesh);
            }
        }
    }
}

// Called to bind functionality to input
//void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
//{
//	Super::SetupPlayerInputComponent(PlayerInputComponent);
//
//}

