// Copyright Vince Bracken

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraSummonAbility.h"
#include "AuraSummonFireMinions.generated.h"

UCLASS()
class AURA_API UAuraSummonFireMinions : public UAuraSummonAbility
{
	GENERATED_BODY()


public:
	TArray<FVector> GetSpawnLocations();

	// Override to define specific behavior for fire minions
	virtual void ExplodeMinion(AActor* Minion);

	// New properties specific to fire minions
	UPROPERTY(EditDefaultsOnly, Category = "Summoning | Fire Minions")
	TSubclassOf<UGameplayEffect> FireEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Summoning | Fire Minions")
	TSubclassOf<UGameplayEffect> ExplosionDamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Summoning | Fire Minions")
	float ExplosionDelay = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Summoning | Fire Minions")
	float ExplosionRadius = 250.f;
};
