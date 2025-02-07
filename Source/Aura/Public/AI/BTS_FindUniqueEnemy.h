// Copyright Vince Bracken

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_FindUniqueEnemy.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UBTS_FindUniqueEnemy : public UBTService
{
    GENERATED_BODY()

public:
    UBTS_FindUniqueEnemy();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
