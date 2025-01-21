// Copyright Vince Bracken


#include "AbilitySystem/Abilities/AuraFireBlast.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"

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