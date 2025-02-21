// Copyright Vince Bracken

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "HexNPCController.generated.h"

class UBlackboardComponent;
class UBehaviorTreeComponent;

UCLASS()
class AHexNPCController : public AAIController
{
    GENERATED_BODY()

public:
    AHexNPCController();

protected:

 //   UPROPERTY(EditDefaultsOnly, Category = "AI")
 //   UBehaviorTree* BehaviorTreeAsset;

//    UPROPERTY(VisibleAnywhere, Category = "AI")
 //   TObjectPtr<UBlackboardComponent> Blackboard;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    TObjectPtr<UBehaviorTreeComponent> BehaviorTree;

private:

};
