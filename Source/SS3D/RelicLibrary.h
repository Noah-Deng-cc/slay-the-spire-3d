#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.h"

class SS3D_API FRelicLibrary
{
public:
    static TArray<FRelicData> GetAllRelics();
    static bool TryGetRelic(const FString& Id, FRelicData& OutRelic);
};
