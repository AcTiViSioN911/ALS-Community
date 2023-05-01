// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ALSLocalPlayer.h"
#include "InputMappingContext.h"
#include "AudioMixerBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"

UALSLocalPlayer::UALSLocalPlayer()
{
}

void UALSLocalPlayer::SwitchController(class APlayerController* PC)
{
	Super::SwitchController(PC);

	OnPlayerControllerChanged(PlayerController);
}

bool UALSLocalPlayer::SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld)
{
	const bool bResult = Super::SpawnPlayActor(URL, OutError, InWorld);

	OnPlayerControllerChanged(PlayerController);

	return bResult;
}

void UALSLocalPlayer::InitOnlineSession()
{
	OnPlayerControllerChanged(PlayerController);

	Super::InitOnlineSession();
}

void UALSLocalPlayer::OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId)
{
	FOnCompletedDeviceSwap DevicesSwappedCallback;
	DevicesSwappedCallback.BindUFunction(this, FName("OnCompletedAudioDeviceSwap"));
	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(GetWorld(), InAudioOutputDeviceId, DevicesSwappedCallback);
}

void UALSLocalPlayer::OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult)
{
	if (SwapResult.Result == ESwapAudioOutputDeviceResultState::Failure)
	{
	}
}

void UALSLocalPlayer::OnPlayerControllerChanged(APlayerController* NewController)
{
}

