#include "BattleHUD.h"

#include "CombatManager.h"

void USBattleHUD::NativeConstruct()
{
    Super::NativeConstruct();
}

void USBattleHUD::InitializeWithCombatManager(UCombatManager* InCombatManager)
{
    CombatManager = InCombatManager;
}
