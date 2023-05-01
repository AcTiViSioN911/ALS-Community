// Fill out your copyright notice in the Description page of Project Settings.

// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameModes/ALSExperienceManager.h"
#include "Engine/AssetManager.h"
#include "GameModes/ALSExperienceDefinition.h"
#include "GameModes/ALSExperienceManager.h"
#include "Engine/Engine.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ALSExperienceManager)

#if WITH_EDITOR

void UALSExperienceManager::OnPlayInEditorBegun()
{
	ensure(GameFeaturePluginRequestCountMap.IsEmpty());
	GameFeaturePluginRequestCountMap.Empty();
}

void UALSExperienceManager::NotifyOfPluginActivation(const FString PluginURL)
{
	if (GIsEditor)
	{
		UALSExperienceManager* ExperienceManagerSubsystem = GEngine->GetEngineSubsystem<UALSExperienceManager>();
		check(ExperienceManagerSubsystem);

		// Track the number of requesters who activate this plugin. Multiple load/activation requests are always allowed because concurrent requests are handled.
		int32& Count = ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.FindOrAdd(PluginURL);
		++Count;
	}
}

bool UALSExperienceManager::RequestToDeactivatePlugin(const FString PluginURL)
{
	if (GIsEditor)
	{
		UALSExperienceManager* ExperienceManagerSubsystem = GEngine->GetEngineSubsystem<UALSExperienceManager>();
		check(ExperienceManagerSubsystem);

		// Only let the last requester to get this far deactivate the plugin
		int32& Count = ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.FindChecked(PluginURL);
		--Count;

		if (Count == 0)
		{
			ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.Remove(PluginURL);
			return true;
		}

		return false;
	}

	return true;
}

#endif
