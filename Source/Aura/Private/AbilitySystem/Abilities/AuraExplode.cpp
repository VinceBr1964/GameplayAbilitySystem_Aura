// Copyright Vince Bracken


#include "AbilitySystem/Abilities/AuraExplode.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"  // For ACharacter
#include "GameFramework/Actor.h"  
#include <PLayer/AuraPlayerController.h>

class UAuraAttributeSet;

void UAuraExplode::Explode(AActor* MinionActor)
{

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(MinionActor);


	// Get the player character from Ability System Component
	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(this, 0);
	AActor* Actor = Character; // Implicitly upcasts ACharacter* to AActor*
 	if (Actor)
	{
		IgnoredActors.Add(Actor);
	}


	TArray<AActor*> HitActors;
	// Get all enemies in range
	UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(
		MinionActor,
		HitActors,
		IgnoredActors,
		200.0f,  // Explosion radius
		MinionActor->GetActorLocation());

	// Apply damage to all targets
	for (AActor* Target : HitActors)
	{
		if (!Target) continue;

		FDamageEffectParams DamageEffectParams = MakeDamageEffectParamsFromClassDefaults(Target);
		FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(DamageEffectParams.DamageGameplayEffectClass, 1.0f);

		if (!DamageSpec.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("DamageSpec is invalid! No damage will be applied."));
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
		if (!TargetASC)
		{
			UE_LOG(LogTemp, Error, TEXT("Target %s has no Ability System Component!"), *Target->GetName());
			continue;
		}

		// Log damage effect spec application
		UE_LOG(LogTemp, Warning, TEXT("Applying Damage Effect to %s"), *Target->GetName());

		// Debug: Check health before applying damage
		// Get the Health attribute using GetNumericAttribute()
		FGameplayAttribute HealthAttribute = UAuraAttributeSet::GetHealthAttribute();
		bool bFound = false;
		float HealthBefore = TargetASC->GetGameplayAttributeValue(HealthAttribute, bFound);

		if (!bFound)
		{
			UE_LOG(LogTemp, Error, TEXT("Health attribute not found on target!"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Target %s Health Before: %f"), *Target->GetName(), HealthBefore);
		}
	
		// stole from Fireball
		const FVector DeathImpulse = Target->GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
		DamageEffectParams.DeathImpulse = DeathImpulse;

		DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
		UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);


		float HealthAfter = TargetASC->GetGameplayAttributeValue(HealthAttribute, bFound);
		UE_LOG(LogTemp, Warning, TEXT("%s Health After: %f"), *Target->GetName(), HealthAfter);
	}

}


