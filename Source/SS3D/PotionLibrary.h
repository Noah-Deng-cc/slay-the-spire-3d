#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.h"

class SS3D_API FPotionLibrary
{
public:
    static TArray<FPotionData> GetAllPotions();
    static bool TryGetPotion(const FString& Id, FPotionData& OutPotion);
};
