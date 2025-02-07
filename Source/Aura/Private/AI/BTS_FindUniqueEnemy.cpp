// Copyright Vince Bracken


#include "AI/BTS_FindUniqueEnemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Character/AuraFriendlyCharacter.h" // Replace with your AI character header

UBTS_FindUniqueEnemy::UBTS_FindUniqueEnemy()
{
    NodeName = "Find Unique Enemy";
    Interval = 1.0f; // Runs every 1 second
}

void UBTS_FindUniqueEnemy::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn();
    if (!ControlledPawn) return;

    AAuraFriendlyCharacter* AICharacter = Cast<AAuraFriendlyCharacter>(ControlledPawn);
    if (!AICharacter) return;

    AICharacter->AssignUniqueTargets(); // Calls your optimized function

}
