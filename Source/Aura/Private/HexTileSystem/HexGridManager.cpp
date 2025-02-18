// Copyright Vince Bracken


#include "HexTileSystem/HexGridManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"  // Add this include at the top
#include "HexTileSystem/AHexTile.h"
#include "Engine/World.h"

// Sets default values
AHexGridManager::AHexGridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AHexGridManager::Tick(float DeltaTime)
{
    FVector PlayerLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();

    for (AHexTile* Tile : HexTiles)
    {
        Tile->UpdateVisibility(PlayerLocation, 5000.0f); // Only render tiles within 5000 units
    }
}

AHexTile* AHexGridManager::GetHexTileAtLocation(FVector WorldLocation)
{
    AHexTile* NearestTile = nullptr;
    float ClosestDistance = FLT_MAX; // Start with a large value

    for (auto& Elem : HexTileMap)
    {
        AHexTile* Tile = Elem.Value;
        if (!Tile) continue;

        float Distance = FVector::Dist(WorldLocation, Tile->GetActorLocation());

        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            NearestTile = Tile;
        }
    }

    return NearestTile;
}

// Called when the game starts or when spawned
void AHexGridManager::BeginPlay()
{
	Super::BeginPlay();
	GenerateHexGrid();
    AssignNeighbors(); //Add this to permanently assign neighbors
	
}

void AHexGridManager::GenerateHexGrid()
{
    if (!HexTileClass) return;

    TSet<FIntPoint> UsedCoordinates; // Track used Q, R coordinates
    int32 TotalGrassland = 0;
    int32 TotalDesert = 0;
    int32 TotalMountains = 0;

    for (int q = -GridRadius; q <= GridRadius; q++)
    {
        for (int r = FMath::Max(-GridRadius, -q - GridRadius); r <= FMath::Min(GridRadius, -q + GridRadius); r++)
        {
            int s = -q - r;
            FIntPoint CoordKey(q, r);

            if (UsedCoordinates.Contains(CoordKey))
            {
                UE_LOG(LogTemp, Error, TEXT("Duplicate Hex Detected at Q=%d, R=%d!"), q, r);
                continue;
            }

            FVector HexLocation;
            HexLocation.X = HexSize * (3.0f / 2.0f * q);
            HexLocation.Y = HexSize * (sqrt(3.0f) * (r + q / 2.0f));
            HexLocation.Z = 0;

            AHexTile* NewHex = GetWorld()->SpawnActor<AHexTile>(HexTileClass, HexLocation, FRotator::ZeroRotator);
            if (NewHex)
            {
                NewHex->Q = q;
                NewHex->R = r;
                NewHex->S = s;

                // **Step 1: Ensure Water on Outer Ring**
                if (FMath::Abs(q) == GridRadius || FMath::Abs(r) == GridRadius || FMath::Abs(s) == GridRadius)
                {
                    NewHex->SetHexType(EHexType::Water);
                }
                else
                {
                    // Step 2: Assign Terrain (Grassland > Desert > Mountains)
                    EHexType AssignedType = DetermineLandType(HexLocation, TotalGrassland, TotalDesert, TotalMountains);
                    NewHex->SetHexType(AssignedType);

                    // Track totals
                    if (AssignedType == EHexType::Grassland) TotalGrassland++;
                    else if (AssignedType == EHexType::Desert) TotalDesert++;
                    else if (AssignedType == EHexType::Mountain) TotalMountains++;
                }

                HexTiles.Add(NewHex);
                HexTileMap.Add(CoordKey, NewHex);
                UsedCoordinates.Add(CoordKey);

                UE_LOG(LogTemp, Warning, TEXT("Spawned Hex at Q=%d, R=%d, S=%d"), q, r, s);
            }
        }
    }
}


EHexType AHexGridManager::DetermineLandType(FVector HexWorldPosition, int32 TotalGrassland, int32 TotalDesert, int32 TotalMountains)
{
    // Get Perlin Noise value
    float NoiseValue = FMath::PerlinNoise2D(FVector2D(HexWorldPosition.X * 0.01f, HexWorldPosition.Y * 0.01f));

    // Normalize the noise and add a slight randomness factor
    float NormalizedNoise = FMath::Clamp(((NoiseValue + 1.0f) / 2.0f) + FMath::RandRange(-0.05f, 0.05f), 0.0f, 1.0f);

    UE_LOG(LogTemp, Warning, TEXT("Noise at (%f, %f) = %f | Normalized = %f"),
        HexWorldPosition.X, HexWorldPosition.Y, NoiseValue, NormalizedNoise);

    if (NormalizedNoise < 0.40f)
    {
        return EHexType::Desert; // Favor desert in low noise areas
    }
    else if (NormalizedNoise < 0.30f)
    {
        return EHexType::Grassland; // Grassland is most common
    }
    else if (NormalizedNoise > 0.75f || (FMath::RandRange(0, 100) < 10))
    {
        return EHexType::Mountain;
    }

    //Fix: Ensure all cases return a value
    return EHexType::Grassland;  // Default fallback in case nothing else is hit
}


bool AHexGridManager::CanPlaceMountainCluster()
{
    int32 ClusterSize = FMath::RandRange(3, 5); // Group size of 3-5
    int32 ClusterCount = 0;

    for (AHexTile* Tile : HexTiles)
    {
        if (Tile->HexType == EHexType::Mountain)
        {
            ClusterCount++;
        }

        if (ClusterCount >= ClusterSize)
        {
            return false; // Stop adding mountains
        }
    }

    return true; // Allow more mountains
}


void AHexGridManager::DrawHexDebugLines(AHexTile* HexTile)
{
    if (!HexTile) return;

    const FVector Center = HexTile->WorldPosition;
    //const float HexRadius = HexSize;

    // Hex Corner Offsets (6 corners)
    const float Angle = PI / 3; // 60 degrees per side
    FVector Corners[6];

    for (int i = 0; i < 6; i++)
    {
        float AngleOffset = Angle * i;
        Corners[i] = FVector(
            Center.X + HexSize * FMath::Cos(AngleOffset),
            Center.Y + HexSize * FMath::Sin(AngleOffset),
            Center.Z
        );
    }

    // Draw lines between the corners
    for (int i = 0; i < 6; i++)
    {
        DrawDebugLine(GetWorld(), Corners[i], Corners[(i + 1) % 6], FColor::Green, true, -1, 0, 2.f);
    }

    // Draw text above the hex to show its coordinates
    FString HexCoordsText = FString::Printf(TEXT("Q:%d, R:%d, S:%d"), HexTile->Q, HexTile->R, HexTile->S);
    DrawDebugString(GetWorld(), Center + FVector(0, 0, 50), HexCoordsText, nullptr, FColor::White, 0.f, true);
}

void AHexGridManager::ShowNeighborHexes(AHexTile* SelectedHex)
{
    if (!SelectedHex) return;

    // Unhighlight previous neighbors
    if (LastSelectedHex)
    {
        int32 NeighborOffsets[6][2] = {
            {+1, 0}, {+1, -1}, {0, -1}, {-1, 0}, {-1, +1}, {0, +1}
        };

        for (int i = 0; i < 6; i++)
        {
            int32 NeighborQ = LastSelectedHex->Q + NeighborOffsets[i][0];
            int32 NeighborR = LastSelectedHex->R + NeighborOffsets[i][1];
        }
    }

    // Highlight new neighbors
    int32 NeighborOffsets[6][2] = {
        {+1, 0}, {+1, -1}, {0, -1}, {-1, 0}, {-1, +1}, {0, +1}
    };

    for (int i = 0; i < 6; i++)
    {
        int32 NeighborQ = SelectedHex->Q + NeighborOffsets[i][0];
        int32 NeighborR = SelectedHex->R + NeighborOffsets[i][1];

    }

    // ?? New Code: Reveal the clicked hex and its neighbors
    RevealHexAndNeighbors(SelectedHex);

    LastSelectedHex = SelectedHex;
}


void AHexGridManager::RevealHexAndNeighbors(AHexTile* CenterHex)
{
    if (!CenterHex) return;

    UE_LOG(LogTemp, Warning, TEXT("Revealing Hex at Q=%d, R=%d"), CenterHex->Q, CenterHex->R);
    CenterHex->RevealHex(); // Reveal the hex the player is on

    // Define neighbor offsets
    int32 NeighborOffsets[6][2] = {
        {+1, 0}, {+1, -1}, {0, -1}, {-1, 0}, {-1, +1}, {0, +1}
    };

    TSet<AHexTile*> HexesToReveal; // Store all hexes to be revealed

    // First pass: Reveal neighbors (1 hex away)
    for (int i = 0; i < 6; i++)
    {
        int32 NeighborQ = CenterHex->Q + NeighborOffsets[i][0];
        int32 NeighborR = CenterHex->R + NeighborOffsets[i][1];

        if (AHexTile** FoundTile = HexTileMap.Find(FIntPoint(NeighborQ, NeighborR)))
        {
            UE_LOG(LogTemp, Warning, TEXT("Revealing Neighbor Hex at Q=%d, R=%d"), NeighborQ, NeighborR);
            (*FoundTile)->RevealHex();
            HexesToReveal.Add(*FoundTile); // Store for second pass
        }
    }

    // Second pass: Reveal neighbors of the neighbors (2 hexes away)
    for (AHexTile* FirstLayerHex : HexesToReveal)
    {
        for (int i = 0; i < 6; i++)
        {
            int32 SecondNeighborQ = FirstLayerHex->Q + NeighborOffsets[i][0];
            int32 SecondNeighborR = FirstLayerHex->R + NeighborOffsets[i][1];

            if (AHexTile** SecondLayerHex = HexTileMap.Find(FIntPoint(SecondNeighborQ, SecondNeighborR)))
            {
                if (!HexesToReveal.Contains(*SecondLayerHex)) // Prevent duplicate reveals
                {
                    UE_LOG(LogTemp, Warning, TEXT("Revealing Second Layer Hex at Q=%d, R=%d"), SecondNeighborQ, SecondNeighborR);
                    (*SecondLayerHex)->RevealHex();
                }
            }
        }
    }
}


TArray<AHexTile*> AHexGridManager::GetValidMovementTiles(AHexTile* StartTile, int MovementRange)
{
    TSet<AHexTile*> ValidTiles;
    TSet<AHexTile*> TilesToCheck;
    TMap<AHexTile*, int> TileDistances;

    if (!StartTile) return ValidTiles.Array();

    // Step 1: Gather all tiles within the movement range (1 or 2 hexes away)
    for (AHexTile* Neighbor : GetNeighborsInRange(StartTile, MovementRange))
    {
        int Distance = GetHexDistance(StartTile, Neighbor);
        TileDistances.Add(Neighbor, Distance);
        TilesToCheck.Add(Neighbor);
    }

    // Step 2: Validate tiles that are 2 hexes away
    for (AHexTile* Tile : TilesToCheck)
    {
        if (TileDistances[Tile] == 2) // Only process tiles exactly 2 hexes away
        {
            // 🚨 NEW FIX: Skip any tile with movement cost -1
            if (Tile->MovementCost == -1)
            {
                UE_LOG(LogTemp, Warning, TEXT("Skipping tile at Q=%d, R=%d due to movement cost -1"), Tile->Q, Tile->R);
                continue;
            }

            bool bHasValidSharedNeighbor = false;

            for (AHexTile* SharedNeighbor : Tile->Neighbors)
            {
                if (StartTile->Neighbors.Contains(SharedNeighbor) && SharedNeighbor->MovementCost == 1)
                {
                    bHasValidSharedNeighbor = true;
                    break; // If one valid path exists, we can stop checking
                }
            }

            if (bHasValidSharedNeighbor)
            {
                ValidTiles.Add(Tile);
            }
        }
    }

    // Step 3: Validate tiles that are 1 hex away
    for (AHexTile* Tile : TilesToCheck)
    {
        if (TileDistances[Tile] == 1) // Directly adjacent tiles
        {
            if (Tile->MovementCost != -1 && MovementRange >= Tile->MovementCost)
            {
                ValidTiles.Add(Tile);
            }
        }
    }

    // Step 4: Highlight only the outer edges of tiles 2 hexes away
    //HighlightOuterEdges(ValidTiles, 2, StartTile);

    // Debug draw the valid tiles
    //DebugDrawValidTiles(ValidTiles);

    // Debug draw the border tiles (Red)
    //DebugDrawBorderTiles(ValidTiles, StartTile);

    // Debug draw perpendicular midpoint borders (Blue), but exclude StartTile
    DebugDrawPerpendicularBorders(ValidTiles, StartTile);

    return ValidTiles.Array();
}

void AHexGridManager::DebugDrawBorderTiles(const TSet<AHexTile*>& ValidTiles, AHexTile* StartTile)
{
    if (!StartTile) return;

    TSet<AHexTile*> BorderTiles;

    // Check the neighbors of tiles 1 distance away that aren't in ValidTiles
    for (AHexTile* Tile : ValidTiles)
    {
        if (GetHexDistance(StartTile, Tile) == 1)
        {
            for (AHexTile* Neighbor : Tile->Neighbors)
            {
                if (!ValidTiles.Contains(Neighbor))
                {
                    BorderTiles.Add(Neighbor);
                }
            }
        }
    }

    // Step 1: Find tiles that are exactly 2 hexes away
    for (AHexTile* Tile : ValidTiles)
    {
       if (GetHexDistance(StartTile, Tile) == 2)
       {
            // Step 2: Identify neighboring tiles of these tiles that are NOT in ValidTiles
            for (AHexTile* Neighbor : Tile->Neighbors)
            {
                if (!ValidTiles.Contains(Neighbor))
                {
                    BorderTiles.Add(Neighbor);
                }
            }
       }
    }

    // Step 3: Draw red debug spheres on border tiles
    for (AHexTile* BorderTile : BorderTiles)
    {
        if (BorderTile)
        {
            DrawDebugSphere(GetWorld(), BorderTile->GetActorLocation(), 50.f, 12, FColor::Red, false, 15.f);
        }
    }
}

void AHexGridManager::DebugDrawValidTiles(const TSet<AHexTile*>& ValidTiles)
{
    for (AHexTile* Tile : ValidTiles)
    {
        if (Tile)
        {
            DrawDebugSphere(GetWorld(), Tile->GetActorLocation(), 50.f, 12, FColor::Green, false, 15.f);
        }
    }
}

void AHexGridManager::DebugDrawPerpendicularBorders(const TSet<AHexTile*>& ValidTiles, AHexTile* StartTile)
{
    if (ValidTiles.Num() == 0 || !StartTile) return;

    float LineLength = 200.0f;

    // Process all valid tiles except StartTile
    for (AHexTile* Tile : ValidTiles)
    {
        if (!Tile || Tile == StartTile) continue; // 🚨 Skip StartTile in this loop

        for (AHexTile* Neighbor : Tile->Neighbors)
        {
            if (!Neighbor || ValidTiles.Contains(Neighbor) || Neighbor == StartTile) continue; // 🚨 Skip valid neighbors

            // Call the new helper function to draw the border line
            DrawBorderLineBetweenTiles(Tile, Neighbor, FColor::Blue, LineLength);
        }
    }

    // Process only StartTile separately
    for (AHexTile* Neighbor : StartTile->Neighbors)
    {
        if (!ValidTiles.Contains(Neighbor)) // 🚨 Only draw for invalid neighbors
        {
            DrawBorderLineBetweenTiles(StartTile, Neighbor, FColor::Blue, LineLength);
        }
    }
}

void AHexGridManager::DrawBorderLineBetweenTiles(AHexTile* TileA, AHexTile* TileB, FColor LineColor, float LineLength)
{
    if (!TileA || !TileB) return;

    FVector CenterA = TileA->GetActorLocation();
    FVector CenterB = TileB->GetActorLocation();

    // Step 1: Compute midpoint
    FVector Midpoint = (CenterA + CenterB) / 2;

    // Step 2: Compute direction from A to B
    FVector Direction = (CenterB - CenterA).GetSafeNormal();

    // Step 3: Compute perpendicular direction (rotate 90 degrees in the XY plane)
    FVector Perpendicular = FVector(-Direction.Y, Direction.X, 0).GetSafeNormal();

    // Step 4: Compute line start and end
    FVector LineStart = Midpoint - Perpendicular * LineLength / 2;
    FVector LineEnd = Midpoint + Perpendicular * LineLength / 2;

    // Step 5: Draw debug line
    DrawDebugLine(GetWorld(), LineStart, LineEnd, LineColor, false, 10.f, 0, 15.f);
}

// Returns all neighbors within given range
TArray<AHexTile*> AHexGridManager::GetNeighborsInRange(AHexTile* StartTile, int Range)
{
    TArray<AHexTile*> Neighbors;
    TQueue<AHexTile*> TileQueue;
    TSet<AHexTile*> VisitedTiles;

    TileQueue.Enqueue(StartTile);
    VisitedTiles.Add(StartTile);

    while (!TileQueue.IsEmpty())
    {
        AHexTile* CurrentTile;
        TileQueue.Dequeue(CurrentTile);

        for (AHexTile* Neighbor : CurrentTile->Neighbors)
        {
            if (!VisitedTiles.Contains(Neighbor) && GetHexDistance(StartTile, Neighbor) <= Range)
            {
                TileQueue.Enqueue(Neighbor);
                VisitedTiles.Add(Neighbor);
            }
        }
    }

    VisitedTiles.Remove(StartTile); // Exclude the starting tile
    return VisitedTiles.Array();
}

// Calculates hex distance (Axial coordinates)
int AHexGridManager::GetHexDistance(AHexTile* A, AHexTile* B)
{
    return (abs(A->Q - B->Q) + abs(A->R - B->R) + abs((A->Q + A->R) - (B->Q + B->R))) / 2;
}

TArray<AHexTile*> AHexGridManager::GetMovementBoundaryTiles(TArray<AHexTile*> ValidTiles)
{
    TArray<AHexTile*> BoundaryTiles;

    for (AHexTile* Tile : ValidTiles)
    {
        bool bIsBoundary = false;

        // Check each neighbor to see if it's outside the movement range
        for (AHexTile* Neighbor : Tile->Neighbors)
        {
            if (!ValidTiles.Contains(Neighbor))
            {
                bIsBoundary = true;
                break;
            }
        }

        if (bIsBoundary)
        {
            BoundaryTiles.Add(Tile);
        }
    }
    return BoundaryTiles;
}

void AHexGridManager::DrawMovementBoundary(TArray<TPair<AHexTile*, int>> BoundaryEdges)
{
    for (TPair<AHexTile*, int> EdgePair : BoundaryEdges)
    {
        AHexTile* Tile = EdgePair.Key;
        int EdgeIndex = EdgePair.Value;

        FVector Center = Tile->GetActorLocation();
        HexSize = 200.0f; // Adjust based on your hex size

        FVector HexCorners[6];

        for (int i = 0; i < 6; i++)
        {
            float Angle = (PI / 3) * i; // 60 degrees per side
            HexCorners[i] = FVector(
                Center.X + HexSize * FMath::Cos(Angle),
                Center.Y + HexSize * FMath::Sin(Angle),
                Center.Z + 5.0f
            );
        }

        int NextIndex = (EdgeIndex + 1) % 6; // Get the next point to complete the edge
        DrawDebugLine(GetWorld(), HexCorners[EdgeIndex], HexCorners[NextIndex], FColor::Cyan, false, 10.0f, 0, 13.0f);
        
    }
}


TArray<TPair<AHexTile*, int>> AHexGridManager::GetMovementBoundaryEdges(TArray<AHexTile*> ValidTiles)
{
    TArray<TPair<AHexTile*, int>> BoundaryEdges;

    for (AHexTile* Tile : ValidTiles)
    {
        int Q = Tile->Q;
        int R = Tile->R;

        int NeighborOffsets[6][2] = {
            {+1, 0}, {+1, -1}, { 0, -1},
            {-1, 0}, {-1, +1}, { 0, +1}
        };

        for (int i = 0; i < 6; i++)
        {
            int NeighborQ = Q + NeighborOffsets[i][0];
            int NeighborR = R + NeighborOffsets[i][1];

            FIntPoint NeighborKey(NeighborQ, NeighborR);

            // If there's no entry for this neighbor in the map, it's definitely an outer edge
            if (!HexTileMap.Contains(NeighborKey))
            {
                BoundaryEdges.Add(TPair<AHexTile*, int>(Tile, i));
            }
            else
            {
                // Otherwise, grab the neighbor tile from the map
                AHexTile* NeighborTile = HexTileMap[NeighborKey];

                // If this neighbor is not in ValidTiles or is restricted, treat it as an outer edge
                if (!ValidTiles.Contains(NeighborTile) || !ValidTiles.Contains(HexTileMap[NeighborKey]))//bIsRevealed
                {
                    BoundaryEdges.Add(TPair<AHexTile*, int>(Tile, i));
                }
            }
        }
    }

    return BoundaryEdges;
}

void AHexGridManager::ShowMovementRange(AHexTile* StartTile, int MovementRange)
{
    if (!StartTile) return;

    TArray<AHexTile*> ValidTiles = GetValidMovementTiles(StartTile, MovementRange);

    for (AHexTile* Tile : ValidTiles)
    {
        Tile->SetHighlighted(true);
    }

    TArray<TPair<AHexTile*, int>> BoundaryEdges = GetMovementBoundaryEdges(ValidTiles);
    //DrawMovementBoundary(BoundaryEdges);
}

void AHexGridManager::ClearMovementRange()
{
    for (AHexTile* Tile : HexTiles)
    {
        Tile->SetHighlighted(false);
    }

    // Debug Draw Lines are temporary (they disappear after 5 seconds),
    // so no need to manually clear them unless using a persistent method.
}

void AHexGridManager::AssignNeighbors()
{
    for (auto& Elem : HexTileMap)
    {
        AHexTile* Tile = Elem.Value;
        if (!Tile) continue;

        int Q = Tile->Q;
        int R = Tile->R;

        // Hex offsets for adjacent tiles
        int NeighborOffsets[6][2] = {
            {+1, 0}, {+1, -1}, {0, -1},
            {-1, 0}, {-1, +1}, {0, +1}
        };

        for (int i = 0; i < 6; i++)
        {
            int NeighborQ = Q + NeighborOffsets[i][0];
            int NeighborR = R + NeighborOffsets[i][1];

            FIntPoint NeighborKey(NeighborQ, NeighborR);
            if (HexTileMap.Contains(NeighborKey))
            {
                AHexTile* NeighborTile = HexTileMap[NeighborKey];
                Tile->Neighbors.Add(NeighborTile);
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("Tile at Q: %d, R: %d assigned %d neighbors"),
            Tile->Q, Tile->R, Tile->Neighbors.Num());
    }
}






