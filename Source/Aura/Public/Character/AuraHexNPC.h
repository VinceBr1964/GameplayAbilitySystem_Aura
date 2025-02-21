// Copyright Vince Bracken

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "AuraHexNPC.generated.h"

class UWidgetComponent;
class UBehaviorTree;
class AHexNPCController;
/**
 * 
 */
UCLASS()
class AURA_API AAuraHexNPC : public AAuraCharacterBase
{
	GENERATED_BODY()
	
public:
	AAuraHexNPC();
	virtual void PossessedBy(AController* NewController) override;

protected:

	UPROPERTY()
	TObjectPtr<AHexNPCController> HexNPCController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> HealthBar;

	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

};
