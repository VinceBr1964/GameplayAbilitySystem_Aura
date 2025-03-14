// Copyright Vince Bracken

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include <AI/AuraFriendlyAIController.h>
#include "Interaction/HighlightInterface.h"
#include "UI\WidgetController\OverlayWidgetController.h"
#include "AuraFriendlyCharacter.generated.h"

class UWidgetComponent;
class UBehaviorTree;
class AAuraFriendlyAIController;
/**
 * 
 */
UCLASS()
class AURA_API AAuraFriendlyCharacter : public AAuraCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()

public:
	AAuraFriendlyCharacter();

	void AssignUniqueTargets();
	virtual void PossessedBy(AController* NewController) override;

	/** Combat Interface*/
	virtual int32 GetPlayerLevel_Implementation() override;
	virtual void Die(const FVector& DeathImpulse) override;
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
	virtual AActor* GetCombatTarget_Implementation() const override;
	/** end Combat Interface*/

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnMaxHealthChanged;

	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHitReacting = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float LifeSpan = 5.f;

	void SetLevel(int32 InLevel) { Level = InLevel; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<AActor> CombatTarget;

	// Stores the actor that summoned this minion
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Summoning")
	AActor* Summoner;

	// Setter for Summoner (so we can assign the summoner when spawning)
	UFUNCTION(BlueprintCallable, Category = "Summoning")
	void SetSummoner(AActor* InSummoner);

	// Getter for Summoner (so we can retrieve it in SendXPEvent)
	UFUNCTION(BlueprintPure, Category = "Summoning")
	AActor* GetSummoner() const;

protected:
	virtual void BeginPlay() override;

	virtual void InitAbilityActorInfo() override;
	virtual void InitializeDefaultAttributes() const override;
	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount) override;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	int32 EnemySightRange = 1000;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> HealthBar;

	UPROPERTY()
	TObjectPtr<AAuraFriendlyAIController> AuraFriendlyAIController;
	
	UPROPERTY(EditAnywhere, Category = "FriendlyAI")
	TObjectPtr<UBehaviorTree> BehaviorTree;



private:


};
