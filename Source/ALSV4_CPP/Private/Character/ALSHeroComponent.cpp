// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ALSHeroComponent.h"
#include "ALSLogChannels.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Player/ALSPlayerController2.h"
#include "Player/ALSPlayerState.h"
#include "Character/ALSPawnExtensionComponent.h"
#include "Character/ALSPawnData.h"
#include "Character/ALSCharacter.h"
#include "AbilitySystem/ALSAbilitySystemComponent.h"
#include "Input/ALSInputConfig.h"
#include "Input/ALSInputComponent.h"
// #include "Camera/ALSCameraComponent.h"
#include "ALSGameplayTags.h"
#include "Engine/LocalPlayer.h"
#include "Camera/ALSCameraMode.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Settings/ALSSettingsLocal.h"
#include "System/ALSAssetManager.h"
#include "PlayerMappableInputConfig.h"
#include "Camera/ALSCameraComponent.h"


#if WITH_EDITOR
#include "Misc/UObjectToken.h"
#endif	// WITH_EDITOR

namespace ALSHero
{
	static const float LookYawRate = 300.0f;
	static const float LookPitchRate = 165.0f;
};

const FName UALSHeroComponent::NAME_BindInputsNow("BindInputsNow");

UALSHeroComponent::UALSHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// AbilityCameraMode = nullptr;
	bPawnHasInitialized = false;
	bReadyToBindInputs = false;
}

void UALSHeroComponent::OnRegister()
{
	Super::OnRegister();

	UE_LOG(LogTemp, Warning, TEXT("UALSHeroComponent::OnRegister()"))

	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (UALSPawnExtensionComponent* PawnExtComp = UALSPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			UE_LOG(LogTemp, Warning, TEXT("UALSHeroComponent::FindPawnExtensionComponent()"))
			PawnExtComp->OnPawnReadyToInitialize_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnPawnReadyToInitialize));
		}
	}
	else
	{
		UE_LOG(LogALS, Error, TEXT("[UALSHeroComponent::OnRegister] This component has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint."));

#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = NSLOCTEXT("ALSHeroComponent", "NotOnPawnError", "has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint. This will cause a crash if you PIE!");
			static const FName HeroMessageLogName = TEXT("ALSHeroComponent");
			
			FMessageLog(HeroMessageLogName).Error()
				->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
				->AddToken(FTextToken::Create(Message));
				
			FMessageLog(HeroMessageLogName).Open();
		}
#endif
	}
}

bool UALSHeroComponent::IsPawnComponentReadyToInitialize() const
{
	// The player state is required.
	if (!GetPlayerState<AALSPlayerState>())
	{
		return false;
	}

	const APawn* Pawn = GetPawn<APawn>();

	// A pawn is required.
	if (!Pawn)
	{
		return false;
	}

	// If we're authority or autonomous, we need to wait for a controller with registered ownership of the player state.
	if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
	{
		AController* Controller = GetController<AController>();

		const bool bHasControllerPairedWithPS = (Controller != nullptr) && \
												(Controller->PlayerState != nullptr) && \
												(Controller->PlayerState->GetOwner() == Controller);

		if (!bHasControllerPairedWithPS)
		{
			return false;
		}
	}
	
	const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
	const bool bIsBot = Pawn->IsBotControlled();

	if (bIsLocallyControlled && !bIsBot)
	{
		// The input component is required when locally controlled.
		if (!Pawn->InputComponent)
		{
			return false;
		}
	}

	return true;
}

void UALSHeroComponent::OnPawnReadyToInitialize()
{
	UE_LOG(LogTemp, Warning, TEXT("UALSHeroComponent::OnPawnReadyToInitialize"))
	
	if (!ensure(!bPawnHasInitialized))
	{
		// Don't initialize twice
		return;
	}

	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}
	const bool bIsLocallyControlled = Pawn->IsLocallyControlled();

	AALSPlayerState* ALSPS = GetPlayerState<AALSPlayerState>();
	check(ALSPS);

	const UALSPawnData* PawnData = nullptr;

	if (UALSPawnExtensionComponent* PawnExtComp = UALSPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		PawnData = PawnExtComp->GetPawnData<UALSPawnData>();

		// The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
		// The ability system component and attribute sets live on the player state.
		PawnExtComp->InitializeAbilitySystem(ALSPS->GetALSAbilitySystemComponent(), ALSPS);
	}

	if (AALSPlayerController2* ALSPC = GetController<AALSPlayerController2>())
	{
		if (Pawn->InputComponent != nullptr)
		{
			InitializePlayerInput(Pawn->InputComponent);
		}
	}

	if (bIsLocallyControlled && PawnData)
	{
		if (UALSCameraComponent* CameraComponent = UALSCameraComponent::FindCameraComponent(Pawn))
		{
			CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);
		}
	}

	bPawnHasInitialized = true;
}

void UALSHeroComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UALSHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (UALSPawnExtensionComponent* PawnExtComp = UALSPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			PawnExtComp->UninitializeAbilitySystem();
		}	
	}

	Super::EndPlay(EndPlayReason);
}

void UALSHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	UE_LOG(LogTemp, Warning, TEXT("UALSHeroComponent::InitializePlayerInput"))
	check(PlayerInputComponent);

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	Subsystem->ClearAllMappings();
	
	if (const UALSPawnExtensionComponent* PawnExtComp = UALSPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const UALSPawnData* PawnData = PawnExtComp->GetPawnData<UALSPawnData>())
		{
			if (const UALSInputConfig* InputConfig = PawnData->InputConfig)
			{
				const FALSGameplayTags& GameplayTags = FALSGameplayTags::Get();
	
				// Register any default input configs with the settings so that they will be applied to the player during AddInputMappings
				for (const FMappableConfigPair& Pair : DefaultInputConfigs)
				{
					FMappableConfigPair::ActivatePair(Pair);
				}
				
				UALSInputComponent* ALSIC = CastChecked<UALSInputComponent>(PlayerInputComponent);
				ALSIC->AddInputMappings(InputConfig, Subsystem);
				if (UALSSettingsLocal* LocalSettings = UALSSettingsLocal::Get())
				{
					LocalSettings->OnInputConfigActivated.AddUObject(this, &UALSHeroComponent::OnInputConfigActivated);
					LocalSettings->OnInputConfigDeactivated.AddUObject(this, &UALSHeroComponent::OnInputConfigDeactivated);
				}

				TArray<uint32> BindHandles;
				ALSIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

				ALSIC->BindNativeAction(InputConfig, GameplayTags.InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ true);
				ALSIC->BindNativeAction(InputConfig, GameplayTags.InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ true);
				ALSIC->BindNativeAction(InputConfig, GameplayTags.InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ true);
				ALSIC->BindNativeAction(InputConfig, GameplayTags.InputTag_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch, /*bLogIfNotFound=*/ true);
				ALSIC->BindNativeAction(InputConfig, GameplayTags.InputTag_AutoRun, ETriggerEvent::Triggered, this, &ThisClass::Input_AutoRun, /*bLogIfNotFound=*/ true);
				UE_LOG(LogTemp, Warning, TEXT("UALSHeroComponent::InitializePlayerInput - BindNativeAction"))
			}
		}
	}

	if (ensure(!bReadyToBindInputs))
	{
		bReadyToBindInputs = true;
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC), NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), NAME_BindInputsNow);
}

void UALSHeroComponent::OnInputConfigActivated(const FLoadedMappableConfigPair& ConfigPair)
{
	UE_LOG(LogTemp, Warning, TEXT("UALSHeroComponent::OnInputConfigActivated"))
	if (AALSPlayerController2* ALSPC = GetController<AALSPlayerController2>())
	{
		if (APawn* Pawn = GetPawn<APawn>())
		{
			if (UALSInputComponent* ALSIC = Cast<UALSInputComponent>(Pawn->InputComponent))
			{
				if (const ULocalPlayer* LP = ALSPC->GetLocalPlayer())
				{
					if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
					{
						ALSIC->AddInputConfig(ConfigPair, Subsystem);	
					}	
				}
			}
		}
	}
}

void UALSHeroComponent::OnInputConfigDeactivated(const FLoadedMappableConfigPair& ConfigPair)
{
	if (AALSPlayerController2* ALSPC = GetController<AALSPlayerController2>())
	{
		if (APawn* Pawn = GetPawn<APawn>())
		{
			if (UALSInputComponent* ALSIC = Cast<UALSInputComponent>(Pawn->InputComponent))
			{
				if (const ULocalPlayer* LP = ALSPC->GetLocalPlayer())
				{
					if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
					{
						ALSIC->RemoveInputConfig(ConfigPair, Subsystem);
					}
				}
			}
		}
	}
}

void UALSHeroComponent::AddAdditionalInputConfig(const UALSInputConfig* InputConfig)
{
	TArray<uint32> BindHandles;

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	UALSInputComponent* ALSIC = Pawn->FindComponentByClass<UALSInputComponent>();
	check(ALSIC);

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	if (const UALSPawnExtensionComponent* PawnExtComp = UALSPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		ALSIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);
	}
}

void UALSHeroComponent::RemoveAdditionalInputConfig(const UALSInputConfig* InputConfig)
{
	//@TODO: Implement me!
}

bool UALSHeroComponent::HasPawnInitialized() const
{
	return bPawnHasInitialized;
}

bool UALSHeroComponent::IsReadyToBindInputs() const
{
	return bReadyToBindInputs;
}

void UALSHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const UALSPawnExtensionComponent* PawnExtComp = UALSPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			if (UALSAbilitySystemComponent* ALSASC = PawnExtComp->GetALSAbilitySystemComponent())
			{
				ALSASC->AbilityInputTagPressed(InputTag);
			}
		}	
	}
}

void UALSHeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	if (const UALSPawnExtensionComponent* PawnExtComp = UALSPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (UALSAbilitySystemComponent* ALSASC = PawnExtComp->GetALSAbilitySystemComponent())
		{
			ALSASC->AbilityInputTagReleased(InputTag);
		}
	}
}

void UALSHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	UE_LOG(LogTemp, Warning, TEXT("INPUT_MOOOOVE"))
	
	APawn* Pawn = GetPawn<APawn>();
	AController* Controller = Pawn ? Pawn->GetController() : nullptr;

	// If the player has attempted to move again then cancel auto running
	if (AALSPlayerController2* ALSController = Cast<AALSPlayerController2>(Controller))
	{
		ALSController->SetIsAutoRunning(false);
	}
	
	if (Controller)
	{
		const FVector2D Value = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			Pawn->AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			Pawn->AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

void UALSHeroComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y);
	}
}

void UALSHeroComponent::Input_LookStick(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	const UWorld* World = GetWorld();
	check(World);

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X * ALSHero::LookYawRate * World->GetDeltaSeconds());
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y * ALSHero::LookPitchRate * World->GetDeltaSeconds());
	}
}

void UALSHeroComponent::Input_Crouch(const FInputActionValue& InputActionValue)
{
	UE_LOG(LogTemp, Warning, TEXT("UALSHeroComponent::Input_Crouch"))
	if (AALSCharacter* Character = GetPawn<AALSCharacter>())
	{
		Character->Crouch();
	}
}

void UALSHeroComponent::Input_AutoRun(const FInputActionValue& InputActionValue)
{
	if (APawn* Pawn = GetPawn<APawn>())
	{
		if (AALSPlayerController2* Controller = Cast<AALSPlayerController2>(Pawn->GetController()))
		{
			// Toggle auto running
			Controller->SetIsAutoRunning(!Controller->GetIsAutoRunning());
		}	
	}
}

TSubclassOf<UALSCameraMode> UALSHeroComponent::DetermineCameraMode() const
{
	if (AbilityCameraMode)
	{
		return AbilityCameraMode;
	}

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return nullptr;
	}

	if (UALSPawnExtensionComponent* PawnExtComp = UALSPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const UALSPawnData* PawnData = PawnExtComp->GetPawnData<UALSPawnData>())
		{
			return PawnData->DefaultCameraMode;
		}
	}

	return nullptr;
}

void UALSHeroComponent::SetAbilityCameraMode(TSubclassOf<UALSCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (CameraMode)
	{
		AbilityCameraMode = CameraMode;
		AbilityCameraModeOwningSpecHandle = OwningSpecHandle;
	}
}

void UALSHeroComponent::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (AbilityCameraModeOwningSpecHandle == OwningSpecHandle)
	{
		AbilityCameraMode = nullptr;
		AbilityCameraModeOwningSpecHandle = FGameplayAbilitySpecHandle();
	}
}
