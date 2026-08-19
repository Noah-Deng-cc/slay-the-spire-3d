#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatTypes.h"
#include "MapTypes.h"
#include "BattleHUD.generated.h"

class ASS3DGameMode;
class SWidget;
class SOverlay;
class SVerticalBox;
class SHorizontalBox;
class STextBlock;
class SEditableTextBox;
class SBorder;
class SMemoryMapCanvas;

UCLASS()
class SS3D_API USBattleHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeWithGameMode(ASS3DGameMode* InGameMode);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:
    UPROPERTY()
    TObjectPtr<ASS3DGameMode> GameMode;

    TSharedPtr<SOverlay> RootOverlay;
    TSharedPtr<SVerticalBox> MainContent;
    TSharedPtr<SVerticalBox> SideContent;
    TSharedPtr<SHorizontalBox> HandContent;
    TSharedPtr<SMemoryMapCanvas> MapCanvas;
    TSharedPtr<SBorder> HandFrame;
    TSharedPtr<STextBlock> HeaderStatus;
    TSharedPtr<SEditableTextBox> SeedInput;

    void BuildLayout();
    void Refresh();
    void HandleGameStateChanged();
    void ExecuteCommand(const FString& Command);
    void AddText(const TSharedPtr<SVerticalBox>& Parent, const FText& Text, const FLinearColor& Color = FLinearColor::White, int32 FontSize = 16, float BottomPadding = 6.0f);
    void AddButton(const TSharedPtr<SVerticalBox>& Parent, const FText& Label, TFunction<void()> Action, const FLinearColor& Color = FLinearColor(0.10f, 0.24f, 0.34f, 1.0f));
    void AddCardButton(const FCardData& Card, int32 CardIndex);
    void RefreshMapPanel();
    void RefreshMainPanel();
    void RefreshSidePanel();
    void RefreshHandPanel();
    void ShowHome();
    void ShowMap();
    void ShowCombat();
    void ShowRewards();
    void ShowRest();
    void ShowEvent();
    void ShowShop();
    void StartOdette();

    static FText NodeTypeLabel(EMapNodeType Type);
    static FText PhaseLabel(ECombatPhase Phase);
};
