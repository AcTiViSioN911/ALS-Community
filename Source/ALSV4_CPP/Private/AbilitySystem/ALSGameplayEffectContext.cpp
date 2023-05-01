#include "AbilitySystem/ALSGameplayEffectContext.h"
#include "Components/PrimitiveComponent.h"
#include "AbilitySystem/ALSAbilitySourceInterface.h"

FALSGameplayEffectContext* FALSGameplayEffectContext::ExtractEffectContext(struct FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* BaseEffectContext = Handle.Get();
	if ((BaseEffectContext != nullptr) && BaseEffectContext->GetScriptStruct()->IsChildOf(FALSGameplayEffectContext::StaticStruct()))
	{
		return (FALSGameplayEffectContext*)BaseEffectContext;
	}

	return nullptr;
}

bool FALSGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);

	// Not serialized for post-activation use:
	// CartridgeID

	return true;
}

void FALSGameplayEffectContext::SetAbilitySource(const IALSAbilitySourceInterface* InObject, float InSourceLevel)
{
	AbilitySourceObject = MakeWeakObjectPtr(Cast<const UObject>(InObject));
	//SourceLevel = InSourceLevel;
}

const IALSAbilitySourceInterface* FALSGameplayEffectContext::GetAbilitySource() const
{
	return Cast<IALSAbilitySourceInterface>(AbilitySourceObject.Get());
}

const UPhysicalMaterial* FALSGameplayEffectContext::GetPhysicalMaterial() const
{
	if (const FHitResult* HitResultPtr = GetHitResult())
	{
		return HitResultPtr->PhysMaterial.Get();
	}
	return nullptr;
}