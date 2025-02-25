// Copyright Vince Bracken


#include "Game/TurnManager.h"

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
    StartTurn();
}

void ATurnManager::EndTurn()
{
    // You might have extra logic or checks before moving on
    NextTurn();
}

void ATurnManager::StartTurn()
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
    // AActor* ActiveEntity = TurnOrder.IsValidIndex(CurrentTurnIndex) ? TurnOrder[CurrentTurnIndex] : nullptr;
    // if (ActiveEntity)
    // {
    //     // e.g. cast to AI Controller, notify them to run their turn,
    //     // or if it's the player, enable "Player Turn" UI.
    // }
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