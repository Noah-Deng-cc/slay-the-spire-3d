#include "BattleHUD.h"

#include "SS3DGameMode.h"
#include "MapManager.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"

namespace
{
    FSlateFontInfo FontAtSize(int32 Size)
    {
        FSlateFontInfo Font = FCoreStyle::Get().GetFontStyle(TEXT("NormalFont"));
        Font.Size = Size;
        return Font;
    }
}

class SMemoryMapCanvas : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SMemoryMapCanvas) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        SetCanTick(false);
    }

    void SetMapState(const FMapRunState& InState)
    {
        MapState = InState;
        Invalidate(EInvalidateWidgetReason::Paint);
    }

    void SetOnNodeClicked(TFunction<void(int32)> InCallback)
    {
        OnNodeClicked = MoveTemp(InCallback);
    }

protected:
    virtual FVector2D ComputeDesiredSize(float) const override
    {
        return FVector2D(1040.0f, 760.0f);
    }

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
        const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
        int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
    {
        const FSlateBrush* Brush = FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
        const FLinearColor Background(0.035f, 0.055f, 0.09f, 1.0f);
        FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Brush,
            ESlateDrawEffect::None, Background);

        if (MapState.Nodes.Num() == 0)
        {
            FSlateDrawElement::MakeText(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(),
                FText::FromString(TEXT("记忆尖塔")), FontAtSize(22), ESlateDrawEffect::None,
                FLinearColor(0.55f, 0.85f, 0.95f, 1.0f));
            return LayerId + 1;
        }

        for (int32 ActIndex = 0; ActIndex < 3; ++ActIndex)
        {
            FSlateDrawElement::MakeText(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(
                MapPosition(ActIndex, 7, 0) + FVector2D(-30.0f, -32.0f), FVector2D(120.0f, 24.0f)),
                FText::FromString(FString::Printf(TEXT("第 %d 层"), ActIndex + 1)), FontAtSize(16),
                ESlateDrawEffect::None, FLinearColor(0.55f, 0.78f, 0.90f, 1.0f));
        }

        for (const FMapNodeData& Node : MapState.Nodes)
        {
            const FVector2D Start = MapPosition(Node) + PanOffset + FVector2D(42.0f, 20.0f);
            for (const int32 NextId : Node.NextNodeIds)
            {
                const FMapNodeData* Next = FindNode(NextId);
                if (!Next) continue;
                const FVector2D End = MapPosition(*Next) + PanOffset + FVector2D(42.0f, 20.0f);
                TArray<FVector2D> Points;
                Points.Add(Start);
                Points.Add(End);
                const bool bActive = Node.NodeId == MapState.CurrentNodeId || Node.bCompleted;
                FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(),
                    Points, ESlateDrawEffect::None,
                    bActive ? FLinearColor(0.28f, 0.65f, 0.78f, 0.85f) : FLinearColor(0.20f, 0.28f, 0.38f, 0.9f),
                    true, 2.0f);
            }
        }

        for (const FMapNodeData& Node : MapState.Nodes)
        {
            const FVector2D Position = MapPosition(Node) + PanOffset;
            const bool bCurrent = Node.NodeId == MapState.CurrentNodeId;
            const bool bSelectable = IsSelectable(Node);
            const FLinearColor Color = bCurrent
                ? FLinearColor(0.12f, 0.58f, 0.70f, 1.0f)
                : (Node.bCompleted ? FLinearColor(0.18f, 0.42f, 0.30f, 1.0f)
                    : (bSelectable ? FLinearColor(0.20f, 0.34f, 0.48f, 1.0f) : FLinearColor(0.10f, 0.15f, 0.22f, 1.0f)));
            FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 2,
                AllottedGeometry.ToPaintGeometry(Position, FVector2D(84.0f, 40.0f)), Brush,
                ESlateDrawEffect::None, Color);

            const FString Label = FString::Printf(TEXT("%s%s"), *NodeTypeLabel(Node.NodeType).ToString(), bSelectable ? TEXT("  ·") : TEXT(""));
            FSlateDrawElement::MakeText(OutDrawElements, LayerId + 3,
                AllottedGeometry.ToPaintGeometry(Position + FVector2D(5.0f, 9.0f), FVector2D(74.0f, 22.0f)),
                FText::FromString(Label), FontAtSize(13), ESlateDrawEffect::None,
                bCurrent || bSelectable ? FLinearColor::White : FLinearColor(0.68f, 0.74f, 0.82f, 1.0f));
        }

        return LayerId + 3;
    }

    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
    {
        if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton) return FReply::Unhandled();
        PressedNodeId = HitTest(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
        LastCursor = MouseEvent.GetScreenSpacePosition();
        bDragging = true;
        bMoved = false;
        return FReply::Handled().CaptureMouse(SharedThis(this));
    }

    virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
    {
        if (!bDragging || !HasMouseCapture()) return FReply::Unhandled();
        const FVector2D Delta = MouseEvent.GetScreenSpacePosition() - LastCursor;
        LastCursor = MouseEvent.GetScreenSpacePosition();
        if (Delta.SizeSquared() > 0.0f)
        {
            bMoved = true;
            PanOffset += Delta;
            PanOffset.X = FMath::Clamp(PanOffset.X, FMath::Min(18.0f, MyGeometry.Size.X - 1040.0f - 18.0f), 18.0f);
            PanOffset.Y = FMath::Clamp(PanOffset.Y, FMath::Min(46.0f, MyGeometry.Size.Y - 760.0f - 18.0f), 46.0f);
            Invalidate(EInvalidateWidgetReason::Paint);
        }
        return FReply::Handled();
    }

    virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
    {
        if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton || !bDragging) return FReply::Unhandled();
        const int32 ClickedNodeId = PressedNodeId;
        const bool bWasClick = !bMoved;
        bDragging = false;
        PressedNodeId = INDEX_NONE;
        FReply Reply = FReply::Handled().ReleaseMouseCapture();
        if (bWasClick && ClickedNodeId != INDEX_NONE && OnNodeClicked)
        {
            OnNodeClicked(ClickedNodeId);
        }
        return Reply;
    }

private:
    FMapRunState MapState;
    TFunction<void(int32)> OnNodeClicked;
    FVector2D PanOffset = FVector2D(18.0f, 46.0f);
    FVector2D LastCursor = FVector2D::ZeroVector;
    int32 PressedNodeId = INDEX_NONE;
    bool bDragging = false;
    bool bMoved = false;

    static FVector2D MapPosition(const FMapNodeData& Node)
    {
        return MapPosition(Node.ActIndex, Node.RowIndex, Node.ColumnIndex);
    }

    static FVector2D MapPosition(int32 ActIndex, int32 RowIndex, int32 ColumnIndex)
    {
        const float ColumnOffset = ColumnIndex == 0 ? 0.0f : (ColumnIndex == 1 ? 48.0f : 96.0f);
        return FVector2D(70.0f + ActIndex * 330.0f + ColumnOffset, 650.0f - RowIndex * 82.0f);
    }

    const FMapNodeData* FindNode(int32 NodeId) const
    {
        for (const FMapNodeData& Node : MapState.Nodes)
        {
            if (Node.NodeId == NodeId) return &Node;
        }
        return nullptr;
    }

    bool IsSelectable(const FMapNodeData& Node) const
    {
        const FMapNodeData* Current = FindNode(MapState.CurrentNodeId);
        return Current && !Node.bVisited && Current->NextNodeIds.Contains(Node.NodeId)
            && (Current->bCompleted || Current->NodeType == EMapNodeType::Start);
    }

    int32 HitTest(const FVector2D& LocalPosition) const
    {
        for (const FMapNodeData& Node : MapState.Nodes)
        {
            const FVector2D Position = MapPosition(Node) + PanOffset;
            if (FBox2D(Position, Position + FVector2D(84.0f, 40.0f)).IsInside(LocalPosition)) return Node.NodeId;
        }
        return INDEX_NONE;
    }

    static FText NodeTypeLabel(EMapNodeType Type)
    {
        switch (Type)
        {
        case EMapNodeType::Start: return FText::FromString(TEXT("起点"));
        case EMapNodeType::Combat: return FText::FromString(TEXT("战斗"));
        case EMapNodeType::Reward: return FText::FromString(TEXT("奖励"));
        case EMapNodeType::Shop: return FText::FromString(TEXT("商店"));
        case EMapNodeType::Event: return FText::FromString(TEXT("事件"));
        case EMapNodeType::Elite: return FText::FromString(TEXT("精英"));
        case EMapNodeType::Rest: return FText::FromString(TEXT("休息"));
        case EMapNodeType::Boss: return FText::FromString(TEXT("Boss"));
        default: return FText::FromString(TEXT("未知"));
        }
    }
};

void USBattleHUD::NativeConstruct()
{
    Super::NativeConstruct();
    Refresh();
}

void USBattleHUD::NativeDestruct()
{
    if (GameMode)
    {
        GameMode->OnStateChanged.RemoveAll(this);
    }
    Super::NativeDestruct();
}

TSharedRef<SWidget> USBattleHUD::RebuildWidget()
{
    BuildLayout();
    return RootOverlay.ToSharedRef();
}

void USBattleHUD::ReleaseSlateResources(bool bReleaseChildren)
{
    RootOverlay.Reset();
    MainContent.Reset();
    SideContent.Reset();
    HandContent.Reset();
    MapCanvas.Reset();
    HandFrame.Reset();
    HeaderStatus.Reset();
    SeedInput.Reset();
    Super::ReleaseSlateResources(bReleaseChildren);
}

void USBattleHUD::InitializeWithGameMode(ASS3DGameMode* InGameMode)
{
    if (GameMode)
    {
        GameMode->OnStateChanged.RemoveAll(this);
    }

    GameMode = InGameMode;
    if (GameMode)
    {
        GameMode->OnStateChanged.AddUObject(this, &USBattleHUD::HandleGameStateChanged);
    }
    if (MapCanvas.IsValid())
    {
        MapCanvas->SetOnNodeClicked([this](int32 NodeId)
        {
            ExecuteCommand(FString::Printf(TEXT("select %d"), NodeId));
        });
    }
    Refresh();
}

void USBattleHUD::BuildLayout()
{
    SAssignNew(RootOverlay, SOverlay);

    RootOverlay->AddSlot()
    [
        SNew(SBorder)
        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(FLinearColor(0.025f, 0.045f, 0.075f, 1.0f))
    ];

    TSharedPtr<SVerticalBox> Frame;
    SAssignNew(Frame, SVerticalBox);

    TSharedPtr<SHorizontalBox> Header;
    SAssignNew(Header, SHorizontalBox)
    + SHorizontalBox::Slot()
    .FillWidth(1.0f)
    .VAlign(VAlign_Center)
    [
        SNew(STextBlock)
        .Text(FText::FromString(TEXT("记忆尖塔")))
        .Font(FontAtSize(28))
        .ColorAndOpacity(FLinearColor(0.35f, 0.90f, 1.0f, 1.0f))
    ]
    + SHorizontalBox::Slot()
    .AutoWidth()
    .VAlign(VAlign_Center)
    [
        SAssignNew(HeaderStatus, STextBlock)
        .Font(FontAtSize(16))
        .ColorAndOpacity(FLinearColor(0.75f, 0.82f, 0.90f, 1.0f))
    ];

    TSharedPtr<SHorizontalBox> Body;
    SAssignNew(Body, SHorizontalBox)
    + SHorizontalBox::Slot()
    .AutoWidth()
    .Padding(0.0f, 12.0f, 12.0f, 0.0f)
    [
        SNew(SBox)
        .WidthOverride(470.0f)
        [
            SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(0.035f, 0.055f, 0.09f, 0.98f))
            .Padding(6.0f)
            [
                SAssignNew(MapCanvas, SMemoryMapCanvas)
            ]
        ]
    ]
    + SHorizontalBox::Slot()
    .FillWidth(1.0f)
        .Padding(0.0f, 12.0f, 12.0f, 0.0f)
    [
        SNew(SBorder)
        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(FLinearColor(0.045f, 0.065f, 0.10f, 0.98f))
        .Padding(18.0f)
        [
            SNew(SScrollBox)
            + SScrollBox::Slot()
            [
                SAssignNew(MainContent, SVerticalBox)
            ]
        ]
    ]
    + SHorizontalBox::Slot()
    .AutoWidth()
    .Padding(0.0f, 12.0f, 0.0f, 0.0f)
    [
        SNew(SBox)
        .WidthOverride(230.0f)
        [
            SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(0.055f, 0.085f, 0.13f, 0.96f))
            .Padding(14.0f)
            [
                SNew(SScrollBox)
                + SScrollBox::Slot()
                [
                    SAssignNew(SideContent, SVerticalBox)
                ]
            ]
        ]
    ];

    Frame->AddSlot()
    .AutoHeight()
    .Padding(18.0f, 14.0f, 18.0f, 0.0f)
    [
        Header.ToSharedRef()
    ];
    Frame->AddSlot()
    .FillHeight(1.0f)
    .Padding(18.0f, 0.0f, 18.0f, 18.0f)
    [
        Body.ToSharedRef()
    ];

    Frame->AddSlot()
    .AutoHeight()
    .Padding(18.0f, 0.0f, 18.0f, 14.0f)
    [
        SAssignNew(HandFrame, SBorder)
        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(FLinearColor(0.055f, 0.075f, 0.11f, 0.98f))
        .Padding(12.0f, 8.0f)
        .Visibility_Lambda([this]()
        {
            return GameMode && GameMode->IsCombatActive() ? EVisibility::Visible : EVisibility::Collapsed;
        })
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 0.0f, 0.0f, 6.0f)
            [
                SNew(STextBlock)
                .Text_Lambda([this]()
                {
                    if (!GameMode || !GameMode->IsCombatActive()) return FText::GetEmpty();
                    return FText::FromString(FString::Printf(TEXT("手牌  %d"), GameMode->GetCombatSnapshot().Hand.Num()));
                })
                .Font(FontAtSize(15))
                .ColorAndOpacity(FLinearColor(0.72f, 0.84f, 0.92f, 1.0f))
            ]
            + SVerticalBox::Slot()
            .FillHeight(1.0f)
            [
                SNew(SScrollBox)
                .Orientation(Orient_Horizontal)
                + SScrollBox::Slot()
                [
                    SAssignNew(HandContent, SHorizontalBox)
                ]
            ]
        ]
    ];

    RootOverlay->AddSlot()
    .Padding(0.0f)
    [
        Frame.ToSharedRef()
    ];
}

void USBattleHUD::Refresh()
{
    if (!RootOverlay.IsValid() || !MapCanvas.IsValid() || !MainContent.IsValid() || !SideContent.IsValid() || !GameMode)
    {
        return;
    }

    if (!GameMode->IsRunStarted())
    {
        HeaderStatus->SetText(FText::FromString(TEXT("待机")));
    }
    else if (GameMode->IsCombatActive())
    {
        HeaderStatus->SetText(PhaseLabel(GameMode->GetCombatSnapshot().Phase));
    }
    else if (GameMode->GetMapState().bRunComplete)
    {
        HeaderStatus->SetText(FText::FromString(TEXT("已通关")));
    }
    else
    {
        HeaderStatus->SetText(FText::FromString(TEXT("地图")));
    }

    RefreshMapPanel();
    RefreshMainPanel();
    RefreshSidePanel();
    RefreshHandPanel();
}

void USBattleHUD::HandleGameStateChanged()
{
    Refresh();
}

void USBattleHUD::ExecuteCommand(const FString& Command)
{
    if (GameMode)
    {
        GameMode->ExecuteCommand(Command);
    }
}

void USBattleHUD::AddText(const TSharedPtr<SVerticalBox>& Parent, const FText& Text, const FLinearColor& Color, int32 FontSize, float BottomPadding)
{
    if (!Parent.IsValid()) return;

    Parent->AddSlot()
    .AutoHeight()
    .Padding(0.0f, 0.0f, 0.0f, BottomPadding)
    [
        SNew(STextBlock)
        .Text(Text)
        .AutoWrapText(true)
        .Font(FontAtSize(FontSize))
        .ColorAndOpacity(Color)
    ];
}

void USBattleHUD::AddButton(const TSharedPtr<SVerticalBox>& Parent, const FText& Label, TFunction<void()> Action, const FLinearColor& Color)
{
    if (!Parent.IsValid()) return;

    Parent->AddSlot()
    .AutoHeight()
    .Padding(0.0f, 0.0f, 0.0f, 8.0f)
    [
        SNew(SButton)
        .ContentPadding(FMargin(12.0f, 9.0f))
        .ButtonColorAndOpacity(Color)
        .OnClicked_Lambda([Action = MoveTemp(Action)]() mutable
        {
            Action();
            return FReply::Handled();
        })
        [
            SNew(STextBlock)
            .Text(Label)
            .AutoWrapText(true)
            .Justification(ETextJustify::Center)
            .Font(FontAtSize(15))
            .ColorAndOpacity(FLinearColor::White)
        ]
    ];
}

void USBattleHUD::RefreshMapPanel()
{
    if (!MapCanvas.IsValid() || !GameMode) return;
    MapCanvas->SetMapState(GameMode->IsRunStarted() ? GameMode->GetMapState() : FMapRunState());
}

void USBattleHUD::RefreshMainPanel()
{
    MainContent->ClearChildren();

    if (!GameMode->IsRunStarted())
    {
        ShowHome();
        return;
    }
    if (GameMode->IsCombatActive())
    {
        ShowCombat();
        return;
    }
    if (GameMode->GetPendingRewards().Num() > 0)
    {
        ShowRewards();
        return;
    }

    const FMapNodeData CurrentNode = GameMode->GetMapManager()->GetCurrentNode();
    if (CurrentNode.NodeType == EMapNodeType::Rest && !CurrentNode.bCompleted)
    {
        ShowRest();
    }
    else if (CurrentNode.NodeType == EMapNodeType::Event && !CurrentNode.bCompleted)
    {
        ShowEvent();
    }
    else if (CurrentNode.NodeType == EMapNodeType::Shop && !CurrentNode.bCompleted)
    {
        ShowShop();
    }
    else
    {
        ShowMap();
    }
}

void USBattleHUD::RefreshSidePanel()
{
    SideContent->ClearChildren();

    if (!GameMode->IsRunStarted())
    {
        return;
    }

    const int32 CurrentHp = GameMode->IsCombatActive() ? GameMode->GetCombatSnapshot().PlayerHp : GameMode->GetPlayerHp();
    const int32 CurrentMaxHp = GameMode->IsCombatActive() ? GameMode->GetCombatSnapshot().PlayerMaxHp : GameMode->GetPlayerMaxHp();
    const int32 CurrentPotionCount = GameMode->IsCombatActive() ? GameMode->GetCombatSnapshot().Potions.Num() : GameMode->GetPotions().Num();
    AddText(SideContent, FText::FromString(FString::Printf(TEXT("奥黛塔\n%d / %d HP\n%d 金币\n%d 卡牌\n%d 藏品\n%d 药水"),
        CurrentHp, CurrentMaxHp, GameMode->GetGold(), GameMode->GetDeck().Num(), GameMode->GetRelics().Num(), CurrentPotionCount)),
        FLinearColor(0.85f, 0.88f, 0.94f), 16, 16.0f);

    if (GameMode->IsCombatActive())
    {
        const FCombatSnapshot& Combat = GameMode->GetCombatSnapshot();
        AddText(SideContent, FText::FromString(FString::Printf(TEXT("回合 %d\n%d / %d 能量\n力量 %d\n敌方护盾 %d"),
            Combat.Turn, Combat.Energy, Combat.MaxEnergy, Combat.PlayerStrength, Combat.EnemyBlock)), FLinearColor(0.95f, 0.78f, 0.45f), 15, 14.0f);
    }
}

void USBattleHUD::RefreshHandPanel()
{
    if (!HandContent.IsValid()) return;
    HandContent->ClearChildren();
    if (!GameMode || !GameMode->IsCombatActive()) return;

    const TArray<FCardData>& Hand = GameMode->GetCombatSnapshot().Hand;
    for (int32 Index = 0; Index < Hand.Num(); ++Index)
    {
        AddCardButton(Hand[Index], Index);
    }
}

void USBattleHUD::AddCardButton(const FCardData& Card, int32 CardIndex)
{
    if (!HandContent.IsValid()) return;
    const FLinearColor Color = Card.Type == ECardType::Attack
        ? FLinearColor(0.38f, 0.16f, 0.16f, 1.0f)
        : (Card.Type == ECardType::Power ? FLinearColor(0.30f, 0.18f, 0.36f, 1.0f) : FLinearColor(0.12f, 0.25f, 0.34f, 1.0f));
    HandContent->AddSlot()
        .AutoWidth()
        .Padding(0.0f, 0.0f, 8.0f, 0.0f)
        [
            SNew(SBox)
            .WidthOverride(178.0f)
            .HeightOverride(116.0f)
            [
                SNew(SButton)
                .ContentPadding(FMargin(10.0f, 8.0f))
                .ButtonColorAndOpacity(Color)
                .OnClicked_Lambda([this, CardIndex]()
                {
                    ExecuteCommand(FString::Printf(TEXT("play %d"), CardIndex));
                    return FReply::Handled();
                })
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(FString::Printf(TEXT("%s    %d\n%s"), *Card.Name.ToString(), Card.Cost, *Card.Description.ToString())))
                    .AutoWrapText(true)
                    .Justification(ETextJustify::Center)
                    .Font(FontAtSize(14))
                    .ColorAndOpacity(FLinearColor::White)
                ]
            ]
        ];
}

void USBattleHUD::ShowHome()
{
    AddText(MainContent, FText::FromString(TEXT("奥黛塔")), FLinearColor(0.35f, 0.90f, 1.0f), 30, 16.0f);
    SAssignNew(SeedInput, SEditableTextBox)
        .Text(FText::FromString(TEXT("1337")))
        .HintText(FText::FromString(TEXT("Seed")));
    MainContent->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 14.0f)[SeedInput.ToSharedRef()];
    AddButton(MainContent, FText::FromString(TEXT("开始")), [this]() { StartOdette(); }, FLinearColor(0.08f, 0.42f, 0.48f, 1.0f));
}

void USBattleHUD::ShowMap()
{
    const FMapRunState& State = GameMode->GetMapState();
    AddText(MainContent, FText::FromString(FString::Printf(TEXT("第 %d 层    节点 %d"), State.CurrentAct + 1, State.CurrentNodeId)), FLinearColor(0.35f, 0.90f, 1.0f), 24, 14.0f);
    const TArray<FMapNodeData> Available = GameMode->GetMapManager()->GetAvailableNextNodes();
    for (const FMapNodeData& Node : Available)
    {
        const int32 NodeId = Node.NodeId;
        AddButton(MainContent, FText::FromString(FString::Printf(TEXT("%s  %d"), *NodeTypeLabel(Node.NodeType).ToString(), NodeId)), [this, NodeId]() { ExecuteCommand(FString::Printf(TEXT("select %d"), NodeId)); });
    }
}

void USBattleHUD::ShowCombat()
{
    const FCombatSnapshot& Combat = GameMode->GetCombatSnapshot();
    AddText(MainContent, FText::FromString(FString::Printf(TEXT("战斗  |  %s"), *Combat.EnemyId)), FLinearColor(0.95f, 0.55f, 0.45f), 26, 6.0f);
    AddText(MainContent, FText::FromString(FString::Printf(TEXT("敌人生命 %d / %d    敌方护盾 %d"), Combat.EnemyHp, Combat.EnemyMaxHp, Combat.EnemyBlock)), FLinearColor(0.90f, 0.82f, 0.78f), 17, 4.0f);
    AddText(MainContent, FText::FromString(FString::Printf(TEXT("敌方状态：易伤 %d    虚弱 %d    中毒 %d    力量 %d"), Combat.EnemyVulnerable, Combat.EnemyWeak, Combat.EnemyPoison, Combat.EnemyStrength)), FLinearColor(0.95f, 0.65f, 0.50f), 14, 8.0f);
    AddText(MainContent, FText::FromString(FString::Printf(TEXT("敌方意图：%s (%d)"), *Combat.EnemyIntentLabel.ToString(), Combat.EnemyIntentDamage)), FLinearColor(0.98f, 0.50f, 0.45f), 16, 14.0f);
    AddText(MainContent, FText::FromString(FString::Printf(TEXT("你的生命 %d / %d    护盾 %d    能量 %d / %d"), Combat.PlayerHp, Combat.PlayerMaxHp, Combat.PlayerBlock, Combat.Energy, Combat.MaxEnergy)), FLinearColor(0.65f, 0.90f, 0.72f), 16, 12.0f);

    AddButton(MainContent, FText::FromString(TEXT("结束回合")), [this]() { ExecuteCommand(TEXT("end")); }, FLinearColor(0.35f, 0.22f, 0.14f, 1.0f));

    if (Combat.Potions.Num() > 0)
    {
        AddText(MainContent, FText::FromString(TEXT("药水")), FLinearColor(0.95f, 0.72f, 0.35f), 18, 6.0f);
        for (int32 Index = 0; Index < Combat.Potions.Num(); ++Index)
        {
            const int32 PotionIndex = Index;
            AddButton(MainContent, FText::FromString(FString::Printf(TEXT("使用 %s"), *Combat.Potions[Index].Name.ToString())), [this, PotionIndex]()
            {
                ExecuteCommand(FString::Printf(TEXT("potion %d"), PotionIndex));
            }, FLinearColor(0.30f, 0.25f, 0.12f, 1.0f));
        }
    }
}

void USBattleHUD::ShowRewards()
{
    AddText(MainContent, FText::FromString(TEXT("奖励")), FLinearColor(0.95f, 0.78f, 0.35f), 28, 12.0f);
    const TArray<FCardData>& Rewards = GameMode->GetPendingRewards();
    for (int32 Index = 0; Index < Rewards.Num(); ++Index)
    {
        const int32 RewardIndex = Index;
        AddButton(MainContent, FText::FromString(FString::Printf(TEXT("%s\n%s"), *Rewards[Index].Name.ToString(), *Rewards[Index].Description.ToString())), [this, RewardIndex]()
        {
            ExecuteCommand(FString::Printf(TEXT("reward %d"), RewardIndex));
        }, FLinearColor(0.28f, 0.22f, 0.10f, 1.0f));
    }
    AddButton(MainContent, FText::FromString(TEXT("跳过")), [this]() { ExecuteCommand(TEXT("reward -1")); }, FLinearColor(0.18f, 0.20f, 0.25f, 1.0f));
}

void USBattleHUD::ShowRest()
{
    AddText(MainContent, FText::FromString(TEXT("休息")), FLinearColor(0.45f, 0.88f, 0.60f), 28, 12.0f);
    AddButton(MainContent, FText::FromString(TEXT("恢复")), [this]() { ExecuteCommand(TEXT("rest heal")); }, FLinearColor(0.15f, 0.38f, 0.24f, 1.0f));
    AddButton(MainContent, FText::FromString(TEXT("升级")), [this]() { ExecuteCommand(TEXT("rest upgrade")); }, FLinearColor(0.24f, 0.30f, 0.18f, 1.0f));
}

void USBattleHUD::ShowEvent()
{
    AddText(MainContent, FText::FromString(TEXT("事件")), FLinearColor(0.70f, 0.55f, 0.95f), 28, 12.0f);
    AddButton(MainContent, FText::FromString(TEXT("恢复 12 HP")), [this]() { ExecuteCommand(TEXT("event 0")); }, FLinearColor(0.28f, 0.20f, 0.40f, 1.0f));
    AddButton(MainContent, FText::FromString(TEXT("获得 40 金币")), [this]() { ExecuteCommand(TEXT("event 1")); }, FLinearColor(0.32f, 0.24f, 0.12f, 1.0f));
}

void USBattleHUD::ShowShop()
{
    AddText(MainContent, FText::FromString(TEXT("商店")), FLinearColor(0.95f, 0.72f, 0.35f), 28, 8.0f);
    AddText(MainContent, FText::FromString(FString::Printf(TEXT("当前金币：%d"), GameMode->GetGold())), FLinearColor(0.95f, 0.84f, 0.48f), 16, 12.0f);

    AddText(MainContent, FText::FromString(TEXT("卡牌 / 50 金币")), FLinearColor(0.35f, 0.90f, 1.0f), 18, 5.0f);
    for (int32 Index = 0; Index < GameMode->GetShopCards().Num(); ++Index)
    {
        const int32 ItemIndex = Index;
        AddButton(MainContent, GameMode->GetShopCards()[Index].Name, [this, ItemIndex]() { ExecuteCommand(FString::Printf(TEXT("buy card %d"), ItemIndex)); });
    }

    AddText(MainContent, FText::FromString(TEXT("藏品 / 100 金币")), FLinearColor(0.70f, 0.55f, 0.95f), 18, 5.0f);
    for (int32 Index = 0; Index < GameMode->GetShopRelics().Num(); ++Index)
    {
        const int32 ItemIndex = Index;
        AddButton(MainContent, GameMode->GetShopRelics()[Index].Name, [this, ItemIndex]() { ExecuteCommand(FString::Printf(TEXT("buy relic %d"), ItemIndex)); }, FLinearColor(0.25f, 0.18f, 0.36f, 1.0f));
    }

    AddText(MainContent, FText::FromString(TEXT("药水 / 30 金币")), FLinearColor(0.95f, 0.72f, 0.35f), 18, 5.0f);
    for (int32 Index = 0; Index < GameMode->GetShopPotions().Num(); ++Index)
    {
        const int32 ItemIndex = Index;
        AddButton(MainContent, GameMode->GetShopPotions()[Index].Name, [this, ItemIndex]() { ExecuteCommand(FString::Printf(TEXT("buy potion %d"), ItemIndex)); }, FLinearColor(0.32f, 0.25f, 0.12f, 1.0f));
    }

    AddButton(MainContent, FText::FromString(TEXT("离开商店")), [this]() { ExecuteCommand(TEXT("buy done")); }, FLinearColor(0.18f, 0.20f, 0.25f, 1.0f));
}

void USBattleHUD::StartOdette()
{
    const FString SeedText = SeedInput.IsValid() ? SeedInput->GetText().ToString() : TEXT("1337");
    const int32 Seed = FMath::Max(1, FCString::Atoi(*SeedText));
    ExecuteCommand(TEXT("character odette"));
    ExecuteCommand(FString::Printf(TEXT("new %d"), Seed));
}

FText USBattleHUD::NodeTypeLabel(EMapNodeType Type)
{
    switch (Type)
    {
    case EMapNodeType::Start: return FText::FromString(TEXT("起点"));
    case EMapNodeType::Combat: return FText::FromString(TEXT("战斗"));
    case EMapNodeType::Reward: return FText::FromString(TEXT("奖励"));
    case EMapNodeType::Shop: return FText::FromString(TEXT("商店"));
    case EMapNodeType::Event: return FText::FromString(TEXT("事件"));
    case EMapNodeType::Elite: return FText::FromString(TEXT("精英"));
    case EMapNodeType::Rest: return FText::FromString(TEXT("休息"));
    case EMapNodeType::Boss: return FText::FromString(TEXT("Boss"));
    default: return FText::FromString(TEXT("未知"));
    }
}

FText USBattleHUD::PhaseLabel(ECombatPhase Phase)
{
    switch (Phase)
    {
    case ECombatPhase::PlayerTurn: return FText::FromString(TEXT("玩家回合"));
    case ECombatPhase::EnemyTurn: return FText::FromString(TEXT("敌人回合"));
    case ECombatPhase::Victory: return FText::FromString(TEXT("胜利"));
    case ECombatPhase::Defeat: return FText::FromString(TEXT("失败"));
    default: return FText::FromString(TEXT("未知"));
    }
}
