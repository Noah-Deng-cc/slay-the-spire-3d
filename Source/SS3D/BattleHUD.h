#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatTypes.h"
#include "BattleHUD.generated.h"

class UBorder;
class UButton;
class UCanvasPanel;
class UCombatManager;
class UTextBlock;
class UVerticalBox;

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

    UPROPERTY()
    TObjectPtr<UCanvasPanel> RootCanvas;

    UPROPERTY()
    TObjectPtr<UVerticalBox> HandBox;

    UPROPERTY()
    TObjectPtr<UTextBlock> PlayerHpText;

    UPROPERTY()
    TObjectPtr<UTextBlock> PlayerBlockText;

    UPROPERTY()
    TObjectPtr<UTextBlock> EnergyText;

    UPROPERTY()
    TObjectPtr<UTextBlock> EnemyHpText;

    UPROPERTY()
    TObjectPtr<UTextBlock> EnemyIntentText;

    UPROPERTY()
    TObjectPtr<UTextBlock> PhaseText;

    UPROPERTY()
    TObjectPtr<UTextBlock> TurnText;

    UPROPERTY()
    TObjectPtr<UTextBlock> DeckText;

    UPROPERTY()
    TObjectPtr<UTextBlock> LogText;

    UPROPERTY()
    TObjectPtr<UButton> ActionButton;

    TArray<FText> CombatLog;
    FCombatSnapshot CurrentSnapshot;
    bool bLayoutBuilt = false;

    void BuildLayout();
    void Render(const FCombatSnapshot& Snapshot);
    void RenderHand(const TArray<FCardData>& Hand);
    void RenderActionState(ECombatPhase Phase);
    void AddCombatLog(const FText& Message, int32 Tone);
    void OnCardClicked(int32 HandIndex);

    UFUNCTION()
    void OnActionClicked();

    UTextBlock* AddTextLine(UVerticalBox* Parent, const FText& Text, int32 FontSize, const FLinearColor& Color);
    UVerticalBox* AddPanel(const FAnchors& Anchors, const FMargin& Offsets, const FLinearColor& Color);
    UWidget* AddToCanvas(UWidget* Widget, const FAnchors& Anchors, const FMargin& Offsets);
    static FButtonStyle MakeButtonStyle(const FLinearColor& Color);
    static FText PhaseLabel(ECombatPhase Phase);
    static FText CardTypeLabel(ECardType Type);
};
