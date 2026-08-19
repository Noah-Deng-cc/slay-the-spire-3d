#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.h"

class SS3D_API FEnemyLibrary
{
public:
    static TArray<FEnemyDefinition> GetActEnemies(int32 ActIndex, bool bElite);
    static FEnemyDefinition GetBoss(int32 ActIndex);
};
