#include "BattleHUD.h"

#include "CombatManager.h"
#include "Styling/CoreStyle.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

namespace SS3DUI
{
    const FLinearColor Panel(0.035f, 0.065f, 0.12f, 0.96f);
    const FLinearColor PanelAlt(0.055f, 0.095f, 0.16f, 0.94f);
    const FLinearColor Cyan(0.32f, 0.78f, 0.96f, 1.0f);
    const FLinearColor Muted(0.55f, 0.66f, 0.75f, 1.0f);
    const FLinearColor Warning(1.0f, 0.50f, 0.38f, 1.0f);
    const FLinearColor Gold(1.0f, 0.80f, 0.32f, 1.0f);
}

void USBattleHUD::NativeConstruct()
{
    Super::NativeConstruct();
    BuildLayout();
}

void USBattleHUD::InitializeWithCombatManager(UCombatManager* InCombatManager)
{
    CombatManager = InCombatManager;
    if (!bLayoutBuilt) BuildLayout();
    if (!CombatManager) return;

    CombatManager->OnCombatStateChanged.AddUObject(this, &USBattleHUD::Render);
    CombatManager->OnCombatLogAdded.AddUObject(this, &USBattleHUD::AddCombatLog);
    Render(CombatManager->GetSnapshot());
}

void USBattleHUD::BuildLayout()
{
    if (bLayoutBuilt) return;
    bLayoutBuilt = true;

    RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
    WidgetTree->RootWidget = RootCanvas;

    AddPanel(FAnchors(0.0f, 1.0f, 1.0f, 1.0f), FMargin(24.0f, -72.0f, -24.0f, -24.0f), SS3DUI::Panel);

    UTextBlock* Title = Cast<UTextBlock>(AddToCanvas(
        WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()),
        FAnchors(0.0f, 1.0f, 0.0f, 1.0f), FMargin(40.0f, -60.0f, 420.0f, -28.0f)));
    Title->SetText(FText::FromString(TEXT("星穹：尖塔  /  BATTLE PROTOCOL")));
    Title->SetColorAndOpacity(SS3DUI::Cyan);
    Title->SetJustification(ETextJustify::Left);
    FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFont();
    TitleFont.Size = 22;
    Title->SetFont(TitleFont);

    UTextBlock* RunInfo = Cast<UTextBlock>(AddToCanvas(
        WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()),
        FAnchors(1.0f, 1.0f, 1.0f, 1.0f), FMargin(-360.0f, -58.0f, -40.0f, -34.0f)));
    RunInfo->SetText(FText::FromString(TEXT("ACT 01  ·  雾海回廊")));
    RunInfo->SetColorAndOpacity(SS3DUI::Muted);
    RunInfo->SetJustification(ETextJustify::Right);

    UTextBlock* AssetStatus = Cast<UTextBlock>(AddToCanvas(
        WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()),
        FAnchors(1.0f, 1.0f, 1.0f, 1.0f), FMargin(-560.0f, -35.0f, -40.0f, -18.0f)));
    AssetStatus->SetText(FText::FromString(TEXT("ASSET STATUS  /  3D VISUALS NOT PROVIDED")));
    AssetStatus->SetColorAndOpacity(SS3DUI::Warning);
    AssetStatus->SetJustification(ETextJustify::Right);

    UVerticalBox* PlayerPanel = AddPanel(FAnchors(0.0f, 1.0f, 0.0f, 1.0f), FMargin(40.0f, -205.0f, 390.0f, -84.0f), SS3DUI::PanelAlt);
    AddTextLine(PlayerPanel, FText::FromString(TEXT("SIGNAL PILOT")), 11, SS3DUI::Muted);
    PlayerHpText = AddTextLine(PlayerPanel, FText::FromString(TEXT("HP  80 / 80")), 22, FLinearColor::White);
    PlayerBlockText = AddTextLine(PlayerPanel, FText::FromString(TEXT("SHIELD  0")), 14, SS3DUI::Cyan);

    UVerticalBox* EnemyPanel = AddPanel(FAnchors(1.0f, 1.0f, 1.0f, 1.0f), FMargin(-430.0f, -205.0f, -40.0f, -84.0f), SS3DUI::PanelAlt);
    AddTextLine(EnemyPanel, FText::FromString(TEXT("RIFT CONSTRUCT  /  LV. 01")), 11, SS3DUI::Muted);
    EnemyHpText = AddTextLine(EnemyPanel, FText::FromString(TEXT("HP  50 / 50")), 22, FLinearColor::White);
    EnemyIntentText = AddTextLine(EnemyPanel, FText::FromString(TEXT("INTENT  8  ·  蓄力攻击")), 14, SS3DUI::Warning);

    UVerticalBox* CenterPanel = AddPanel(FAnchors(0.5f, 1.0f, 0.5f, 1.0f), FMargin(-145.0f, -180.0f, 145.0f, -112.0f), SS3DUI::Panel);
    PhaseText = AddTextLine(CenterPanel, FText::FromString(TEXT("YOUR TURN")), 14, SS3DUI::Cyan);
    TurnText = AddTextLine(CenterPanel, FText::FromString(TEXT("TURN 01")), 11, SS3DUI::Muted);

    UVerticalBox* LogPanel = AddPanel(FAnchors(0.0f, 0.0f, 0.0f, 0.0f), FMargin(40.0f, 104.0f, 400.0f, 292.0f), SS3DUI::Panel);
    AddTextLine(LogPanel, FText::FromString(TEXT("COMBAT TELEMETRY")), 11, SS3DUI::Cyan);
    LogText = AddTextLine(LogPanel, FText::FromString(TEXT("等待战斗数据...")), 13, SS3DUI::Muted);
    LogText->SetAutoWrapText(true);
    LogText->SetJustification(ETextJustify::Left);

    UVerticalBox* DeckPanel = AddPanel(FAnchors(1.0f, 0.0f, 1.0f, 0.0f), FMargin(-330.0f, 104.0f, -40.0f, 292.0f), SS3DUI::Panel);
    AddTextLine(DeckPanel, FText::FromString(TEXT("DECK")), 11, SS3DUI::Cyan);
    DeckText = AddTextLine(DeckPanel, FText::FromString(TEXT("DRAW  00\nDISCARD  00")), 16, FLinearColor::White);

    HandBox = Cast<UVerticalBox>(WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HandBox")));
    AddToCanvas(HandBox, FAnchors(0.5f, 0.0f, 0.5f, 0.0f), FMargin(-560.0f, 320.0f, 560.0f, 520.0f));

    UVerticalBox* ActionPanel = AddPanel(FAnchors(1.0f, 0.0f, 1.0f, 0.0f), FMargin(-330.0f, 40.0f, -40.0f, 88.0f), SS3DUI::Panel);
    UTextBlock* EnergyLabel = AddTextLine(ActionPanel, FText::FromString(TEXT("ENERGY")), 11, SS3DUI::Muted);
    EnergyText = AddTextLine(ActionPanel, FText::FromString(TEXT("3 / 3")), 20, SS3DUI::Gold);
    ActionButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ActionButton"));
    ActionButton->SetStyle(MakeButtonStyle(FLinearColor(0.10f, 0.34f, 0.52f, 1.0f)));
    UTextBlock* ActionLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    ActionLabel->SetText(FText::FromString(TEXT("结束回合  ›")));
    ActionLabel->SetColorAndOpacity(FLinearColor::White);
    ActionLabel->SetJustification(ETextJustify::Center);
    ActionButton->AddChild(ActionLabel);
    UVerticalBoxSlot* ActionSlot = ActionPanel->AddChildToVerticalBox(ActionButton);
    ActionSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));
    ActionButton->OnClicked.AddDynamic(this, &USBattleHUD::OnActionClicked);

    AddTextLine(AddPanel(FAnchors(0.0f, 0.0f, 0.0f, 0.0f), FMargin(40.0f, 40.0f, 400.0f, 88.0f), SS3DUI::Panel),
        FText::FromString(TEXT("UI / COMBAT SLICE  ·  DATA-DRIVEN")), 11, SS3DUI::Muted);
}

void USBattleHUD::Render(const FCombatSnapshot& Snapshot)
{
    CurrentSnapshot = Snapshot;
    if (PlayerHpText) PlayerHpText->SetText(FText::Format(NSLOCTEXT("HUD", "PlayerHp", "HP  {0} / {1}"), Snapshot.PlayerHp, Snapshot.PlayerMaxHp));
    if (PlayerBlockText) PlayerBlockText->SetText(FText::Format(NSLOCTEXT("HUD", "PlayerBlock", "SHIELD  {0}"), Snapshot.PlayerBlock));
    if (EnergyText) EnergyText->SetText(FText::Format(NSLOCTEXT("HUD", "Energy", "{0} / {1}"), Snapshot.Energy, Snapshot.MaxEnergy));
    if (EnemyHpText) EnemyHpText->SetText(FText::Format(NSLOCTEXT("HUD", "EnemyHp", "HP  {0} / {1}"), Snapshot.EnemyHp, Snapshot.EnemyMaxHp));
    if (EnemyIntentText) EnemyIntentText->SetText(Snapshot.Phase == ECombatPhase::Victory
        ? FText::FromString(TEXT("TARGET  DOWN"))
        : FText::Format(NSLOCTEXT("HUD", "Intent", "INTENT  {0}  ·  {1}"), Snapshot.EnemyIntentDamage, Snapshot.EnemyIntentLabel));
    if (PhaseText) PhaseText->SetText(PhaseLabel(Snapshot.Phase));
    if (TurnText) TurnText->SetText(FText::Format(NSLOCTEXT("HUD", "Turn", "TURN {0}"), FText::AsNumber(Snapshot.Turn)));
    if (DeckText) DeckText->SetText(FText::Format(NSLOCTEXT("HUD", "Deck", "DRAW  {0}\nDISCARD  {1}"), Snapshot.DrawPileCount, Snapshot.DiscardPileCount));
    RenderHand(Snapshot.Hand);
    RenderActionState(Snapshot.Phase);
}

void USBattleHUD::RenderHand(const TArray<FCardData>& Hand)
{
    if (!HandBox) return;
    HandBox->ClearChildren();
    UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
    UVerticalBoxSlot* RowSlot = HandBox->AddChildToVerticalBox(Row);
    RowSlot->SetHorizontalAlignment(HAlign_Center);

    for (int32 Index = 0; Index < Hand.Num(); ++Index)
    {
        const FCardData& Card = Hand[Index];
        USizeBox* CardSize = WidgetTree->ConstructWidget<USizeBox>();
        CardSize->SetWidthOverride(172.0f);
        CardSize->SetHeightOverride(184.0f);
        UHorizontalBoxSlot* CardSlot = Row->AddChildToHorizontalBox(CardSize);
        CardSlot->SetPadding(FMargin(5.0f, 0.0f));

        UButton* CardButton = WidgetTree->ConstructWidget<UButton>();
        const bool bCanPlay = CurrentSnapshot.Phase == ECombatPhase::PlayerTurn && Card.Cost <= CurrentSnapshot.Energy;
        CardButton->SetIsEnabled(bCanPlay);
        CardButton->SetStyle(MakeButtonStyle(Card.Type == ECardType::Attack
            ? FLinearColor(0.18f, 0.11f, 0.23f, 1.0f)
            : FLinearColor(0.08f, 0.20f, 0.27f, 1.0f)));
        CardSize->AddChild(CardButton);
        CardButton->OnClicked.AddLambda([this, Index]()
        {
            OnCardClicked(Index);
            return FReply::Handled();
        });

        UVerticalBox* CardContent = WidgetTree->ConstructWidget<UVerticalBox>();
        CardButton->AddChild(CardContent);
        UTextBlock* Cost = AddTextLine(CardContent, FText::AsNumber(Card.Cost), 21, SS3DUI::Gold);
        Cost->SetJustification(ETextJustify::Left);
        UTextBlock* Type = AddTextLine(CardContent, CardTypeLabel(Card.Type), 10, SS3DUI::Muted);
        Type->SetJustification(ETextJustify::Right);
        UTextBlock* Name = AddTextLine(CardContent, Card.Name, 18, FLinearColor::White);
        Name->SetJustification(ETextJustify::Center);
        UTextBlock* Description = AddTextLine(CardContent, Card.Description, 12, FLinearColor(0.78f, 0.84f, 0.90f, 1.0f));
        Description->SetAutoWrapText(true);
        Description->SetJustification(ETextJustify::Center);
    }
}

void USBattleHUD::RenderActionState(ECombatPhase Phase)
{
    if (!ActionButton) return;
    UTextBlock* Label = Cast<UTextBlock>(ActionButton->GetContent());
    if (!Label) return;
    if (Phase == ECombatPhase::Victory) Label->SetText(FText::FromString(TEXT("继续探索  ›")));
    else if (Phase == ECombatPhase::Defeat) Label->SetText(FText::FromString(TEXT("重新同步  ↻")));
    else Label->SetText(FText::FromString(TEXT("结束回合  ›")));
    ActionButton->SetIsEnabled(Phase == ECombatPhase::PlayerTurn || Phase == ECombatPhase::Victory || Phase == ECombatPhase::Defeat);
}

void USBattleHUD::AddCombatLog(const FText& Message, int32 Tone)
{
    CombatLog.Add(Message);
    while (CombatLog.Num() > 6) CombatLog.RemoveAt(0);
    TArray<FString> Lines;
    for (const FText& Entry : CombatLog) Lines.Add(Entry.ToString());
    if (LogText) LogText->SetText(FText::FromString(FString::Join(Lines, TEXT("\n"))));
}

void USBattleHUD::OnCardClicked(int32 HandIndex)
{
    if (CombatManager) CombatManager->PlayCard(HandIndex);
}

void USBattleHUD::OnActionClicked()
{
    if (!CombatManager) return;
    if (CurrentSnapshot.Phase == ECombatPhase::PlayerTurn) CombatManager->EndPlayerTurn();
    else if (CurrentSnapshot.Phase == ECombatPhase::Victory || CurrentSnapshot.Phase == ECombatPhase::Defeat) CombatManager->RestartCombat();
}

UTextBlock* USBattleHUD::AddTextLine(UVerticalBox* Parent, const FText& Text, int32 FontSize, const FLinearColor& Color)
{
    UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>();
    Label->SetText(Text);
    Label->SetColorAndOpacity(Color);
    Label->SetJustification(ETextJustify::Center);
    FSlateFontInfo Font = FCoreStyle::GetDefaultFont();
    Font.Size = FontSize;
    Label->SetFont(Font);
    UVerticalBoxSlot* Slot = Parent->AddChildToVerticalBox(Label);
    Slot->SetPadding(FMargin(14.0f, 4.0f));
    Slot->SetHorizontalAlignment(HAlign_Fill);
    return Label;
}

UVerticalBox* USBattleHUD::AddPanel(const FAnchors& Anchors, const FMargin& Offsets, const FLinearColor& Color)
{
    UBorder* Border = WidgetTree->ConstructWidget<UBorder>();
    Border->SetBrushColor(Color);
    UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>();
    Border->SetContent(Content);
    AddToCanvas(Border, Anchors, Offsets);
    return Content;
}

UWidget* USBattleHUD::AddToCanvas(UWidget* Widget, const FAnchors& Anchors, const FMargin& Offsets)
{
    UCanvasPanelSlot* Slot = RootCanvas->AddChildToCanvas(Widget);
    Slot->SetAnchors(Anchors);
    Slot->SetOffsets(Offsets);
    return Widget;
}

FButtonStyle USBattleHUD::MakeButtonStyle(const FLinearColor& Color)
{
    FButtonStyle Style;
    FSlateBrush Normal;
    Normal.DrawAs = ESlateBrushDrawType::RoundedBox;
    Normal.Tint = Color;
    Normal.OutlineSettings.CornerRadii = FVector4(4.0f, 4.0f, 4.0f, 4.0f);
    Normal.OutlineSettings.OutlineSize = 1.0f;
    Normal.OutlineSettings.Color = FLinearColor(0.25f, 0.65f, 0.82f, 0.45f);
    Style.SetNormal(Normal);
    Style.SetHovered(Normal);
    Style.SetPressed(Normal);
    return Style;
}

FText USBattleHUD::PhaseLabel(ECombatPhase Phase)
{
    switch (Phase)
    {
    case ECombatPhase::EnemyTurn: return FText::FromString(TEXT("ENEMY TURN"));
    case ECombatPhase::Victory: return FText::FromString(TEXT("SIGNAL CLEARED"));
    case ECombatPhase::Defeat: return FText::FromString(TEXT("SYSTEM FAILURE"));
    default: return FText::FromString(TEXT("YOUR TURN"));
    }
}

FText USBattleHUD::CardTypeLabel(ECardType Type)
{
    switch (Type)
    {
    case ECardType::Skill: return FText::FromString(TEXT("SKILL"));
    case ECardType::Power: return FText::FromString(TEXT("POWER"));
    default: return FText::FromString(TEXT("ATTACK"));
    }
}
