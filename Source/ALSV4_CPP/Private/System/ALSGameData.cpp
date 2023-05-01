// Fill out your copyright notice in the Description page of Project Settings.


#include "System/ALSGameData.h"
#include "System/ALSAssetManager.h"

UALSGameData::UALSGameData()
{
}

const UALSGameData& UALSGameData::UALSGameData::Get()
{
	return UALSAssetManager::Get().GetGameData();
}