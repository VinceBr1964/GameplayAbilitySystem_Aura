// Copyright Vince Bracken


#include "AI/HexNPCController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTree.h"

AHexNPCController::AHexNPCController()
{
    // Create Blackboard + BehaviorTree components
    Blackboard = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
    check(Blackboard);
    BehaviorTree = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
    check(BehaviorTree);
}

