// Copyright Vince Bracken

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TurnManager.generated.h"


/**
 * Simple enum to represent different phases of a turn if you need them
 */
UENUM(BlueprintType)
enum class ETurnPhase : uint8
{
    PreTurn     UMETA(DisplayName = "PreTurn"),   // E.g. resource updates, upkeep
    Turn        UMETA(DisplayName = "Turn"),      // Active player's turn
    PostTurn    UMETA(DisplayName = "PostTurn")   // Cleanup after turn
};

/**
 * Delegate for broadcasting turn events (e.g. UI updates or AI triggers).
 * Provides current turn index and round number for convenience.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTurnEventSignature, int32, TurnIndex, int32, RoundNumber);

/**
 * ATurnManager
 *
 * Manages turn order and round progression in a turn-based game.
 * You can place this in the level or spawn it at runtime.
 */

UCLASS()
class AURA_API ATurnManager : public AGameStateBase
{
	GENERATED_BODY()
	
public:
    // Sets default values for this actor's properties
    ATurnManager();

    /** Called at game start or when spawned */
    virtual void BeginPlay() override;

    /**
     * Initialize the turn order with a list of participants (players & AI).
     * You can store either PlayerStates, Controllers, or custom objects—whatever suits your design.
     */
    UFUNCTION(BlueprintCallable, Category = "Turn Manager")
    void InitializeTurnOrder(const TArray<AActor*>& InTurnOrder);

    /**
     * Moves the turn manager to the next turn in the sequence.
     * If we pass the last participant, we increment RoundNumber and wrap around.
     */
    UFUNCTION(BlueprintCallable, Category = "Turn Manager")
    void NextTurn();

    /** Manually end the current turn. This might just call NextTurn internally. */
    UFUNCTION(BlueprintCallable, Category = "Turn Manager")
    void EndTurn();

    /** Called at the start of each participant's turn. */
    UFUNCTION(BlueprintCallable, Category = "Turn Manager")
    void StartTurn();

    /** For hooking up UI or AI logic: called just before a new turn starts. */
    UPROPERTY(BlueprintAssignable, Category = "Turn Manager")
    FTurnEventSignature OnPreTurn;

    /** For hooking up UI or AI logic: called when a new turn actually starts. */
    UPROPERTY(BlueprintAssignable, Category = "Turn Manager")
    FTurnEventSignature OnTurnStarted;

    /** For hooking up end-of-turn logic. */
    UPROPERTY(BlueprintAssignable, Category = "Turn Manager")
    FTurnEventSignature OnTurnEnded;

protected:

    /** List of all participants in turn order—could be Player Controllers or custom Pawn/Unit references. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Manager")
    TArray<AActor*> TurnOrder;

    /** Index into TurnOrder for the current turn. */
    UPROPERTY(BlueprintReadOnly, Category = "Turn Manager")
    int32 CurrentTurnIndex;

    /** Which round are we in? Increments each time we wrap past the last participant. */
    UPROPERTY(BlueprintReadOnly, Category = "Turn Manager")
    int32 RoundNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Turn Manager")
    int32 NumPlayers;

    /** The current turn phase (e.g. PreTurn, Turn, PostTurn). */
    UPROPERTY(BlueprintReadOnly, Category = "Turn Manager")
    ETurnPhase TurnPhase;

    /** Helper function to advance to the next participant or next round. */
    void AdvanceTurnIndex();
};
