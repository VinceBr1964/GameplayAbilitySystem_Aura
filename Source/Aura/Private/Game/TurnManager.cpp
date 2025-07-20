// Copyright Vince Bracken


#include "Game/TurnManager.h"
#include "HexTileSystem/HexGridManager.h"
#include "HexTileSystem/AHexTile.h"
#include "Character/AuraCharacterBase.h"
#include "EngineUtils.h"          // For TActorIterator
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include <Actions/PawnAction.h>
#include <PLayer/AuraPlayerController.h>

ATurnManager::ATurnManager()
{
    NumPlayers = 2;
    PrimaryActorTick.bCanEverTick = false;
    CurrentTurnIndex = 0;
    RoundNumber = 1;

    TurnPhase = ETurnPhase::PreTurn;
}

void ATurnManager::BeginPlay()
{
    Super::BeginPlay();


    // Optionally, do an immediate "StartTurn" if the turn order is already set.
    // If your design requires waiting, you can call StartTurn from GameMode or after you init the TurnOrder.
}

void ATurnManager::InitializeTurnOrder(const TArray<AActor*>& InTurnOrder)
{
    
    TurnOrder = InTurnOrder;
    CurrentTurnIndex = 0;
    RoundNumber = 1;


    // Optionally call StartTurn() here if you want to begin immediately.
}

void ATurnManager::NextTurn()
{
    // Finish up the current turn
    // Broadcast OnTurnEnded for UI, logs, etc.
    OnTurnEnded.Broadcast(CurrentTurnIndex, RoundNumber);

    // Advance the turn index
    AdvanceTurnIndex();

    // Now handle the next turn
    //StartTurn();
}

void ATurnManager::EndTurn()
{
    // You might have extra logic or checks before moving on
    NextTurn();
}

void ATurnManager::StartTurn(AHexGridManager* InHexGridManager)
{
    // Pre-turn phase: e.g. resource ticks, upkeep, etc.
    TurnPhase = ETurnPhase::PreTurn;
    OnPreTurn.Broadcast(CurrentTurnIndex, RoundNumber);

    // Then the "active turn" phase
    TurnPhase = ETurnPhase::Turn;
    OnTurnStarted.Broadcast(CurrentTurnIndex, RoundNumber);

    // 
    // Here you'd typically let the "Current" actor (or AI) know that it's their turn 
    // so they can make moves. For example:
    //
    AActor* ActiveEntity = TurnOrder.IsValidIndex(CurrentTurnIndex) ? TurnOrder[CurrentTurnIndex] : nullptr;
    if (ActiveEntity)
    {
        // Reset movement points for the actor starting its turn
        if (AAuraCharacterBase* Char = Cast<AAuraCharacterBase>(ActiveEntity))
        {
            Char->CurrentHexMoveRange = Char->MaxHexMoveRange;
        }

        AAuraPlayerController* PC = Cast<AAuraPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
        if (PC)
        {
            /*for (TActorIterator<AHexGridManager> It(GetWorld()); It; ++It)
            {
                GridManager = *It;
                break;  // Stop after finding the first one
            }*/
            PC->DispatchBeginPlay();
            PC->ToggleHexMovementMode(ActiveEntity, InHexGridManager);
        }
    }
    //
}

void ATurnManager::AdvanceTurnIndex()
{
    // Move to the next index
    CurrentTurnIndex++;

    // If we passed the last participant, wrap around and increment round
    if (CurrentTurnIndex >= TurnOrder.Num())
    {
        CurrentTurnIndex = 0;
        RoundNumber++;
    }

    // Set the phase back to pre-turn by default
    TurnPhase = ETurnPhase::PreTurn;
}

void ATurnManager::SpawnAvatarsOnEdge(AHexGridManager* InHexGridManager)
{
    // Find the HexGridManager

    if (!InHexGridManager)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnAvatarsOnEdge: No AHexGridManager found in the world!"));
        return;
    }

    // Gather all edge tiles (outer ring tiles)
    TArray<AHexTile*> EdgeTiles;
    if (EdgeTiles.Num() == 0)
    {
        InHexGridManager->GenerateHexGrid();
    }
    for (AHexTile* Tile : InHexGridManager->HexTiles)
    {
        if (!Tile) continue;

        // A tile is "edge" if |Q|, |R|, or |S| equals the grid radius
        const int32 Radius = InHexGridManager->GridRadius;
        if (FMath::Abs(Tile->Q) == Radius ||
            FMath::Abs(Tile->R) == Radius ||
            FMath::Abs(Tile->S) == Radius)
        {
            EdgeTiles.Add(Tile);
        }
    }

    if (EdgeTiles.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnAvatarsOnEdge: No edge tiles found - cannot spawn avatars!"));
        return;
    }

    // We want to place NumPlayers avatars spaced roughly evenly.
    // A simple approach: sort the EdgeTiles by angle around center, then pick them at equal intervals.
    // Alternatively, just pick them by stepping through the array at intervals. Here is a simple stepping:
    EdgeTiles.Sort([](AHexTile& A, AHexTile& B)
        {
            // Compare by angle relative to origin
            const float AngleA = FMath::Atan2(A.WorldPosition.Y, A.WorldPosition.X);
            const float AngleB = FMath::Atan2(B.WorldPosition.Y, B.WorldPosition.X);
            return AngleA < AngleB;
        });

    const int32 CountEdge = EdgeTiles.Num();
    if (CountEdge < NumPlayers)
    {
        UE_LOG(LogTemp, Warning, TEXT("Not enough edge tiles to space out all players. Will just spawn them in order."));
    }

    // Step size
    const int32 Step = CountEdge / NumPlayers;

    for (int32 i = 0; i < NumPlayers; i++)
    {
        const int32 Index = i * Step;
        if (!EdgeTiles.IsValidIndex(Index)) break; // safety check

        AHexTile* SpawnTile = EdgeTiles[Index];
        if (!SpawnTile) continue;

        FVector SpawnLocation = SpawnTile->GetActorLocation();
        SpawnLocation.Z += 100.f; // Just to avoid spawning at ground collision, adjust as you see fit

        TSubclassOf<AActor> ChosenClass = nullptr;

        ChosenClass = NPCAvatarClass;
        if (!ChosenClass) continue;

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;      // If you wish
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        // Spawn the avatar
        AActor* NewAvatar = GetWorld()->SpawnActor<AActor>(ChosenClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

        if (NewAvatar)
        {
            // (Optional) You could do extra initialization on the avatar here, e.g. set its name, 
            // or set up controller/possession if needed.

            // Add to the TurnOrder so we know the spawn belongs in the turn sequence
            TurnOrder.Add(NewAvatar);

            UE_LOG(LogTemp, Log, TEXT("Spawned %s at tile Q=%d, R=%d"),
                *NewAvatar->GetName(), SpawnTile->Q, SpawnTile->R);
        }
    }

    // If you like, automatically call InitializeTurnOrder() so the new TurnOrder is used:
    //InitializeTurnOrder(TurnOrder);

    // Or you might wait and call InitializeTurnOrder(...) somewhere else in your setup process.
}