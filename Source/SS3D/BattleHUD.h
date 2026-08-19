#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleHUD.generated.h"

class UCombatManager;

UCLASS()
class SS3D_API USBattleHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeWithCombatManager(UCombatManager* InCombatManager);

protected:
    virtual void NativeConstruct() override;

private:
    UPROPERTY()
    TObjectPtr<UCombatManager> CombatManager;
};
