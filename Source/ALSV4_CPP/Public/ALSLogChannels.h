#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

ALSV4_CPP_API DECLARE_LOG_CATEGORY_EXTERN(LogALS, Log, All);
ALSV4_CPP_API DECLARE_LOG_CATEGORY_EXTERN(LogALSAbilitySystem, Log, All);
ALSV4_CPP_API DECLARE_LOG_CATEGORY_EXTERN(LogALSExperience, Log, All);

ALSV4_CPP_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
