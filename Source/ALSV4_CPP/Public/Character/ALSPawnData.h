// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Templates/SubclassOf.h"
#include "UObject/UObjectGlobals.h"
#include "ALSPawnData.generated.h"

class APawn;
class UALSAbilitySet;
class UALSInputConfig;
class UALSAbilityTagRelationshipMapping;
class UALSCameraMode;


/**
 * UALSPawnData
 *
 *	Non-mutable data asset that contains properties used to define a pawn.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "ALS Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class ALSV4_CPP_API UALSPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UALSPawnData(const FObjectInitializer& ObjectInitializer);

public:

	// Class to instantiate for this pawn (should usually derive from AALSPawn or AALSCharacter).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|Pawn")
	TSubclassOf<APawn> PawnClass;

	// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|Abilities")
	TArray<UALSAbilitySet*> AbilitySets;

	// What mapping of ability tags to use for actions taking by this pawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|Abilities")
	UALSAbilityTagRelationshipMapping* TagRelationshipMapping;

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|Input")
	UALSInputConfig* InputConfig;

	// Default camera mode used by player controlled pawns.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|Camera")
	TSubclassOf<UALSCameraMode> DefaultCameraMode;
};
