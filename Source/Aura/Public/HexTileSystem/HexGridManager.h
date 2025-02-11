// Copyright Vince Bracken

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HexTileSystem/AHexTile.h"
#include "DrawDebugHelpers.h"
#include "HexGridManager.generated.h"

UCLASS()
class AURA_API AHexGridManager : public AActor
{
    GENERATED_BODY()

public:
    AHexGridManager();

    void Tick(float DeltaTime);

    AHexTile* GetHexTileAtLocation(FVector WorldLocation);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
    int32 GridRadius = 5; // Determines size of the hex map

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
    float HexSize = 200.0f; // Radius of a single hex

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
    TSubclassOf<AHexTile> HexTileClass;

//    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
  //  float NormalizedNoise = 0.f;

    TArray<AHexTile*> HexTiles;

    //AHexTile* StartTile;

    virtual void BeginPlay() override;

    void GenerateHexGrid();

    EHexType DetermineLandType(FVector HexWorldPosition, int32 TotalGrassland, int32 TotalDesert, int32 TotalMountains);

    bool CanPlaceMountainCluster();

    void DrawHexDebugLines(AHexTile* HexTile);

    void ShowNeighborHexes(AHexTile* SelectedHex);

    void RevealHexAndNeighbors(AHexTile* CenterHex);

    TMap<FIntPoint, AHexTile*> HexTileMap;

    TArray<AHexTile*> GetValidMovementTiles(AHexTile* StartTile, int MovementRange);

    void DebugDrawBorderTiles(const TSet<AHexTile*>& ValidTiles, AHexTile* StartTile);

    void DebugDrawValidTiles(const TSet<AHexTile*>& ValidTiles);

    void DebugDrawPerpendicularBorders(const TSet<AHexTile*>& ValidTiles, AHexTile* StartTile);

    void DrawBorderLineBetweenTiles(AHexTile* TileA, AHexTile* TileB, FColor LineColor, float LineLength);

    TArray<AHexTile*> GetNeighborsInRange(AHexTile* StartTile, int Range);

    int GetHexDistance(AHexTile* A, AHexTile* B);

    void HighlightOuterEdges(TSet<AHexTile*> ValidTiles, int Distance, AHexTile* ReferenceTile);

    TArray<AHexTile*> GetMovementBoundaryTiles(TArray<AHexTile*> ValidTiles);

    void DrawMovementBoundary(TArray<TPair<AHexTile*, int>> BoundaryEdges);

    TArray<TPair<AHexTile*, int>> GetMovementBoundaryEdges(TArray<AHexTile*> ValidTiles);

    void ShowMovementRange(AHexTile* StartTile, int MovementRange);

    void ClearMovementRange();

    void AssignNeighbors();

private:

    AHexTile* LastSelectedHex = nullptr;  // Track the last selected hex
};
