// Copyright Vince Bracken

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AuraFriendlyAIController.generated.h"


class UBlackboardComponent;
class UBehaviorTreeComponent;

/**
 * 
 */
UCLASS()
class AURA_API AAuraFriendlyAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AAuraFriendlyAIController();

	virtual void BeginPlay() override;

	void SetEnemy(AActor* Enemy);

protected:
	/** Blackboard and Behavior Tree */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;
};
