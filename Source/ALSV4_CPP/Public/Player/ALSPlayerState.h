// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "HAL/Platform.h"
#include "Messages/ALSVerbMessage.h"
#include "ModularPlayerState.h"
#include "System/GameplayTagStack.h"
#include "Templates/Casts.h"
#include "UObject/UObjectGlobals.h"

#include "ALSPlayerState.generated.h"

class AController;
class AALSPlayerController2;
class APlayerState;
class FName;
class UAbilitySystemComponent;
class UALSAbilitySystemComponent;
class UALSExperienceDefinition;
class UALSPawnData;
class UObject;
struct FFrame;
struct FGameplayTag;

/** Defines the types of client connected */
UENUM()
enum class EALSPlayerConnectionType : uint8
{
	// An active player
	Player = 0,

	// Spectator connected to a running game
	LiveSpectator,

	// Spectating a demo recording offline
	ReplaySpectator,

	// A deactivated player (disconnected)
	InactivePlayer
};

/**
 * AALSPlayerState
 *
 *	Base player state class used by this project.
 */
UCLASS(Config=Game)
class ALSV4_CPP_API AALSPlayerState : public AModularPlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AALSPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "ALS|PlayerState")
	AALSPlayerController2* GetALSPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "ALS|PlayerState")
	UALSAbilitySystemComponent* GetALSAbilitySystemComponent() const { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	void SetPawnData(const UALSPawnData* InPawnData);

	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	//~End of AActor interface

	//~APlayerState interface
	virtual void Reset() override;
	virtual void ClientInitialize(AController* C) override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OnDeactivated() override;
	virtual void OnReactivated() override;
	//~End of APlayerState interface

	static const FName NAME_ALSAbilityReady;

	void SetPlayerConnectionType(EALSPlayerConnectionType NewType);
	EALSPlayerConnectionType GetPlayerConnectionType() const { return MyPlayerConnectionType; }

	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	UFUNCTION(BlueprintCallable, Category=Teams)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// Returns true if there is at least one stack of the specified tag
	UFUNCTION(BlueprintCallable, Category=Teams)
	bool HasStatTag(FGameplayTag Tag) const;

	// Send a message to just this player
	// (use only for client notifications like accolades, quest toasts, etc... that can handle being occasionally lost)
	UFUNCTION(Client, Unreliable, BlueprintCallable, Category = "ALS|PlayerState")
	void ClientBroadcastMessage(const FALSVerbMessage Message);

private:
	void OnExperienceLoaded(const UALSExperienceDefinition* CurrentExperience);

protected:
	UFUNCTION()
	void OnRep_PawnData();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<const UALSPawnData> PawnData;

private:

	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "ALS|PlayerState")
	TObjectPtr<UALSAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Replicated)
	EALSPlayerConnectionType MyPlayerConnectionType;

	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;
};
