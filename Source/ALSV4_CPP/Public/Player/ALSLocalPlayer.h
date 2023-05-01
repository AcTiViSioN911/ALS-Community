// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonLocalPlayer.h"
#include "ALSLocalPlayer.generated.h"

class UInputMappingContext;

/**
 * UALSLocalPlayer
 */
UCLASS()
class ALSV4_CPP_API UALSLocalPlayer : public UCommonLocalPlayer
{
	GENERATED_BODY()

public:

	UALSLocalPlayer();

	//~UPlayer interface
	virtual void SwitchController(class APlayerController* PC) override;
	//~End of UPlayer interface

	//~ULocalPlayer interface
	virtual bool SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld) override;
	virtual void InitOnlineSession() override;
	//~End of ULocalPlayer interface

protected:
	void OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId);
	
	UFUNCTION()
	void OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult);

	virtual void OnPlayerControllerChanged(APlayerController* NewController);

protected:
	UPROPERTY(Transient)
	mutable const UInputMappingContext* InputMappingContext;

	UPROPERTY()
	TWeakObjectPtr<APlayerController> LastBoundPC;
};

