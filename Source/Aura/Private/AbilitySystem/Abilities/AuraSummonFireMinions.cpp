// Copyright Vince Bracken

#include "AbilitySystem/Abilities/AuraSummonFireMinions.h"
#include "AbilitySystem/Abilities/AuraSummonAbility.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include <AbilitySystem/AuraAbilitySystemLibrary.h>
#include <AbilitySystem/Abilities/AuraBeamSpell.h>



TArray<FVector> UAuraSummonFireMinions::GetSpawnLocations()
{

	TArray<FVector> SpawnLocations = Super::GetSpawnLocations();
	return SpawnLocations;
}


void UAuraSummonFireMinions::ExplodeMinion(AActor* Minion)
{
	if (!Minion) return;

	// Apply explosion damage
	if (ExplosionDamageEffect)
	{
		TArray<AActor*> AffectedActors;
		UKismetSystemLibrary::SphereOverlapActors(
			GetWorld(),
			Minion->GetActorLocation(),
			ExplosionRadius,
			TArray<TEnumAsByte<EObjectTypeQuery>>(),
			ACharacter::StaticClass(),
			TArray<AActor*>(),
			AffectedActors);

		for (AActor* Target : AffectedActors)
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
			if (TargetASC)
			{
				FGameplayEffectSpecHandle EffectSpec = TargetASC->MakeOutgoingSpec(ExplosionDamageEffect, 1.0f, TargetASC->MakeEffectContext());
				TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
			}
		}
	}

	// Destroy Minion
	Minion->Destroy();
}

