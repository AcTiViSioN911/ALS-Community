// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/ALSPlayerState.h"
#include "AbilitySystem/ALSAbilitySet.h"
#include "AbilitySystem/ALSAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"
#include "Character/ALSPawnData.h"
#include "Character/ALSPawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "CoreTypes.h"
#include "Delegates/Delegate.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Pawn.h"
#include "GameModes/ALSExperienceManagerComponent.h"
//@TODO: Would like to isolate this a bit better to get the pawn data in here without this having to know about other stuff
#include "GameModes/ALSGameMode.h"
#include "GameplayTagContainer.h"
#include "Logging/LogCategory.h"
#include "Logging/LogMacros.h"
#include "ALSLogChannels.h"
#include "Player/ALSPlayerController2.h"
#include "Misc/AssertionMacros.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Net/UnrealNetwork.h"
#include "Trace/Detail/Channel.h"
#include "UObject/NameTypes.h"
#include "UObject/UObjectBaseUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ALSPlayerState)

class AController;
class APlayerState;
class FLifetimeProperty;

const FName AALSPlayerState::NAME_ALSAbilityReady("ALSAbilitiesReady");

AALSPlayerState::AALSPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MyPlayerConnectionType(EALSPlayerConnectionType::Player)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UALSAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// AbilitySystemComponent needs to be updated at a high frequency.
	NetUpdateFrequency = 100.0f;
}

void AALSPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AALSPlayerState::Reset()
{
	Super::Reset();
}

void AALSPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);

	if (UALSPawnExtensionComponent* PawnExtComp = UALSPawnExtensionComponent::FindPawnExtensionComponent(GetPawn()))
	{
		PawnExtComp->CheckDefaultInitialization();
	}
}

void AALSPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	//@TODO: Copy stats
}

void AALSPlayerState::OnDeactivated()
{
	bool bDestroyDeactivatedPlayerState = false;

	switch (GetPlayerConnectionType())
	{
		case EALSPlayerConnectionType::Player:
		case EALSPlayerConnectionType::InactivePlayer:
			//@TODO: Ask the experience if we should destroy disconnecting players immediately or leave them around
			// (e.g., for long running servers where they might build up if lots of players cycle through)
			bDestroyDeactivatedPlayerState = true;
			break;
		default:
			bDestroyDeactivatedPlayerState = true;
			break;
	}
	
	SetPlayerConnectionType(EALSPlayerConnectionType::InactivePlayer);

	if (bDestroyDeactivatedPlayerState)
	{
		Destroy();
	}
}

void AALSPlayerState::OnReactivated()
{
	if (GetPlayerConnectionType() == EALSPlayerConnectionType::InactivePlayer)
	{
		SetPlayerConnectionType(EALSPlayerConnectionType::Player);
	}
}

void AALSPlayerState::OnExperienceLoaded(const UALSExperienceDefinition* /*CurrentExperience*/)
{
	UE_LOG(LogTemp, Warning, TEXT("AALSPlayerState::OnExperienceLoaded()"))
	if (AALSGameMode* ALSGameMode = GetWorld()->GetAuthGameMode<AALSGameMode>())
	{
		if (const UALSPawnData* NewPawnData = ALSGameMode->GetPawnDataForController(GetOwningController()))
		{
			SetPawnData(NewPawnData);
		}
		else
		{
			UE_LOG(LogALS, Error, TEXT("AALSPlayerState::OnExperienceLoaded(): Unable to find PawnData to initialize player state [%s]!"), *GetNameSafe(this));
		}
	}
}

void AALSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyPlayerConnectionType, SharedParams)

	DOREPLIFETIME(ThisClass, StatTags);
}

AALSPlayerController2* AALSPlayerState::GetALSPlayerController() const
{
	return Cast<AALSPlayerController2>(GetOwner());
}

UAbilitySystemComponent* AALSPlayerState::GetAbilitySystemComponent() const
{
	return GetALSAbilitySystemComponent();
}

void AALSPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());

	if (GetNetMode() != NM_Client)
	{
		AGameStateBase* GameState = GetWorld()->GetGameState();
		check(GameState);
		UALSExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UALSExperienceManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnALSExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	}
}

void AALSPlayerState::SetPawnData(const UALSPawnData* InPawnData)
{
	check(InPawnData);

	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (PawnData)
	{
		UE_LOG(LogALS, Error, TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	PawnData = InPawnData;

	for (const UALSAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
		}
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_ALSAbilityReady);
	
	ForceNetUpdate();
}

void AALSPlayerState::OnRep_PawnData()
{
}

void AALSPlayerState::SetPlayerConnectionType(EALSPlayerConnectionType NewType)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyPlayerConnectionType, this);
	MyPlayerConnectionType = NewType;
}

void AALSPlayerState::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

void AALSPlayerState::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
}

int32 AALSPlayerState::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

bool AALSPlayerState::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}

void AALSPlayerState::ClientBroadcastMessage_Implementation(const FALSVerbMessage Message)
{
	// This check is needed to prevent running the action when in standalone mode
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
	}
}

