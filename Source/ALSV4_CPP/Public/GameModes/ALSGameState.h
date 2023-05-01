// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemInterface.h"
#include "Engine/EngineTypes.h"
#include "Messages/ALSVerbMessage.h"
#include "ModularGameState.h"
#include "UObject/UObjectGlobals.h"

#include "ALSGameState.generated.h"

class APlayerState;
class UAbilitySystemComponent;
class UALSAbilitySystemComponent;
class UALSExperienceManagerComponent;
class UObject;
struct FFrame;

/**
 * AALSGameState
 *
 *	The base game state class used by this project.
 */
UCLASS(Config = Game)
class ALSV4_CPP_API AALSGameState : public AModularGameStateBase, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	AALSGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	float GetServerFPS() const { return ServerFPS; }

	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	//~AGameStateBase interface
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	//~End of AGameStateBase interface

	//~IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface

	UFUNCTION(BlueprintCallable, Category = "ALS|GameState")
	UALSAbilitySystemComponent* GetALSAbilitySystemComponent() const { return AbilitySystemComponent; }

	// Send a message that all clients will (probably) get
	// (use only for client notifications like eliminations, server join messages, etc... that can handle being lost)
	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable, Category = "ALS|GameState")
	void MulticastMessageToClients(const FALSVerbMessage Message);

	// Send a message that all clients will be guaranteed to get
	// (use only for client notifications that cannot handle being lost)
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "ALS|GameState")
	void MulticastReliableMessageToClients(const FALSVerbMessage Message);

private:
	UPROPERTY()
	TObjectPtr<UALSExperienceManagerComponent> ExperienceManagerComponent;

	// The ability system component subobject for game-wide things (primarily gameplay cues)
	UPROPERTY(VisibleAnywhere, Category = "ALS|GameState")
	TObjectPtr<UALSAbilitySystemComponent> AbilitySystemComponent;


protected:

	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(Replicated)
	float ServerFPS;
};
