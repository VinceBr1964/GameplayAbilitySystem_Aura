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

void AHexGridManager::DrawHexEdgesOneByOne(AHexTile* HexToDraw, float PauseTime)
{
    if (!HexToDraw)
    {
        UE_LOG(LogTemp, Warning, TEXT("DrawHexEdgesOneByOne called with null HexTile."));
        return;
    }

    // Clear any previous timer
    GetWorldTimerManager().ClearTimer(EdgeTimerHandle);
    HexCorners.Empty();
    CurrentCornerIndex = 0;

    // 1) Gather corners (6) for this hex. 
    //    - We assume you know the hex radius (from your manager or from the tile).
    //    - You can also store a HexSize in AHexTile or pass it in, as needed.

    const float HexRadius = (/* e.g. HexSize or your tile’s known size */ 200.f);
    const int32 NumSides = 6;

    // Center is the actor’s location or a known stored location
    FVector Center = HexToDraw->GetActorLocation();

    // Precompute corners
    for (int32 i = 0; i < NumSides; i++)
    {
        // Each side is 60 degrees apart
        float AngleDeg = 60.f * i;
        float AngleRad = FMath::DegreesToRadians(AngleDeg);

        float X = Center.X + HexRadius * FMath::Cos(AngleRad);
        float Y = Center.Y + HexRadius * FMath::Sin(AngleRad);
        float Z = Center.Z; // Keep it flat

        HexCorners.Add(FVector(X, Y, Z));
    }

    // 2) Start a repeating timer that calls DrawNextEdge() once per PauseTime.
    GetWorldTimerManager().SetTimer(
        EdgeTimerHandle,
        this,
        &AHexGridManager::DrawNextEdge,
        PauseTime,
        true  // bLoop = true, so it keeps firing until we manually stop.
    );
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
	//GenerateHexGrid();
    //AssignNeighbors(); //Add this to permanently assign neighbors
	
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
    AssignNeighbors();
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
    DrawDebugLine(GetWorld(), LineStart, LineEnd, LineColor, true, 10.f, 0, 15.f);
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
        AHexTile* CurrentTile = StartTile;
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

        FVector LocalHexCorners[6];

        for (int i = 0; i < 6; i++)
        {
            float Angle = (PI / 3) * i; // 60 degrees per side
            LocalHexCorners[i] = FVector(
                Center.X + HexSize * FMath::Cos(Angle),
                Center.Y + HexSize * FMath::Sin(Angle),
                Center.Z + 5.0f
            );
        }

        int NextIndex = (EdgeIndex + 1) % 6; // Get the next point to complete the edge
        DrawDebugLine(GetWorld(), LocalHexCorners[EdgeIndex], LocalHexCorners[NextIndex], FColor::Cyan, true, 10.0f, 0, 13.0f);
        
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
            {+1, 0}, {0, +1}, {-1, +1},
            {-1,  0}, { 0, -1}, {+1, -1}
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

void AHexGridManager::DrawNextEdge()
{
    // Each edge is between HexCorners[i] and HexCorners[i+1],
    // wrapping around using modulus or by connecting corner 5 -> corner 0.

    if (HexCorners.Num() < 2)
    {
        // Something went wrong or user didn't initialize corners
        GetWorldTimerManager().ClearTimer(EdgeTimerHandle);
        return;
    }

    // We draw the edge from CurrentCornerIndex to the next corner
    int32 NextIndex = (CurrentCornerIndex + 1) % HexCorners.Num();

    // Actually draw the line
    DrawDebugLine(
        GetWorld(),
        HexCorners[CurrentCornerIndex],
        HexCorners[NextIndex],
        FColor::Green,
        false,   // bPersistentLines: false means it’ll eventually go away, or you can set true if you want indefinite lines
        10.f,    // Lifetime in seconds (can be large if you want them to remain visible)
        0,
        10.f      // Thickness of the line
    );

    // Move the index forward
    CurrentCornerIndex++;

    // If we have reached the final corner, we’ve drawn all 6 edges -> stop the timer
    if (CurrentCornerIndex >= HexCorners.Num())
    {
        // We’ve drawn all edges
        GetWorldTimerManager().ClearTimer(EdgeTimerHandle);
    }
}

void AHexGridManager::SetHexOwnerToPlayerIfStanding(AActor* PlayerActor)
{
    if (!PlayerActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerActor is null in SetHexOwnerToPlayerIfStanding."));
        return;
    }

    // 1. Find which hex tile the player is standing on
    const FVector PlayerLocation = PlayerActor->GetActorLocation();

    // If you already have a helper like GetHexTileAtLocation(FVector), you can use it:
    AHexTile* TileUnderPlayer = GetHexTileAtLocation(PlayerLocation);

    if (!TileUnderPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("No hex found under the player’s location."));
        return;
    }
    if (TileUnderPlayer->HexType == EHexType::Water)
    {
        return;
    }

    // 2. Set that tile's ownership to Player
    TileUnderPlayer->HexOwner = EHexOwner::Player;

    UE_LOG(LogTemp, Display, TEXT("Hex at Q=%d, R=%d is now owned by the player."),
        TileUnderPlayer->Q, TileUnderPlayer->R);
}

void AHexGridManager::DrawDebugLinesAroundPlayerOwnedHexes(float LineThickness /*= 5.0f*/) //
{
    for (AHexTile* Tile : HexTiles)
    {
        if (!Tile) continue;
        if (Tile->HexOwner == EHexOwner::Player)
        {
            DrawHexEdges(Tile, FColor::White, LineThickness);
        }
    }
}

void AHexGridManager::DrawPerimeterEdgesOfPlayerOwnedTiles(float LineThickness, FColor LineColor)
{
    // Go through all tiles
    for (AHexTile* Tile : HexTiles)
    {
        if (!Tile) continue;

        if (Tile->HexType == EHexType::Water)
        {
            Tile->HexOwner = EHexOwner::Neutral;
        }

        // Only care about player-owned tiles
        if (Tile->HexOwner != EHexOwner::Player) continue;

        // Precompute the corners
        TArray<FVector> Corners;
        Corners.Reserve(6);

        const FVector Center = Tile->GetActorLocation();
        const float HexRadius = /* e.g. your hex size */ 200.f;
        for (int32 i = 0; i < 6; i++)
        {
            float AngleDeg = ((60.f * i) + 0.f);
            float AngleRad = FMath::DegreesToRadians(AngleDeg);

            float X = Center.X + HexRadius * FMath::Cos(AngleRad);
            float Y = Center.Y + HexRadius * FMath::Sin(AngleRad);
            float Z = Center.Z;

            Corners.Add(FVector(X, Y, Z));
            
        }

        // For each of the 6 sides, check the corresponding neighbor
        for (int32 i = 0; i < 6; i++)
        {
            AHexTile* NeighborTile = nullptr;
            if (i < Tile->Neighbors.Num())  // Just in case
            {
                NeighborTile = Tile->Neighbors[i];
            }

            bool bShouldDrawEdge = false;
            if (NeighborTile == nullptr)
            {
                // No neighbor => definitely an outer edge
                bShouldDrawEdge = true;
            }
            else
            {
                // If neighbor is not owned by the player, then this is a border
                if (NeighborTile->HexOwner != EHexOwner::Player)
                {
                    bShouldDrawEdge = true;
                }
            }

            if (bShouldDrawEdge)
            {
                int32 NextIndex = (i + 1) % 6; // wrap around from edge 5 → 0

                DrawDebugLine(
                    GetWorld(),
                    Corners[i],
                    Corners[NextIndex],
                    LineColor,
                    true,         // bPersistentLines
                    10.0f,         // Lifetime (seconds)
                    0,             // Depth priority
                    LineThickness
                );
            }
        }
    }
}
void AHexGridManager::DrawHexEdges(AHexTile* HexTile, FColor Color, float Thickness)
{
    if (!HexTile) return;

    const FVector Center = HexTile->GetActorLocation();
    const float HexRadius = /* e.g. your HexSize */ 200.f;

    // 6 corners for a regular hex
    constexpr int32 NumCorners = 6;
    TArray<FVector> Corners;
    Corners.Reserve(NumCorners);

    // Compute corners. Each side is 60 degrees (PI/3) apart
    for (int32 i = 0; i < NumCorners; i++)
    {
        float Angle = FMath::DegreesToRadians(60.f * i);
        float X = Center.X + HexRadius * FMath::Cos(Angle);
        float Y = Center.Y + HexRadius * FMath::Sin(Angle);
        float Z = Center.Z;

        Corners.Add(FVector(X, Y, Z));
    }

    // Draw lines between consecutive corners
    for (int32 i = 0; i < NumCorners; i++)
    {
        int32 NextIndex = (i + 1) % NumCorners; // wrap around
        DrawDebugLine(
            GetWorld(),
            Corners[i],
            Corners[NextIndex],
            Color,
            true,      // bPersistentLines
            10.0f,      // Lifetime
            0,          // DepthPriority
            Thickness
        );
    }
}

void AHexGridManager::DrawPerpendicularBorderBetweenTiles(
    AHexTile* TileA,
    AHexTile* TileB,
    float HalfLineLength,   // how far the border extends from midpoint in each direction
    FColor LineColor,
    float LineThickness)
{
    if (!TileA || !TileB) return;

    // 1) Centers
    FVector CenterA = TileA->GetActorLocation();
    FVector CenterB = TileB->GetActorLocation();

    // 2) Midpoint
    FVector Midpoint = (CenterA + CenterB) * 0.5f;

    // 3) Direction from A -> B (normalized in XY)
    FVector Dir = (CenterB - CenterA);
    Dir.Z = 0.0f; // ignore vertical, if you just want it in 2D plane
    Dir.Normalize(); // safe to do if A != B

    // 4) Perpendicular direction (in XY plane)
    // For a 2D cross, you can swap X/Y and flip one sign:
    // e.g. if Dir = (dx, dy), then Perp = (-dy, dx) or (dy, -dx)
    FVector2D Dir2D(Dir.X, Dir.Y);
    FVector2D Perp2D(-Dir2D.Y, Dir2D.X);
    Perp2D.Normalize(); // just in case

    // 5) Convert back to 3D
    FVector Perp3D(Perp2D.X, Perp2D.Y, 0);

    // 6) Start and end points on either side of midpoint
    FVector StartPos = Midpoint - Perp3D * HalfLineLength;
    FVector EndPos = Midpoint + Perp3D * HalfLineLength;

    // 7) Draw the debug line
    DrawDebugLine(
        GetWorld(),
        StartPos,
        EndPos,
        LineColor,
        true,         // bPersistentLines
        10.0f,         // lifetime
        0,             // depth priority
        LineThickness
    );
}

void AHexGridManager::DrawPerpendicularBordersForPlayerTiles(
    float HalfLineLength /*=100.f*/,
    FColor LineColor /*=FColor::Yellow*/,
    float LineThickness /*=5.f*/)
{
    for (AHexTile* Tile : HexTiles)
    {
        if (!Tile) continue;
        if (Tile->HexType == EHexType::Water)
        {
            Tile->HexOwner = EHexOwner::Neutral;
        }

        // Only process tile if owned by the player
        if (Tile->HexOwner == EHexOwner::Player)
        {
            // For each neighbor
            for (AHexTile* Neighbor : Tile->Neighbors)
            {
                // If neighbor doesn't exist or is not owned by the player,
                // we want a border line 
                if (Neighbor == nullptr || Neighbor->HexOwner != EHexOwner::Player)
                {
                    // Optional: a check to avoid drawing duplicates. E.g. only draw if Tile < Neighbor
                    // or some other condition. Otherwise you might see the line repeated from each side.

                    DrawPerpendicularBorderBetweenTiles(Tile, Neighbor, HalfLineLength, LineColor, LineThickness);
                }
            }
        }
    }
}

void AHexGridManager::DebugHexOrientation(AHexTile* Tile, float HexRadius /*=200.f*/)
{
    if (!Tile) return;

    // Grab the tile’s center location
    const FVector Center = Tile->GetActorLocation();

    // 1) East (0°)
    {
        float AngleDeg = 0.f;
        float AngleRad = FMath::DegreesToRadians(AngleDeg);
        FVector Direction(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.f);

        FVector End = Center + (Direction * HexRadius);
        // Red line for East
        DrawDebugLine(
            GetWorld(),
            Center,
            End,
            FColor::Red,
            false,      // bPersistentLines
            30.f,       // Lifetime
            0,          // Depth priority
            15.f         // Thickness
        );
    }

    // 2) NE corner (60°)
    {
        float AngleDeg = 60.f;
        float AngleRad = FMath::DegreesToRadians(AngleDeg);
        FVector Direction(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.f);

        FVector End = Center + (Direction * HexRadius);
        // Green line for NE
        DrawDebugLine(
            GetWorld(),
            Center,
            End,
            FColor::Green,
            false,
            30.f,
            0,
            15.f
        );
    }

    // 3) NW corner (120°)
    {
        float AngleDeg = 120.f;
        float AngleRad = FMath::DegreesToRadians(AngleDeg);
        FVector Direction(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.f);

        FVector End = Center + (Direction * HexRadius);
        // Blue line for NW
        DrawDebugLine(
            GetWorld(),
            Center,
            End,
            FColor::Blue,
            false,
            30.f,
            0,
            15.f
        );
    }

    // Add more lines if desired (e.g. 180°, 240°, 300°) 
    // to see full orientation
}




