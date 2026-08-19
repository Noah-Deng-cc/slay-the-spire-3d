#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.h"

class SS3D_API FCardLibrary
{
public:
    static TArray<FCardData> CreateStarterDeck();
    static TArray<FCardData> GetAllCards();
    static bool TryGetCard(const FString& Id, FCardData& OutCard);

private:
    static FCardData Card(const TCHAR* Id, const TCHAR* Name, int32 Cost, ECardType Type,
        ECardRarity Rarity, const TCHAR* Description, TArray<FCardEffect> Effects);
    static FCardEffect Effect(ECardEffectType Type, int32 Value, bool bExhaust = false);
};
