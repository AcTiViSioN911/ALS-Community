// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community


#include "Character/ALSCharacter.h"

#include "ALSGameplayTags.h"
#include "AbilitySystem/ALSAbilitySystemComponent.h"
#include "Engine/StaticMesh.h"
#include "AI/ALSAIController.h"
#include "Character/ALSPawnExtensionComponent.h"
#include "Kismet/GameplayStatics.h"

AALSCharacter::AALSCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HeldObjectRoot = CreateDefaultSubobject<USceneComponent>(TEXT("HeldObjectRoot"));
	HeldObjectRoot->SetupAttachment(GetMesh());

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(HeldObjectRoot);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(HeldObjectRoot);

	PawnExtComponent = CreateDefaultSubobject<UALSPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	PawnExtComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));

	AIControllerClass = AALSAIController::StaticClass();
}

UALSAbilitySystemComponent* AALSCharacter::GetALSAbilitySystemComponent() const
{
	return Cast<UALSAbilitySystemComponent>(GetAbilitySystemComponent());
}

UAbilitySystemComponent* AALSCharacter::GetAbilitySystemComponent() const
{
	return PawnExtComponent->GetALSAbilitySystemComponent();
}

void AALSCharacter::ClearHeldObject()
{
	StaticMesh->SetStaticMesh(nullptr);
	SkeletalMesh->SetSkeletalMesh(nullptr);
	SkeletalMesh->SetAnimInstanceClass(nullptr);
}

void AALSCharacter::AttachToHand(UStaticMesh* NewStaticMesh, USkeletalMesh* NewSkeletalMesh, UClass* NewAnimClass,
                                 bool bLeftHand, FVector Offset)
{
	ClearHeldObject();

	if (IsValid(NewStaticMesh))
	{
		StaticMesh->SetStaticMesh(NewStaticMesh);
	}
	else if (IsValid(NewSkeletalMesh))
	{
		SkeletalMesh->SetSkeletalMesh(NewSkeletalMesh);
		if (IsValid(NewAnimClass))
		{
			SkeletalMesh->SetAnimInstanceClass(NewAnimClass);
		}
	}

	FName AttachBone;
	if (bLeftHand)
	{
		AttachBone = TEXT("VB LHS_ik_hand_gun");
	}
	else
	{
		AttachBone = TEXT("VB RHS_ik_hand_gun");
	}

	HeldObjectRoot->AttachToComponent(GetMesh(),
	                                  FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachBone);
	HeldObjectRoot->SetRelativeLocation(Offset);
}

void AALSCharacter::RagdollStart()
{
	ClearHeldObject();
	Super::RagdollStart();
}

void AALSCharacter::RagdollEnd()
{
	Super::RagdollEnd();
	UpdateHeldObject();
}

ECollisionChannel AALSCharacter::GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius)
{
	const FName CameraSocketName = bRightShoulder ? TEXT("TP_CameraTrace_R") : TEXT("TP_CameraTrace_L");
	TraceOrigin = GetMesh()->GetSocketLocation(CameraSocketName);
	TraceRadius = 15.0f;
	return ECC_Camera;
}

FTransform AALSCharacter::GetThirdPersonPivotTarget()
{	
	return FTransform(GetActorRotation(),
	                  (GetMesh()->GetSocketLocation(TEXT("Head")) + GetMesh()->GetSocketLocation(TEXT("root"))) / 2.0f,
	                  FVector::OneVector);
}

FTransform AALSCharacter::GetTopDownPivotTarget()
{
	return FTransform(GetActorRotation(),
					  (GetMesh()->GetSocketLocation(TEXT("Head")) + TopDownPivotOffset),
					  FVector::OneVector);
}

FVector AALSCharacter::GetFirstPersonCameraTarget()
{
	return GetMesh()->GetSocketLocation(TEXT("FP_Camera"));
}

void AALSCharacter::OnAbilitySystemInitialized()
{
	UALSAbilitySystemComponent* ALSASC = GetALSAbilitySystemComponent();
	check(ALSASC);
}

void AALSCharacter::OnAbilitySystemUninitialized()
{
	// TODO: Unititialize components
}

void AALSCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	PawnExtComponent->HandleControllerChanged();
}

void AALSCharacter::UnPossessed()
{
	AController* const OldController = Controller;

	Super::UnPossessed();

	PawnExtComponent->HandleControllerChanged();
}

void AALSCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	PawnExtComponent->HandleControllerChanged();
}

void AALSCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	PawnExtComponent->HandlePlayerStateReplicated();
}

void AALSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PawnExtComponent->SetupPlayerInputComponent();
}

void AALSCharacter::InitializeGameplayTags()
{
	// Clear tags that may be lingering on the ability system from the previous pawn.
	if (UALSAbilitySystemComponent* ALSASC = GetALSAbilitySystemComponent())
	{
		const FALSGameplayTags& GameplayTags = FALSGameplayTags::Get();

		for (const TPair<uint8, FGameplayTag>& TagMapping : GameplayTags.MovementModeTagMap)
		{
			if (TagMapping.Value.IsValid())
			{
				ALSASC->SetLooseGameplayTagCount(TagMapping.Value, 0);
			}
		}

		for (const TPair<uint8, FGameplayTag>& TagMapping : GameplayTags.CustomMovementModeTagMap)
		{
			if (TagMapping.Value.IsValid())
			{
				ALSASC->SetLooseGameplayTagCount(TagMapping.Value, 0);
			}
		}

		//UALSCharacterMovementComponent* ALSMoveComp = CastChecked<UALSCharacterMovementComponent>(GetCharacterMovement());
		//SetMovementModeTag(ALSMoveComp->MovementMode, ALSMoveComp->CustomMovementMode, true);
	}
}


void AALSCharacter::OnOverlayStateChanged(EALSOverlayState PreviousState)
{
	Super::OnOverlayStateChanged(PreviousState);
	UpdateHeldObject();
}

void AALSCharacter::OnViewModeChanged(EALSViewMode PreviousViewMode)
{
	Super::OnViewModeChanged(PreviousViewMode);

	if (ViewMode == EALSViewMode::TopDown)
	{
		EnableCursor(true);
	}
	else
	{
		EnableCursor(false);		
	}
}

void AALSCharacter::EnableCursor(bool bEnable)
{
	if (!PlayerController)
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("EanbleCursor: %s"), bEnable ? TEXT("true") : TEXT("false"));

	/*PlayerController->bShowMouseCursor = bEnable;
	PlayerController->bEnableClickEvents = bEnable;
	PlayerController->bEnableMouseOverEvents = bEnable;*/
}

void AALSCharacter::UpdateAimMovement(float DeltaTime)
{
	if (!PlayerController
		|| GetViewMode() != EALSViewMode::TopDown
		|| GetRotationMode() != EALSRotationMode::LookingDirection)
	{
		return;
	}
	
	FVector MouseWorldLocation, MouseWorldDirection;
	const bool SuccessConvert = PlayerController->DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection);

	if (SuccessConvert)
	{
		// Find intersect point with plane originating on actor 
		FVector ActorLocation = GetActorLocation();
		FVector AimTargetLocation = FMath::LinePlaneIntersection(
					MouseWorldLocation, 
					MouseWorldLocation + (MouseWorldDirection * 10000.f), 
					ActorLocation, 
					FVector{ 0.f, 0.f, 1.f });

		// Change actor's yaw rotation
		FRotator ActorRotation = GetActorRotation();
		ActorRotation.Yaw = (AimTargetLocation - ActorLocation).Rotation().Yaw;
		PlayerController->SetControlRotation(ActorRotation);
	}
}

void AALSCharacter::Tick(float DeltaTime)
{
	UpdateAimMovement(DeltaTime);
	
	Super::Tick(DeltaTime);

	UpdateHeldObjectAnimations();
}

void AALSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerController = Cast<APlayerController>(GetController());

	/*if (ViewMode == EALSViewMode::TopDown)
	{
		EnableCursor(true);
	}*/

	UpdateHeldObject();
}
