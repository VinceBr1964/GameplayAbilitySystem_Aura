// Copyright Vince Bracken


#include "AI/AuraFriendlyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/PlayerController.h"
#include <AI/AuraAIController.h>

void AAuraFriendlyAIController::BeginPlay()
{
	Super::BeginPlay();


}

AAuraFriendlyAIController::AAuraFriendlyAIController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>("FriendlyBlackboardComponent");
	check(Blackboard);
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>("FriendlyBehaviorTreeComponent");
	check(BehaviorTreeComponent);
}

void AAuraFriendlyAIController::SetEnemy(AActor* Enemy)
{
	if (Blackboard)
	{
		Blackboard->SetValueAsObject("EnemyTarget", Enemy);
	}
}