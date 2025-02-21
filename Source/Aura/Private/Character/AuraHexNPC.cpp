// Copyright Vince Bracken


#include "Character/AuraHexNPC.h"
#include <AbilitySystem/AuraAbilitySystemComponent.h>
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include <AbilitySystem/AuraAttributeSet.h>
#include "Components/WidgetComponent.h"
#include <Aura/Aura.h>
#include "UI/Widget/AuraUserWidget.h"
#include "AuraGameplayTags.h"
#include "AI/HexNPCController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <AIController.h>


AAuraHexNPC::AAuraHexNPC()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());

	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	GetMesh()->MarkRenderStateDirty();
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->MarkRenderStateDirty();

	BaseWalkSpeed = 250.f;
}

void AAuraHexNPC::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!HasAuthority()) return;
	HexNPCController = Cast<AHexNPCController>(NewController);
	HexNPCController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	HexNPCController->RunBehaviorTree(BehaviorTree);
}


