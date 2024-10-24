// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemy.h"
#include "Engine/StaticMeshActor.h"

//#include "Engine/SkeletalMesh.h"
//#include "Engine/StaticMesh.h"
//#include "Engine/World.h"
//#include "Components/StaticMeshComponent.h"
//#include "Components/SkeletalMeshComponent.h"

//CLEAN THE INCLUEDES

#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Animation/AnimInstance.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Components/SkeletalMeshComponent.h"
#include "Rendering/MultiSizeIndexContainer.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "ProceduralMeshComponent.h"

#include "../CombatSystemComponents/CombatSystemComponent.h"


#define PRINT(mess, mtime)  GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::Green, TEXT(mess));
#define PRINTC(mess, color)  GEngine->AddOnScreenDebugMessage(-1, 3, color, TEXT(mess));
#define PRINT_F(prompt, mess, mtime) GEngine->AddOnScreenDebugMessage(-1, mtime, FColor::Green, FString::Printf(TEXT(prompt), mess));
#define PRINTC_F(prompt, mess, mtime, color) GEngine->AddOnScreenDebugMessage(-1, mtime, color, FString::Printf(TEXT(prompt), mess));
#define PRINT_B(prompt, mess) GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Green, FString::Printf(TEXT(prompt), mess ? TEXT("TRUE") : TEXT("FALSE")));


// Sets default values
ABaseEnemy::ABaseEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
    ProceduralMesh->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	
    ProceduralMesh->SetRelativeTransform(GetMesh()->GetRelativeTransform());
}

// Called every frame
void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ABaseEnemy::HandleHitReaction(const FCombatHitData& CombatHitData)
{
	if (!bIsGettingHit)
	{
		ApplyDamage(CombatHitData);

		//---------------------------------------------- previous dismemberment solution ---------------------------------------------- 
		GetMesh()->HideBoneByName(HeadBoneName, EPhysBodyOp::PBO_None);

		AStaticMeshActor* SpawnedHead = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass());
		SpawnedHead->SetMobility(EComponentMobility::Movable);

		SpawnedHead->SetActorTransform(GetMesh()->GetBoneTransform(HeadBoneName));

		auto Location = SpawnedHead->GetActorLocation();
		Location.Z += 20.f;
		SpawnedHead->SetActorLocation(Location);

		SpawnedHead->SetActorScale3D(FVector(1.4f));

        FVector HeadImpulse = (CombatHitData.CutPlaneNormal + CombatHitData.CutVelocity).GetSafeNormal() * DismembermentStrength;

		UStaticMeshComponent* MeshComponent = SpawnedHead->GetStaticMeshComponent();
		if (MeshComponent)
		{
			MeshComponent->SetStaticMesh(HeadMesh);
			MeshComponent->SetSimulatePhysics(true);
			MeshComponent->SetEnableGravity(true);
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
			MeshComponent->AddImpulse(HeadImpulse);
			MeshComponent->SetAllMassScale(1);
		}
		//---------------------------------------------- previous dismemberment solution ---------------------------------------------- 

		bIsGettingHit = true;
		return false;
	}

	return true;
}

void ABaseEnemy::CopySkeletalMeshToProcedural(USkeletalMeshComponent* SkeletalMeshComponent, int32 LODIndex, UProceduralMeshComponent* ProcMeshComponent)
{
    PRINTC("using c++ CopySkeletalMeshToProcedural function", FColor::Red);

    FSkeletalMeshRenderData* SkMeshRenderData = SkeletalMeshComponent->GetSkeletalMeshRenderData();
    const FSkeletalMeshLODRenderData& DataArray = SkMeshRenderData->LODRenderData[LODIndex];
    FSkinWeightVertexBuffer& SkinWeights = *SkeletalMeshComponent->GetSkinWeightBuffer(LODIndex);

    TArray<FVector> VerticesArray;
    TArray<FVector> Normals;
    TArray<FVector2D> UV;
    TArray<FColor> Colors;
    TArray<FProcMeshTangent> Tangents;

    for (int32 j = 0; j < DataArray.RenderSections.Num(); j++)
    {
        //get num vertices and offset 
        const int32 NumSourceVertices = DataArray.RenderSections[j].NumVertices;
        const int32 BaseVertexIndex = DataArray.RenderSections[j].BaseVertexIndex;

        for (int32 i = 0; i < NumSourceVertices; i++)
        {
            const int32 VertexIndex = i + BaseVertexIndex;

            //get skinned vector positions
            const FVector3f SkinnedVectorPos = USkeletalMeshComponent::GetSkinnedVertexPosition(
                SkeletalMeshComponent, VertexIndex, DataArray, SkinWeights);
            VerticesArray.Add(FVector(SkinnedVectorPos));

            //Calc normals and tangents from the static version instead of the skeletal one
            const FVector3f ZTangentStatic = DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(
                VertexIndex);
            const FVector3f XTangentStatic = DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentX(
                VertexIndex);

            //add normals from the static mesh version instead because using the skeletal one doesnt work right.
            Normals.Add(FVector(ZTangentStatic));

            //add tangents
            Tangents.Add(FProcMeshTangent(FVector(XTangentStatic), false));

            //get UVs
            const FVector2f SourceUVs = DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.
                GetVertexUV(VertexIndex, 0);
            FVector2d ResUVs;
            ResUVs.X = SourceUVs.X;
            ResUVs.Y = SourceUVs.Y;
            UV.Add(ResUVs);

            //dummy vertex colors
            Colors.Add(FColor(0.0, 0.0, 0.0, 255));
        }
    }

    //get index buffer
    FMultiSizeIndexContainerData IndicesData;
    DataArray.MultiSizeIndexContainer.GetIndexBuffer(IndicesData.Indices);


    for (int32 j = 0; j < DataArray.RenderSections.Num(); j++)
    {
        TArray<int32> Tris;

        // get number triangles and offset
        const int32 SectionNumTriangles = DataArray.RenderSections[j].NumTriangles;
        const int32 SectionBaseIndex = DataArray.RenderSections[j].BaseIndex;

        //iterate over num indices and add traingles
        for (int32 i = 0; i < SectionNumTriangles; i++)
        {
            int32 TriVertexIndex = i * 3 + SectionBaseIndex;
            Tris.Add(IndicesData.Indices[TriVertexIndex]);
            Tris.Add(IndicesData.Indices[TriVertexIndex + 1]);
            Tris.Add(IndicesData.Indices[TriVertexIndex + 2]);
        }

        //Create the procedural mesh section
        ProcMeshComponent->CreateMeshSection(j, VerticesArray, Tris, Normals, UV, Colors, Tangents, false);
    }
}

void ABaseEnemy::ConvertSkeletalMeshToProceduralMesh(USkeletalMeshComponent* InSkeletalMeshComponent, int32 LODIndex, UProceduralMeshComponent* InProcMeshComponent)
{
    PRINTC("using c++ ConvertSkeletalMeshToProceduralMesh function", FColor::Green);

    FSkeletalMeshRenderData* SkMeshRenderData = InSkeletalMeshComponent->GetSkeletalMeshRenderData();
    const FSkeletalMeshLODRenderData& DataArray = SkMeshRenderData->LODRenderData[LODIndex];
    FSkinWeightVertexBuffer& SkinWeights = *InSkeletalMeshComponent->GetSkinWeightBuffer(LODIndex);

    TArray<FVector> VerticesArray;
    TArray<FVector> Normals;
    TArray<FVector2D> UV;
    TArray<FColor> Colors;
    TArray<FProcMeshTangent> Tangents;

    for (int32 j = 0; j < DataArray.RenderSections.Num(); j++)
    {
        //get num vertices and offset 
        const int32 NumSourceVertices = DataArray.RenderSections[j].NumVertices;
        const int32 BaseVertexIndex = DataArray.RenderSections[j].BaseVertexIndex;

        for (int32 i = 0; i < NumSourceVertices; i++)
        {
            const int32 VertexIndex = i + BaseVertexIndex;

            //get skinned vector positions
            const FVector3f SkinnedVectorPos = USkeletalMeshComponent::GetSkinnedVertexPosition(
                InSkeletalMeshComponent, VertexIndex, DataArray, SkinWeights);
            //VerticesArray.Add(fromFVector3f(SkinnedVectorPos));
            VerticesArray.Add(FVector(SkinnedVectorPos.X, SkinnedVectorPos.Y, SkinnedVectorPos.Z));

            //Calc normals and tangents from the static version instead of the skeletal one
            const FVector3f ZTangentStatic = DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(
                VertexIndex);
            const FVector3f XTangentStatic = DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentX(
                VertexIndex);

            //add normals from the static mesh version instead because using the skeletal one doesnt work right.
            //Normals.Add(fromFVector3f(ZTangentStatic));
            Normals.Add(FVector(ZTangentStatic.X, ZTangentStatic.Y, ZTangentStatic.Z));

            //add tangents
            //Tangents.Add(FProcMeshTangent(fromFVector3f(XTangentStatic), false));
            Tangents.Add(FProcMeshTangent(FVector(XTangentStatic.X, XTangentStatic.Y, XTangentStatic.Z), false));

            //get UVs
            const FVector2f SourceUVs = DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.
                GetVertexUV(VertexIndex, 0);
            FVector2d ResUVs;
            ResUVs.X = SourceUVs.X;
            ResUVs.Y = SourceUVs.Y;
            UV.Add(ResUVs);

            //dummy vertex colors
            Colors.Add(FColor(0.0, 0.0, 0.0, 255));
        }
    }

    //get index buffer
    FMultiSizeIndexContainerData IndicesData;
    DataArray.MultiSizeIndexContainer.GetIndexBuffer(IndicesData.Indices);


    for (int32 j = 0; j < DataArray.RenderSections.Num(); j++)
    {
        TArray<int32> Tris;

        // get number triangles and offset
        const int32 SectionNumTriangles = DataArray.RenderSections[j].NumTriangles;
        const int32 SectionBaseIndex = DataArray.RenderSections[j].BaseIndex;

        //iterate over num indices and add triangles
        for (int32 i = 0; i < SectionNumTriangles; i++)
        {
            int32 TriVertexIndex = i * 3 + SectionBaseIndex;
            Tris.Add(IndicesData.Indices[TriVertexIndex]);
            Tris.Add(IndicesData.Indices[TriVertexIndex + 1]);
            Tris.Add(IndicesData.Indices[TriVertexIndex + 2]);
        }

        //Create the procedural mesh section
        InProcMeshComponent->CreateMeshSection(j, VerticesArray, Tris, Normals, UV, Colors, Tangents, false);
    }
}

// Called to bind functionality to input
//void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
//{
//	Super::SetupPlayerInputComponent(PlayerInputComponent);
//
//}

