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

    // The main Blueprint-callable function
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawHexEdgesOneByOne(AHexTile* HexToDraw, float PauseTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Ownership")
    void SetHexOwnerToPlayerIfStanding(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Ownership")
    void DrawDebugLinesAroundPlayerOwnedHexes(float LineThickness = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Ownership")
    void DrawPerimeterEdgesOfPlayerOwnedTiles(float LineThickness = 15.0f, FColor LineColor = FColor::White);

    UFUNCTION(BlueprintCallable, Category = "Ownership")
    void DrawPerpendicularBordersForPlayerTiles(float HalfLineLength, FColor LineColor, float LineThickness);

    void DrawPerpendicularBorderBetweenTiles(AHexTile* TileA, AHexTile* TileB, float HalfLineLength, FColor LineColor, float LineThickness);

    UFUNCTION(BlueprintCallable, Category = "Ownership")
    AHexTile* GetHexTileAtLocation(FVector WorldLocation);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
    int32 GridRadius = 5; // Determines size of the hex map

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
    float HexSize = 200.0f; // Radius of a single hex

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
    TSubclassOf<AHexTile> HexTileClass;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugHexOrientation(AHexTile* Tile, float HexRadius = 200.f);

//    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
  //  float NormalizedNoise = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid")
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

    // Helper to draw edges for a single tile
    void DrawHexEdges(AHexTile* HexTile, FColor Color, float Thickness);





    
protected:
    // We'll store the corners to draw, so the timer can access them incrementally
    TArray<FVector> HexCorners;
    // Index of the current corner
    int32 CurrentCornerIndex = 0;
    // Timer handle
    FTimerHandle EdgeTimerHandle;

    // Helper function that draws the next edge, then increments corner index
    void DrawNextEdge();

private:

    AHexTile* LastSelectedHex = nullptr;  // Track the last selected hex
};
