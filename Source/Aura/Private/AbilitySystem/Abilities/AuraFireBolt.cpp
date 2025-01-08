// Copyright Vince Bracken


#include "AbilitySystem/Abilities/AuraFireBolt.h"
#include "Aura/Public/AuraGameplayTags.h"


FString UAuraFireBolt::GetDescription(int32 Level)
{
	const int32 Damage = GetDamageByDamageType(Level, FAuraGameplayTags::Get().Damage_Fire);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	if (Level == 1)
	{
		return FString::Printf(TEXT(
			//Title
			"<Title>FIRE BOLT:</>\n"

			//Level
			"<Small>Level: </><Level>%d</>\n"
			//ManaCost
			"<Small>Manna Cost: </><ManaCost>%.1f</>\n"
			//Cooldown
			"<Small>Cooldown Duration: </><Cooldown>%.1f</>\n"
			//Damage
			"<Small>Damage: </><Damage>%d</>\n\n"

			//Description
			"<Default>Launches a bolt of fire, "
			"exploding on impact and dealing: </>"

			//Damage
			"<Damage>%d</><Default> fire damage with"
			" a chance to burn</>"
		),
			//Values
			Level,
			ManaCost,
			Cooldown,
			Damage,
			Damage);
	}
	else
	{
		return FString::Printf(TEXT(
			//Title
			"<Title>FIRE BOLT:</>\n"

			//Level
			"<Small>Level: </><Level>%d</>\n"
			//ManaCost
			"<Small>Manna Cost: </><ManaCost>%.1f</>\n"
			//Cooldown
			"<Small>Cooldown Duration: </><Cooldown>%.1f</>\n"
			//Damage
			"<Small>Damage: </><Damage>%d</>\n\n"

			//Number of Firebolts
			"<Default>Launches %d bolts of fire, "
			"exploding on impact and dealing: </>"

			//Damage
			"<Damage>%d</><Default> fire damage with"
			" a chance to burn</>"
		),
			//Values
			Level,
			ManaCost,
			Cooldown,
			Damage,
			FMath::Min(Level, NumProjectiles),
			Damage);
	}
}

FString UAuraFireBolt::GetNextLevelDescription(int32 Level)
{
	const int32 Damage = GetDamageByDamageType(Level, FAuraGameplayTags::Get().Damage_Fire);
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

		//Number of Firebolts
		"<Default>Launches %d bolts of fire, "
		"exploding on impact and dealing: </>"

		//Damage
		"<Damage>%d</><Default> fire damage with"
		" a chance to burn</>"
	),
		//Values
		Level,
		ManaCost,
		Cooldown,
		Damage,
		FMath::Min(Level, NumProjectiles),
		Damage);
}