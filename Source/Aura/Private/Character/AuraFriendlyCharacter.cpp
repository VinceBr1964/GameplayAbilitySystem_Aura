// Copyright Vince Bracken


#include "Character/AuraFriendlyCharacter.h"
#include <Character/AuraEnemy.h>
#include "Kismet/GameplayStatics.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "Aura/Aura.h"
#include "UI/Widget/AuraUserWidget.h"
#include "AuraGameplayTags.h"
#include "AI/AuraFriendlyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


AAuraFriendlyCharacter::AAuraFriendlyCharacter()
{
	// Set this character to use our Friendly AI Controller
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = true;


	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());

	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	GetMesh()->MarkRenderStateDirty();
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->MarkRenderStateDirty();

	BaseWalkSpeed = 250.f;
	AIControllerClass = AAuraFriendlyAIController::StaticClass();
}

void AAuraFriendlyCharacter::AssignUniqueTargets()
{
	// Find all available enemies
	TArray<AActor*> AvailableEnemies;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), AvailableEnemies);

	// Find all Fire Minions
	TArray<AActor*> FireMinions;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Friend"), FireMinions);

	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(this, 0);
	AActor* AuraActor = Character;

	AAIController* MinionAIController = Cast<AAIController>(FireMinions[0]->GetInstigatorController());
	UBlackboardComponent* Blackboard = MinionAIController->FindComponentByClass<UBlackboardComponent>();

	if (AvailableEnemies.Num() == 0)
	{
		// No enemies found, assign all minions to follow the Avatar
		for (AActor* Minion : FireMinions)
		{
			MinionAIController = Cast<AAIController>(Minion->GetInstigatorController());
			Blackboard = MinionAIController->FindComponentByClass<UBlackboardComponent>();

			if (Blackboard)
			{
				Blackboard->SetValueAsObject("TargetToFollow", AuraActor); // Assign Avatar as default
				Blackboard->SetValueAsFloat("DistanceToTarget", GetDistanceTo(AuraActor));
				Blackboard->SetValueAsBool("HasTarget", false);
			}
		}
		return;
	}

	//Sorts for closest distance against First Fire Minion
	AActor* FirstMinion = FireMinions[0];
	AvailableEnemies.Sort([&](const AActor& A, const AActor& B)
		{
			return FirstMinion->GetDistanceTo(&A) < FirstMinion->GetDistanceTo(&B);
		});

	AActor* ClosestEnemy = AvailableEnemies[0];

	MinionAIController = Cast<AAIController>(FirstMinion->GetInstigatorController());
	Blackboard = MinionAIController->GetBlackboardComponent();

	float DistanceToEnemy = FirstMinion->GetDistanceTo(ClosestEnemy);
	if (DistanceToEnemy < EnemySightRange)
	{
		Blackboard->SetValueAsObject("TargetToFollow", ClosestEnemy);
		Blackboard->SetValueAsFloat("DistanceToTarget", DistanceToEnemy);
		Blackboard->SetValueAsBool("HasTarget", true);
	}
	else
	{
		Blackboard->SetValueAsObject("TargetToFollow", AuraActor);
		Blackboard->SetValueAsFloat("DistanceToTarget", GetDistanceTo(AuraActor));
		Blackboard->SetValueAsBool("HasTarget", false);
	}

	
	int32 EnemyIndex = 0;
	for (int i = 1; i < FireMinions.Num(); i++)
	{
		MinionAIController = Cast<AAIController>(FireMinions[i]->GetInstigatorController());
		Blackboard = MinionAIController->GetBlackboardComponent();

		Blackboard->SetValueAsObject("TargetToFollow", AuraActor);
		Blackboard->SetValueAsFloat("DistanceToTarget", GetDistanceTo(AuraActor));
		Blackboard->SetValueAsBool("HasTarget", false);
	}
}

void AAuraFriendlyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!HasAuthority()) return;
	AuraFriendlyAIController = Cast<AAuraFriendlyAIController>(NewController);
	AuraFriendlyAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	AuraFriendlyAIController->RunBehaviorTree(BehaviorTree);
	AuraFriendlyAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
	AuraFriendlyAIController->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), CharacterClass != ECharacterClass::Warrior);
}

int32 AAuraFriendlyCharacter::GetPlayerLevel_Implementation()
{
	return Level;
}

void AAuraFriendlyCharacter::Die(const FVector& DeathImpulse)
{
	SetLifeSpan(LifeSpan);
	if (AuraFriendlyAIController) AuraFriendlyAIController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);

	Super::Die(DeathImpulse);
}

void AAuraFriendlyCharacter::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

AActor* AAuraFriendlyCharacter::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

void AAuraFriendlyCharacter::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
	if (AuraFriendlyAIController && AuraFriendlyAIController->GetBlackboardComponent())
	{
		AuraFriendlyAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}

}

void AAuraFriendlyCharacter::BeginPlay()
{
	Super::BeginPlay();


	InitAbilityActorInfo();
	if (HasAuthority())
	{
		UAuraAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);
	}

	if (UAuraUserWidget* AuraUserWidget = Cast<UAuraUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		AuraUserWidget->SetWidgetController(this);
	}

	if (const UAuraAttributeSet* AuraAS = Cast<UAuraAttributeSet>(AttributeSet))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);

		AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(
			this,
			&AAuraFriendlyCharacter::HitReactTagChanged
		);

		OnHealthChanged.Broadcast(AuraAS->GetHealth());
		OnMaxHealthChanged.Broadcast(AuraAS->GetMaxHealth());
	}
}

void AAuraFriendlyCharacter::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
	AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AAuraFriendlyCharacter::StunTagChanged);


	if (HasAuthority())
	{
		InitializeDefaultAttributes();
	}
	OnASCRegistered.Broadcast(AbilitySystemComponent);
}

void AAuraFriendlyCharacter::InitializeDefaultAttributes() const
{
	UAuraAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}

void AAuraFriendlyCharacter::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	Super::StunTagChanged(CallbackTag, NewCount);

	if (AuraFriendlyAIController && AuraFriendlyAIController->GetBlackboardComponent())
	{
		AuraFriendlyAIController->GetBlackboardComponent()->SetValueAsBool(FName("Stunned"), bIsStunned);
	}
}

void AAuraFriendlyCharacter::SetSummoner(AActor* InSummoner)
{
	Summoner = InSummoner;

	if (Summoner)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetSummoner called on %s. Summoner set to: %s"), *GetName(), *Summoner->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SetSummoner called on %s, but Summoner is NULL!"), *GetName());
	}
}

AActor* AAuraFriendlyCharacter::GetSummoner() const
{
	return Summoner;
}

