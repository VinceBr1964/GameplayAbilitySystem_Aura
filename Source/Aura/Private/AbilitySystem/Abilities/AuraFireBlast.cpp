// Copyright Vince Bracken


#include "AbilitySystem/Abilities/AuraFireBlast.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraFireBall.h"

FString UAuraFireBlast::GetDescription(int32 Level)
{

	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	return FString::Printf(TEXT(
		//Title
		"<Title>FIRE BLAST:</>\n"

		//Level
		"<Small>Level: </><Level>%d</>\n"
		//ManaCost
		"<Small>Manna Cost: </><ManaCost>%.1f</>\n"
		//Cooldown
		"<Small>Cooldown Duration: </><Cooldown>%.1f</>\n"
		//Damage
		"<Small>Damage: </><Damage>%d</>\n\n"
		//Number of Fireballs
		"<Default>Launches: %d </>"
		"<Default>fireballs in all directions.</>"
		"Exploding on return for radial damage. </>"

		//Damage
		"<Damage>%d</><Default> fire damage with"
		" a chance to burn</>"),
		
		//Values
		Level,
		ManaCost,
		Cooldown,
		ScaledDamage,
		NumFireBalls,
		ScaledDamage);
}

FString UAuraFireBlast::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	return FString::Printf(TEXT(
		//Title
		"<Title>NEXT LEVEL:</>\n"

		//Level
		"<Small>Level: </><Level>%d</>\n"
		//ManaCost
		"<Small>Manna Cost: </><ManaCost>%.1f</>\n"
		//Cooldown
		"<Small>Cooldown Duration: </><Cooldown>%.1f</>\n"
		//Damage
		"<Small>Damage: </><Damage>%d</>\n\n"
		//Number of Fireballs
		"<Default>Launches: %d </>"
		"<Default>fireballs in all directions.</>"
		"Exploding on return for radial damage. </>"

		//Damage
		"<Damage>%d</><Default> fire damage with"
		" a chance to burn</>"),

		//Values
		Level,
		ManaCost,
		Cooldown,
		ScaledDamage,
		NumFireBalls,
		ScaledDamage);
}

TArray<AAuraFireBall*> UAuraFireBlast::SpawnFireBalls()
{
	TArray<AAuraFireBall*> FireBalls;
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	TArray<FRotator> Rotators = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward,FVector::UpVector, 360.f, NumFireBalls);
	
	for (const FRotator& Rotator : Rotators)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(Location);
		SpawnTransform.SetRotation(Rotator.Quaternion());

		AAuraFireBall* FireBall = GetWorld()->SpawnActorDeferred<AAuraFireBall>(
			FireBallClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			CurrentActorInfo->PlayerController->GetPawn(),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		FireBall->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

		FireBalls.Add(FireBall);

		FireBall->FinishSpawning(SpawnTransform);
	}
	return FireBalls;
}
