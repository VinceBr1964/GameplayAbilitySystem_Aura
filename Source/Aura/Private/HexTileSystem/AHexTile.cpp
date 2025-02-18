// Copyright Vince Bracken


#include "HexTileSystem/AHexTile.h"
#include "DrawDebugHelpers.h"
#include <Navigation/GridPathFollowingComponent.h>
#include <PLayer/AuraPlayerController.h>
#include "EngineUtils.h" // Include this at the top of your file
#include "Components/StaticMeshComponent.h"
#include "HexTileSystem/HexGridManager.h" // Ensure you include the manager header
#include "Materials/MaterialInstance.h"
#include "UObject/ConstructorHelpers.h"
#include <Materials/MaterialInstanceConstant.h>

// Sets default values
AHexTile::AHexTile()
{
    PrimaryActorTick.bCanEverTick = false;

    // Enable input events
    SetActorEnableCollision(true);  // Ensure collision is enabled
    bGenerateOverlapEventsDuringLevelStreaming = true;

    // Enable mouse interaction
    AutoReceiveInput = EAutoReceiveInput::Player0; // Allow player input

    // Create and attach the mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HexMesh"));
    RootComponent = MeshComponent;

    FVector WorldLoc = GetActorLocation();
    UE_LOG(LogTemp, Warning, TEXT("HexTile Created at Location: %s"), *WorldLoc.ToString());

    // Debugging to ensure no duplicates
    static int HexCounter = 0;
    HexCounter++;
    UE_LOG(LogTemp, Warning, TEXT("Total HexTiles Spawned: %d"), HexCounter);

    // Make sure the MeshComponent receives input events
    MeshComponent->SetGenerateOverlapEvents(true);
    MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    // Load different terrain materials
    static ConstructorHelpers::FObjectFinder<UMaterialInstance> GrassMat(
        TEXT("MaterialInstanceConstant'/Game/Assets/HexGrid/M_HexTile_Grassland_Instance'")
    );
    static ConstructorHelpers::FObjectFinder<UMaterialInstance> MountainMat(
        TEXT("MaterialInstanceConstant'/Game/Assets/HexGrid/M_HexTile_Mountain_Instance'")
    );
    static ConstructorHelpers::FObjectFinder<UMaterialInstance> WaterMat(
        TEXT("MaterialInstanceConstant'/Game/Assets/HexGrid/M_HexTile_Water_Instance'")
    );
    static ConstructorHelpers::FObjectFinder<UMaterialInstance> DesertMat(
        TEXT("MaterialInstanceConstant'/Game/Assets/HexGrid/M_HexTile_Desert_Instance'")
    );

    if (GrassMat.Succeeded()) GrassMaterial = GrassMat.Object;
    if (MountainMat.Succeeded()) MountainMaterial = MountainMat.Object;
    if (WaterMat.Succeeded()) WaterMaterial = WaterMat.Object;
    if (DesertMat.Succeeded()) DesertMaterial = DesertMat.Object;

    // Assign a random hex type
    SetHexType(static_cast<EHexType>(FMath::RandRange(0, 3)));

    // ?? Load Fog of War Material (NEW CODE)
    static ConstructorHelpers::FObjectFinder<UMaterialInstance> FogMat(
        TEXT("MaterialInstanceConstant'/Game/Assets/HexGrid/M_HexTile_FogofWar_Instance'")
    );

    if (FogMat.Succeeded())
    {
        FogOfWarMaterial = FogMat.Object;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Fog of War Material!"));
    }

    // ?? Apply Fog of War (NEW CODE - Ensures hex starts hidden)
    ApplyFogOfWarState();
}


void AHexTile::RevealHex()
{
    if (!bIsRevealed)
    {
        bIsRevealed = true;
        ApplyFogOfWarState();

        UE_LOG(LogTemp, Warning, TEXT("Hex at Q=%d, R=%d revealed!"), Q, R);
    }
}

void AHexTile::ApplyFogOfWarState()
{
    if (!MeshComponent) return;

    if (bIsRevealed)
    {
        // Use HexType's correct material
        switch (HexType)
        {
        case EHexType::Grassland:
            MeshComponent->SetMaterial(0, GrassMaterial);
            break;
        case EHexType::Mountain:
            MeshComponent->SetMaterial(0, MountainMaterial);
            break;
        case EHexType::Water:
            MeshComponent->SetMaterial(0, WaterMaterial);
            break;
        case EHexType::Desert:
            MeshComponent->SetMaterial(0, DesertMaterial);
            break;
        }
    }
    else
    {
        // Ensure we apply the Fog of War material
        if (FogOfWarMaterial)
        {
            MeshComponent->SetMaterial(0, FogOfWarMaterial);
            UE_LOG(LogTemp, Warning, TEXT("Applying Fog of War to Hex at Q=%d, R=%d"), Q, R);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FogOfWarMaterial is NULL!"));
        }
    }
}

void AHexTile::SetHexType(EHexType NewType)
{
    HexType = NewType;

    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("MeshComponent is NULL in SetHexType!"));
        return;
    }

    switch (HexType)
    {
    case EHexType::Grassland:
        if (GrassMaterial)
        {
            MovementCost = 1;
            UE_LOG(LogTemp, Warning, TEXT("Setting Grassland Material on Hex: Q=%d, R=%d"), Q, R);
            MeshComponent->SetMaterial(0, GrassMaterial);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("GrassMaterial is NULL!"));
        }
        break;

    case EHexType::Mountain:
        if (MountainMaterial)
        {
            MovementCost = 2;
            UE_LOG(LogTemp, Warning, TEXT("Setting Mountain Material on Hex: Q=%d, R=%d"), Q, R);
            MeshComponent->SetMaterial(0, MountainMaterial);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("MountainMaterial is NULL!"));
        }
        break;

    case EHexType::Water:
        if (WaterMaterial)
        {
            MovementCost = -1;
            UE_LOG(LogTemp, Warning, TEXT("Setting Water Material on Hex: Q=%d, R=%d"), Q, R);
            MeshComponent->SetMaterial(0, WaterMaterial);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("WaterMaterial is NULL!"));
        }
        break;

    case EHexType::Desert:
        if (DesertMaterial)
        {
            MovementCost = 2;
            UE_LOG(LogTemp, Warning, TEXT("Setting Desert Material on Hex: Q=%d, R=%d"), Q, R);
            MeshComponent->SetMaterial(0, DesertMaterial);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("DesertMaterial is NULL!"));
        }
        break;
    }

    
    UE_LOG(LogTemp, Warning, TEXT("Setting Fog of War on Hex: Q=%d, R=%d"), Q, R);
    MeshComponent->SetMaterial(0, FogOfWarMaterial);

}


void AHexTile::CalculateWorldPosition(float HexSize)
{
    float x = HexSize * (3.0f / 2.0f * Q);
    float y = HexSize * (sqrt(3.0f) * (R + Q / 2.0f));

    WorldPosition = FVector(x, y, 0);
    SetActorLocation(WorldPosition);
}


void AHexTile::UpdateVisibility(const FVector& PlayerLocation, float MaxRenderDistance)
{
    float Distance = FVector::Dist(PlayerLocation, GetActorLocation());

    bool bShouldBeVisible = Distance <= MaxRenderDistance;
    MeshComponent->SetVisibility(bShouldBeVisible, true);
}

void AHexTile::SetHighlighted(bool bHighlight)
{
    if (!MeshComponent) return;

    if (bHighlight && HighlightMaterial)
    {
        MeshComponent->SetMaterial(0, HighlightMaterial);
    }
    else if (DefaultMaterial)
    {
        MeshComponent->SetMaterial(0, DefaultMaterial);
    }
}



