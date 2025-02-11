// Copyright Vince Bracken

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h" // Include this
#include "AHexTile.generated.h"

class AHexGridManager;

UENUM(BlueprintType)
enum class EHexType : uint8
{
    Grassland UMETA(DisplayName = "Grassland"),
    Mountain UMETA(DisplayName = "Mountain"),
    Water UMETA(DisplayName = "Water"),
    Desert UMETA(DisplayName = "Desert")
};

UENUM(BlueprintType)
enum class EHexOwner : uint8
{
    Neutral UMETA(DisplayName = "Neutral"),
    Player UMETA(DisplayName = "Player"),
    Faction UMETA(DisplayName = "Faction")
};

UCLASS()
class AURA_API AHexTile : public AActor
{
    GENERATED_BODY()

public:
    AHexTile();

    /** Mesh component for visualization */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hex Tile")
    UStaticMeshComponent* MeshComponent;

    // Position in hex coordinates (Axial system)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile")
    int32 Q; // Axial coordinate

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile")
    int32 R; // Axial coordinate

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile")
    int32 S; // Derived from Q and R

    // World Position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile")
    FVector WorldPosition;

    // Hex Owner (Neutral, Player, NPC)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile")
    FString HexOwner;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile")
    UMaterialInstance* DefaultMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile")
    UMaterialInstance* HighlightMaterial;

    // Type of terrain
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile")
    EHexType HexType = EHexType::Grassland;

    // Add a movement cost variable
    UPROPERTY(BlueprintReadOnly, Category = "Hex Tile")
    int32 MovementCost = 3; // Default cost (can be overridden based on terrain)

    // Resource type (Gold, Mana, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile")
    FString ResourceType = "None";

    // Units/Buildings in this hex
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile")
    TArray<AActor*> OccupyingUnits;

    // Neighbors (linked hexes)
    UPROPERTY()
    TArray<AHexTile*> Neighbors;

    // Materials for different hex types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile")
    UMaterialInstance* GrassMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile")
    UMaterialInstance* MountainMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile")
    UMaterialInstance* WaterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Tile")
    UMaterialInstance* DesertMaterial;

    // Fog of War State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog of War")
    bool bIsRevealed = false;

    // Material for Fog of War
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog of War")
    UMaterialInstance* FogOfWarMaterial;

    void RevealHex();
    void ApplyFogOfWarState();

    void HighlightEdge(int EdgeIndex);

    void SetHexType(EHexType NewType);

    // Function to calculate world position
    void CalculateWorldPosition(float HexSize);


    void UpdateVisibility(const FVector& PlayerLocation, float MaxRenderDistance);

    void SetHighlighted(bool bHighlight);

    void HighlightOuterEdges(TSet<AHexTile*> ValidTiles, AHexGridManager* HexGridManager);

};

