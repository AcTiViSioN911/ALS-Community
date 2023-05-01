// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "Math/Rotator.h"
#include "Math/Transform.h"
#include "ModularGameMode.h"
#include "UObject/UObjectGlobals.h"
#include "ALSGameMode.generated.h"

class AActor;
class AController;
class AGameModeBase;
class APawn;
class APlayerController;
class UClass;
class UALSExperienceDefinition;
class UALSPawnData;
class UObject;
struct FFrame;
struct FPrimaryAssetId;

/**
 * Post login event, triggered when a player joins the game as well as after non-seamless ServerTravel
 *
 * This is called after the player has finished initialization
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGameModeCombinedPostLogin, AGameModeBase* /*GameMode*/, AController* /*NewPlayer*/);

/**
 * AALSGameMode
 *
 *	The base game mode class used by this project.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class ALSV4_CPP_API AALSGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:

	AALSGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "ALS|Pawn")
	const UALSPawnData* GetPawnDataForController(const AController* InController) const;

	//~AGameModeBase interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	virtual void InitGameState() override;
	//~End of AGameModeBase interface

	FOnGameModeCombinedPostLogin& OnGameModeCombinedPostLogin() { return OnGameModeCombinedPostLoginDelegate; }

	// Restart (respawn) the specified player or bot next frame
	// - If bForceReset is true, the controller will be reset this frame (abandoning the currently possessed pawn, if any)
	UFUNCTION(BlueprintCallable)
	void RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset = false);

	// Agnostic version of PlayerCanRestart that can be used for both player bots and players
	virtual bool ControllerCanRestart(AController* Controller);

private:
	FOnGameModeCombinedPostLogin OnGameModeCombinedPostLoginDelegate;

protected:
	//~AGameModeBase interface
	virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;
	virtual void OnPostLogin(AController* NewPlayer) override;
	virtual void FailedToRestartPlayer(AController* NewPlayer) override;
	//~End of AGameModeBase interface
	
	void OnExperienceLoaded(const UALSExperienceDefinition* CurrentExperience);
	bool IsExperienceLoaded() const;

	void OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource);

	void HandleMatchAssignmentIfNotExpectingOne();
};
