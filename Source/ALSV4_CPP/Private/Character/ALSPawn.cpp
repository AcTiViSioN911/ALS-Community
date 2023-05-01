// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ALSPawn.h"
#include "GameFramework/Controller.h"


AALSPawn::AALSPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AALSPawn::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AALSPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AALSPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AALSPawn::UnPossessed()
{
	Super::UnPossessed();
}
