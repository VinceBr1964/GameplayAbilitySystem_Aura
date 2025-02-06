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
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;


};
