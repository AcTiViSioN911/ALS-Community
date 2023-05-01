// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ALSPawnData.h"

UALSPawnData::UALSPawnData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PawnClass = nullptr;
	InputConfig = nullptr;
	DefaultCameraMode = nullptr;
}
