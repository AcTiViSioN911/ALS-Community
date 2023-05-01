// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "ALSCameraComponent.generated.h"

class UALSCameraMode;

DECLARE_DELEGATE_RetVal(TSubclassOf<UALSCameraMode>, FALSCameraModeDelegate);

/**
 * 
 */
UCLASS()
class ALSV4_CPP_API UALSCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UALSCameraComponent(const FObjectInitializer& ObjectInitializer);

	// Returns the camera component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "ALS|Camera")
	static UALSCameraComponent* FindCameraComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UALSCameraComponent>() : nullptr); }
	
	// Delegate used to query for the best camera mode.
	FALSCameraModeDelegate DetermineCameraModeDelegate;
};
