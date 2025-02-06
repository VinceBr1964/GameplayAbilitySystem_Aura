// Copyright Vince Bracken

#include "AbilitySystem/Abilities/AuraSummonFireMinions.h"
#include "AbilitySystem/Abilities/AuraExplode.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"




FString UAuraSummonFireMinions::GetDescription(int32 Level)
{

	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	if (Level > 5) Level = 5;
	NumMinions = Level;

	if (Level == 1)
	{
		return FString::Printf(TEXT(
			//Title
			"<Title>Summons a Fire Minion:</>\n"

			//Level
			"<Small>Level: </><Level>%d</>\n"
			//ManaCost
			"<Small>Manna Cost: </><ManaCost>%.1f</>\n"
			//Cooldown
			"<Small>Cooldown Duration: </><Cooldown>%.1f</>\n"

			//Description
			"<Default>Spawns an enemy seeking Fire Demon, "
			"that will explode on impact with radial damage.</>"

		),
			//Values
			Level,
			ManaCost,
			Cooldown);
	}
	else
	{
		return FString::Printf(TEXT(
			//Title
			"<Title>Summons Fire Demons:</>\n"

			//Level
			"<Small>Level: </><Level>%d</>\n"
			//ManaCost
			"<Small>Manna Cost: </><ManaCost>%.1f</>\n"
			//Cooldown
			"<Small>Cooldown Duration: </><Cooldown>%.1f</>\n"

			//Number of Minions
			"<Default>Spawns %d enemy seeking Fire Demons, "
			"that will explode on impact with radial damage.</>"

		),
			//Values
			Level,
			ManaCost,
			Cooldown,
			NumMinions);
	}
}

FString UAuraSummonFireMinions::GetNextLevelDescription(int32 Level)
{
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	if (Level > 5) Level = 5;
	NumMinions = Level;

	return FString::Printf(TEXT(
		//Title
		"<Title>NEXT LEVEL:</>\n"

		//Level
		"<Small>Level: </><Level>%d</>\n"
		//ManaCost
		"<Small>Manna Cost: </><ManaCost>%.1f</>\n"
		//Cooldown
		"<Small>Cooldown Duration: </><Cooldown>%.1f</>\n"

		//Number of Minions
		"<Default>Spawns %d enemy seeking Fire Demons, "
		"that will explode on impact with radial damage.</>"

	),
		//Values
		Level,
		ManaCost,
		Cooldown,
		NumMinions);
}
