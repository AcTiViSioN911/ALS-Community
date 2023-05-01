// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "UObject/Interface.h"
#include "ALSPawnComponent.generated.h"


UINTERFACE(BlueprintType)
class ALSV4_CPP_API UALSReadyInterface : public UInterface
{
	GENERATED_BODY()
};

class IALSReadyInterface
{
	GENERATED_BODY()

public:
	virtual bool IsPawnComponentReadyToInitialize() const = 0;
};


/**
 * UALSPawnComponent
 *
 *	An actor component that can be used for adding custom behavior to pawns.
 */
UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class ALSV4_CPP_API UALSPawnComponent : public UPawnComponent, public IALSReadyInterface
{
	GENERATED_BODY()

public:

	UALSPawnComponent(const FObjectInitializer& ObjectInitializer);

	virtual bool IsPawnComponentReadyToInitialize() const override { return true; }
};
