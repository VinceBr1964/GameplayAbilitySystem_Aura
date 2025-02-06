// Copyright Vince Bracken

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "AuraExplode.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraExplode : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void Explode(AActor* MinionActor);


};
