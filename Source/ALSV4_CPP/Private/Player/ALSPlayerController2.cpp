// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ALSPlayerController2.h"
#include "ALSLogChannels.h"
#include "Player/ALSPlayerState.h"
#include "Character/ALSPawnData.h"
#include "AbilitySystem/ALSAbilitySystemComponent.h"
#include "EngineUtils.h"
#include "ALSGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemGlobals.h"
#include "CommonInputSubsystem.h"
#include "Player/ALSLocalPlayer.h"

AALSPlayerController2::AALSPlayerController2(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AALSPlayerController2::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AALSPlayerController2::BeginPlay()
{
	Super::BeginPlay();
	SetActorHiddenInGame(false);
}

void AALSPlayerController2::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AALSPlayerController2::ReceivedPlayer()
{
	Super::ReceivedPlayer();
}

void AALSPlayerController2::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// If we are auto running then add some player input
	if (GetIsAutoRunning())
	{
		if (APawn* CurrentPawn = GetPawn())
		{
			const FRotator MovementRotation(0.0f, GetControlRotation().Yaw, 0.0f);
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			CurrentPawn->AddMovementInput(MovementDirection, 1.0f);	
		}
	}
}

AALSPlayerState* AALSPlayerController2::GetALSPlayerState() const
{
	return CastChecked<AALSPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

UALSAbilitySystemComponent* AALSPlayerController2::GetALSAbilitySystemComponent() const
{
	const AALSPlayerState* ALSPS = GetALSPlayerState();
	return (ALSPS ? ALSPS->GetALSAbilitySystemComponent() : nullptr);
}

void AALSPlayerController2::OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
}

void AALSPlayerController2::OnPlayerStateChanged()
{
	// Empty, place for derived classes to implement without having to hook all the other events
}

void AALSPlayerController2::BroadcastOnPlayerStateChanged()
{
	OnPlayerStateChanged();

	LastSeenPlayerState = PlayerState;
}

void AALSPlayerController2::InitPlayerState()
{
	Super::InitPlayerState();
	BroadcastOnPlayerStateChanged();
}

void AALSPlayerController2::CleanupPlayerState()
{
	Super::CleanupPlayerState();
	BroadcastOnPlayerStateChanged();
}

void AALSPlayerController2::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BroadcastOnPlayerStateChanged();
}

void AALSPlayerController2::SetPlayer(UPlayer* InPlayer)
{
	Super::SetPlayer(InPlayer);
}

void AALSPlayerController2::PreProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PreProcessInput(DeltaTime, bGamePaused);
}

void AALSPlayerController2::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UALSAbilitySystemComponent* ALSASC = GetALSAbilitySystemComponent())
	{
		ALSASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

// TODO: Fix this
/*
void AALSPlayerController2::OnCameraPenetratingTarget()
{
	bHideViewTargetPawnNextFrame = true;
}*/

void AALSPlayerController2::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	SetIsAutoRunning(false);
}

void AALSPlayerController2::SetIsAutoRunning(const bool bEnabled)
{
	const bool bIsAutoRunning = GetIsAutoRunning();
	if (bEnabled != bIsAutoRunning)
	{
		if (!bEnabled)
		{
			OnEndAutoRun();
		}
		else
		{
			OnStartAutoRun();
		}
	}
}

bool AALSPlayerController2::GetIsAutoRunning() const
{
	bool bIsAutoRunning = false;
	if (const UALSAbilitySystemComponent* ALSASC = GetALSAbilitySystemComponent())
	{
		bIsAutoRunning = ALSASC->GetTagCount(FALSGameplayTags::Get().Status_AutoRunning) > 0;
	}
	return bIsAutoRunning;
}

void AALSPlayerController2::OnStartAutoRun()
{
	if (UALSAbilitySystemComponent* ALSASC = GetALSAbilitySystemComponent())
	{
		ALSASC->SetLooseGameplayTagCount(FALSGameplayTags::Get().Status_AutoRunning, 1);
		K2_OnStartAutoRun();
	}	
}

void AALSPlayerController2::OnEndAutoRun()
{
	if (UALSAbilitySystemComponent* ALSASC = GetALSAbilitySystemComponent())
	{
		ALSASC->SetLooseGameplayTagCount(FALSGameplayTags::Get().Status_AutoRunning, 0);
		K2_OnEndAutoRun();
	}
}

void AALSPlayerController2::UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId)
{
	if (bForceFeedbackEnabled)
	{
		if (const UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(GetLocalPlayer()))
		{
			const ECommonInputType CurrentInputType = CommonInputSubsystem->GetCurrentInputType();
			if (CurrentInputType == ECommonInputType::Gamepad || CurrentInputType == ECommonInputType::Touch)
			{
				InputInterface->SetForceFeedbackChannelValues(ControllerId, ForceFeedbackValues);
				return;
			}
		}
	}
	
	InputInterface->SetForceFeedbackChannelValues(ControllerId, FForceFeedbackValues());
}

void AALSPlayerController2::UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents)
{
	Super::UpdateHiddenComponents(ViewLocation, OutHiddenComponents);

	if (bHideViewTargetPawnNextFrame)
	{
		AActor* const ViewTargetPawn = PlayerCameraManager ? Cast<AActor>(PlayerCameraManager->GetViewTarget()) : nullptr;
		if (ViewTargetPawn)
		{
			// internal helper func to hide all the components
			auto AddToHiddenComponents = [&OutHiddenComponents](const TInlineComponentArray<UPrimitiveComponent*>& InComponents)
			{
				// add every component and all attached children
				for (UPrimitiveComponent* Comp : InComponents)
				{
					if (Comp->IsRegistered())
					{
						OutHiddenComponents.Add(Comp->ComponentId);

						for (USceneComponent* AttachedChild : Comp->GetAttachChildren())
						{
							static FName NAME_NoParentAutoHide(TEXT("NoParentAutoHide"));
							UPrimitiveComponent* AttachChildPC = Cast<UPrimitiveComponent>(AttachedChild);
							if (AttachChildPC && AttachChildPC->IsRegistered() && !AttachChildPC->ComponentTags.Contains(NAME_NoParentAutoHide))
							{
								OutHiddenComponents.Add(AttachChildPC->ComponentId);
							}
						}
					}
				}
			};

			//TODO Solve with an interface.  Gather hidden components or something.
			//TODO Hiding isn't awesome, sometimes you want the effect of a fade out over a proximity, needs to bubble up to designers.

			// hide pawn's components
			TInlineComponentArray<UPrimitiveComponent*> PawnComponents;
			ViewTargetPawn->GetComponents(PawnComponents);
			AddToHiddenComponents(PawnComponents);

			//// hide weapon too
			//if (ViewTargetPawn->CurrentWeapon)
			//{
			//	TInlineComponentArray<UPrimitiveComponent*> WeaponComponents;
			//	ViewTargetPawn->CurrentWeapon->GetComponents(WeaponComponents);
			//	AddToHiddenComponents(WeaponComponents);
			//}
		}

		// we consumed it, reset for next frame
		bHideViewTargetPawnNextFrame = false;
	}
}

void AALSPlayerController2::OnUnPossess()
{
	// Make sure the pawn that is being unpossessed doesn't remain our ASC's avatar actor
	if (APawn* PawnBeingUnpossessed = GetPawn())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerState))
		{
			if (ASC->GetAvatarActor() == PawnBeingUnpossessed)
			{
				ASC->SetAvatarActor(nullptr);
			}
		}
	}

	Super::OnUnPossess();
}

//////////////////////////////////////////////////////////////////////
// AALSReplayPlayerController

void AALSReplayPlayerController::SetPlayer(UPlayer* InPlayer)
{
	Super::SetPlayer(InPlayer);
}
