// Copyright Vince Bracken



#include "AbilitySystem/Abilities/Electrocute.h"


FString UElectrocute::GetDescription(int32 Level)
{

	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	if (Level == 1)
	{
		return FString::Printf(TEXT(
			//Title
			"<Title>ELECTROCUTE:</>\n"

			//Level
			"<Small>Level: </><Level>%d</>\n"
			//ManaCost
			"<Small>Manna Cost: </><ManaCost>%.1f</>\n"
			//Cooldown
			"<Small>Cooldown Duration: </><Cooldown>%.1f</>\n"
			//Damage
			"<Small>Damage: </><Damage>%d</>\n\n"

			//Description
			"<Default>Emits a ligthning beam, "
			"shocking the target: </>"

			//Damage
			"<Damage>%d</><Default> lightning damage with"
			" a chance to stun</>"
		),
			//Values
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage,
			ScaledDamage);
	}
	else
	{
		return FString::Printf(TEXT(
			//Title
			"<Title>Electric Beam:</>\n"

			//Level
			"<Small>Level: </><Level>%d</>\n"
			//ManaCost
			"<Small>Manna Cost: </><ManaCost>%.1f</>\n"
			//Cooldown
			"<Small>Cooldown Duration: </><Cooldown>%.1f</>\n"
			//Damage
			"<Small>Damage: </><Damage>%d</>\n\n"

			//Number of Beams
			"<Default>Emits %d beams of lightning, "
			"dealing: </>"

			//Damage
			"<Damage>%d</><Default> lightning damage with"
			" a chance to stun</>"
		),
			//Values
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage,
			FMath::Min(Level, MaxNumShockTargets),
			ScaledDamage);
	}
}

FString UElectrocute::GetNextLevelDescription(int32 Level)
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

		//Number of Beams
		"<Default>Emits %d beams of lightning, "
		"dealing: </>"

		//Damage
		"<Damage>%d</><Default> lightning damage with"
		" a chance to stun</>"
	),
		//Values
		Level,
		ManaCost,
		Cooldown,
		ScaledDamage,
		FMath::Min(Level, MaxNumShockTargets - 1),
		ScaledDamage);
}
