// Copyright Vince Bracken
// Copyright Druid Mechanics


#include "Player/AuraPlayerController.h"
#include "HexTileSystem/AHexTile.h" 
#include "HexTileSystem/HexGridManager.h"
#include "EngineUtils.h" 

#include "EnhancedInputSubsystems.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include <AbilitySystemBlueprintLibrary.h>
#include "AuraGameplayTags.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Gameframework/Character.h"
#include "GameFramework/Pawn.h"
#include "UI/Widget/DamageTextComponent.h"
#include "Interaction/HighlightInterface.h"
#include "NiagaraFunctionLibrary.h"
// Added by ChatGPT
#include "Character/AuraCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
// end added by GPT
#include "Actor/MagicCircle.h"
#include "Components/DecalComponent.h"
#include "Aura/Aura.h"


AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	CursorTrace();
	AutoRun();
	UpdateMagicCircleLocation();

	//New: Reveal hexes only when entering a new hex
	AHexTile* CurrentHex = GetHexUnderPlayer();
	AHexGridManager* GridManager = nullptr;
	for (TActorIterator<AHexGridManager> It(GetWorld()); It; ++It)
	{
		GridManager = *It;
		break;  // Stop after finding the first one
	}
	if (CurrentHex && CurrentHex != LastRevealedHex) // Only reveal if changed
	{
		GridManager->RevealHexAndNeighbors(CurrentHex);
		LastRevealedHex = CurrentHex; // Store the last revealed hex
	}

}

void AAuraPlayerController::UpdateMagicCircleLocation()
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->SetActorLocation(CursorHit.ImpactPoint);

	}
}

void AAuraPlayerController::HighlightActor(AActor* InActor)
{
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_HighlightActor(InActor);
	}
}

void AAuraPlayerController::UnHighlightActor(AActor* InActor)
{
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_UnHighlightActor(InActor);
	}
}


void AAuraPlayerController::ShowMagicCircle(UMaterialInterface* DecalMaterial)
{
	if (!IsValid(MagicCircle))
	{
		MagicCircle = GetWorld()->SpawnActor<AMagicCircle>(MagicCircleClass);
		if (DecalMaterial)
		{
			MagicCircle->MagicCircleDecal->SetMaterial(0, DecalMaterial);
		}
	}


}

void AAuraPlayerController::HideMagicCircle()
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->Destroy();
	}

}

void AAuraPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)
{
	if (IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController())
	{
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);

	}
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
}

void AAuraPlayerController::CursorTrace()
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_CursorTrace))
	{
		UnHighlightActor(LastActor);
		UnHighlightActor(ThisActor);
		if (IsValid(ThisActor) && ThisActor->Implements<UHighlightInterface>())

		LastActor = nullptr;
		ThisActor = nullptr;
		return;
	}
	const ECollisionChannel TraceChannel = IsValid(MagicCircle) ? ECC_ExcludePlayers : ECC_Visibility;
	GetHitResultUnderCursor(TraceChannel, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	
	LastActor = ThisActor;

	if (IsValid(CursorHit.GetActor()) && CursorHit.GetActor()->Implements<UHighlightInterface>())
	{
		ThisActor = CursorHit.GetActor();
	}
	else
	{
		ThisActor = nullptr;
	}
	
	if (LastActor != ThisActor)
	{
		UnHighlightActor(LastActor);
		HighlightActor(ThisActor);
	}
}


void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
	{
		return;
	}
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (IsValid(ThisActor))
		{
			TargetingStatus = ThisActor->Implements<UEnemyInterface>() ? ETargetingStatus::TargetingEnemy : ETargetingStatus::TargetingNonEnemy;
		}
		else
		{
			TargetingStatus = ETargetingStatus::NotTargeting;
		}
		bAutoRunning = false;
	}
	if (GetASC()) GetASC()->AbilityInputTagPressed(InputTag);

}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputReleased))
	{
		return;
	}
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
		return;
	}

	if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);

	if (TargetingStatus != ETargetingStatus::TargetingEnemy && !bShiftKeyDown)
	{
		const APawn* ControlledPawn = GetPawn();
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			if (IsValid(ThisActor) && ThisActor->Implements<UHighlightInterface>())
			{
				IHighlightInterface::Execute_SetMoveToLocation(ThisActor, CachedDestination);
			}
			else if (GetASC() && !GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, CachedDestination);
			}
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				Spline->ClearSplinePoints();
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
				}
				if (NavPath->PathPoints.Num() > 0)
				{
					CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
					bAutoRunning = true;
				}
			}


		}
		FollowTime = 0.f;
		TargetingStatus = ETargetingStatus::NotTargeting;
	}
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputHeld))
	{
		return;
	}
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
		return;
	}

	if (TargetingStatus == ETargetingStatus::TargetingEnemy || bShiftKeyDown)
	{
		if (GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
	}
	else
	{
		FollowTime += GetWorld()->GetDeltaSeconds();
		if (CursorHit.bBlockingHit) CachedDestination = CursorHit.ImpactPoint;
	
		if (APawn* ControlledPawn = GetPawn())
		{
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}

	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AuraAbilitySystemComponent;
}



void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);


	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}

	// Get a reference to HexGridManager
	for (TActorIterator<AHexGridManager> It(GetWorld()); It; ++It)
	{
		HexGridManager = *It;
		break; // Stop at the first found instance
	}

	if (HexGridManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("HexGridManager successfully assigned in AuraPlayerController"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HexGridManager is NULL in AuraPlayerController!"));
	}
	
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAuraPlayerController::ShiftPressed);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAuraPlayerController::ShiftReleased);

	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
	//Code added by GPT
	// Binds Input Tags for abilities (LMB, etc.)
	AuraInputComponent->BindAbilityActions(
		InputConfig,
		this,
		&ThisClass::AbilityInputTagPressed,
		&ThisClass::AbilityInputTagReleased,
		&ThisClass::AbilityInputTagHeld
	);
	// ------------------------------------------------------------------
	// *** CAMERA CONTROL ADDITIONS ***
	// ------------------------------------------------------------------
	// 1) Bind the "toggle" (MMB pressed/released):
	if (CameraRotateToggleAction)
	{
		AuraInputComponent->BindAction(
			CameraRotateToggleAction,
			ETriggerEvent::Started,
			this,
			&AAuraPlayerController::OnMiddleMousePressed
		);
		AuraInputComponent->BindAction(
			CameraRotateToggleAction,
			ETriggerEvent::Completed,
			this,
			&AAuraPlayerController::OnMiddleMouseReleased
		);
	}

	// 2) Bind camera rotation axis (driven by Mouse X):
	if (CameraRotateAction)
	{
		AuraInputComponent->BindAction(
			CameraRotateAction,
			ETriggerEvent::Triggered,
			this,
			&AAuraPlayerController::RotateCamera
		);
	}

	// 3) Bind camera zoom axis (driven by Mouse Wheel):
	if (CameraZoomAction)
	{
		AuraInputComponent->BindAction(
			CameraZoomAction,
			ETriggerEvent::Triggered,
			this,
			&AAuraPlayerController::ZoomCamera
		);
	}

	// ------------------------------------------------------------------
	// *** MOVEMENT MODE SWITCH (M Key) ***
	// ------------------------------------------------------------------
	if (IA_M)
	{
		AuraInputComponent->BindAction(
			IA_M,
			ETriggerEvent::Started,
			this,
			&AAuraPlayerController::ToggleHexMovementMode
		);
	}
}

void AAuraPlayerController::ToggleHexMovementMode()
{
	UE_LOG(LogTemp, Warning, TEXT("M Key Pressed - Attempting to Switch Movement Mode"));

	if (CurrentMovementMode == EPlayerMovementMode::FreeMovement)
	{
		CurrentMovementMode = EPlayerMovementMode::HexMovement;
		UE_LOG(LogTemp, Warning, TEXT("Hex Movement Mode Activated"));

		if (HexGridManager)
		{
			APawn* PlayerPawn = GetPawn();
			if (PlayerPawn)
			{
				AHexTile* StartTile = HexGridManager->GetHexTileAtLocation(PlayerPawn->GetActorLocation());
				HexGridManager->ShowMovementRange(StartTile, 2); // Example movement range
			}
		}
	}
	else
	{
		CurrentMovementMode = EPlayerMovementMode::FreeMovement;
		UE_LOG(LogTemp, Warning, TEXT("Free Movement Mode Activated"));

		if (HexGridManager)
		{
			HexGridManager->ClearMovementRange();
		}
	}
}



//Code Added By GPT
void AAuraPlayerController::OnMiddleMousePressed()
{
	bRotateCamera = true;
	// Optionally hide the mouse cursor while rotating:
	bShowMouseCursor = false;
}

void AAuraPlayerController::OnMiddleMouseReleased()
{
	bRotateCamera = false;
	// Show the mouse cursor again:
	bShowMouseCursor = true;
}

void AAuraPlayerController::RotateCamera(const FInputActionValue& Value)
{
	if (!bRotateCamera) return;

	const float AxisValue = Value.Get<float>();
	if (FMath::IsNearlyZero(AxisValue, KINDA_SMALL_NUMBER)) return;

	if (AAuraCharacter* MyCharacter = Cast<AAuraCharacter>(GetPawn()))
	{
		if (MyCharacter->CameraBoom)
		{
			// Current rotation
			FRotator CurrentRotation = MyCharacter->CameraBoom->GetComponentRotation();

			// Desired rotation
			FRotator TargetRotation = CurrentRotation;
			TargetRotation.Yaw += AxisValue * 50.0f; // Example: tweak this speed as desired

			// Interp speed
			float InterpSpeed = 8.0f; // Tune this for the 'feel' of smoothing

			// Delta time
			float DeltaTime = GetWorld()->GetDeltaSeconds();

			// Interpolate
			FRotator SmoothRotation = FMath::RInterpTo(
				CurrentRotation,
				TargetRotation,
				DeltaTime,
				InterpSpeed
			);

			MyCharacter->CameraBoom->SetWorldRotation(SmoothRotation);
		}
	}
}


void AAuraPlayerController::ZoomCamera(const FInputActionValue& Value)
{
	const float AxisValue = Value.Get<float>();
	if (FMath::IsNearlyZero(AxisValue, KINDA_SMALL_NUMBER)) return;

	// Grab the possessed AuraCharacter and zoom via CameraBoom->TargetArmLength
	if (AAuraCharacter* MyCharacter = Cast<AAuraCharacter>(GetPawn()))
	{
		if (MyCharacter->CameraBoom)
		{
			float DesiredArmLength = MyCharacter->CameraBoom->TargetArmLength - (AxisValue * 100.f);
			// Clamp so we don’t zoom too close/far
			DesiredArmLength = FMath::Clamp(DesiredArmLength, 300.f, 2000.f);
			MyCharacter->CameraBoom->TargetArmLength = DesiredArmLength;
		}
	}
}
//Ends GPT code adds

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
	{
		return;
	}
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

AHexTile* AAuraPlayerController::GetHexUnderPlayer()
{
	if (!GetPawn()) return nullptr;

	FVector PlayerLocation = GetPawn()->GetActorLocation();
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetPawn()); // Ignore the player itself

	if (GetWorld()->LineTraceSingleByChannel(
		HitResult,
		PlayerLocation + FVector(0, 0, 100),  // Start above the player
		PlayerLocation - FVector(0, 0, 200), // Trace downward
		ECC_Visibility,
		QueryParams
	))
	{
		return Cast<AHexTile>(HitResult.GetActor());
	}

	return nullptr;
}


