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
#include "Widgets/Text/STextBlock.h"
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
    MapContent.Reset();
    MainContent.Reset();
    SideContent.Reset();
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
        .WidthOverride(300.0f)
        [
            SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(0.055f, 0.085f, 0.13f, 0.96f))
            .Padding(14.0f)
            [
                SNew(SScrollBox)
                + SScrollBox::Slot()
                [
                    SAssignNew(MapContent, SVerticalBox)
                ]
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
        .WidthOverride(280.0f)
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

    RootOverlay->AddSlot()
    .Padding(0.0f)
    [
        Frame.ToSharedRef()
    ];
}

void USBattleHUD::Refresh()
{
    if (!RootOverlay.IsValid() || !MapContent.IsValid() || !MainContent.IsValid() || !SideContent.IsValid() || !GameMode)
    {
        return;
    }

    if (!GameMode->IsRunStarted())
    {
        HeaderStatus->SetText(FText::FromString(TEXT("准备开始")));
    }
    else if (GameMode->IsCombatActive())
    {
        HeaderStatus->SetText(FText::Format(FText::FromString(TEXT("战斗中 · {0}")), PhaseLabel(GameMode->GetCombatSnapshot().Phase)));
    }
    else if (GameMode->GetMapState().bRunComplete)
    {
        HeaderStatus->SetText(FText::FromString(TEXT("已通关")));
    }
    else
    {
        HeaderStatus->SetText(FText::FromString(TEXT("规划路线")));
    }

    RefreshMapPanel();
    RefreshMainPanel();
    RefreshSidePanel();
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
    MapContent->ClearChildren();

    if (!GameMode->IsRunStarted())
    {
        AddText(MapContent, FText::FromString(TEXT("记忆尖塔")), FLinearColor(0.35f, 0.90f, 1.0f, 1.0f), 22, 10.0f);
        AddText(MapContent, FText::FromString(TEXT("进入神秘空间，规划路线并登上塔顶。")), FLinearColor(0.75f, 0.82f, 0.90f), 15);
        return;
    }

    const FMapRunState& State = GameMode->GetMapState();
    AddText(MapContent, FText::FromString(FString::Printf(TEXT("尖塔地图  |  第 %d 层"), State.CurrentAct + 1)), FLinearColor(0.35f, 0.90f, 1.0f, 1.0f), 20, 10.0f);
    AddText(MapContent, FText::FromString(FString::Printf(TEXT("Seed %d  |  当前节点 %d"), State.Seed, State.CurrentNodeId)), FLinearColor(0.65f, 0.72f, 0.82f), 13, 12.0f);

    for (const FMapNodeData& Node : State.Nodes)
    {
        if (Node.ActIndex != State.CurrentAct && Node.NodeType != EMapNodeType::Boss) continue;

        FString Marker = Node.NodeId == State.CurrentNodeId ? TEXT("> ") : (Node.bCompleted ? TEXT("✓ ") : TEXT("  "));
        const FLinearColor Color = Node.NodeId == State.CurrentNodeId
            ? FLinearColor(0.35f, 0.90f, 1.0f)
            : (Node.bCompleted ? FLinearColor(0.45f, 0.75f, 0.55f) : FLinearColor(0.72f, 0.76f, 0.84f));
        AddText(MapContent, FText::FromString(FString::Printf(TEXT("%s[%d] Row%d  %s"), *Marker, Node.NodeId, Node.RowIndex, *NodeTypeLabel(Node.NodeType).ToString())), Color, 14, 3.0f);
    }
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
        AddText(SideContent, FText::FromString(TEXT("状态")), FLinearColor(0.35f, 0.90f, 1.0f), 20, 10.0f);
        AddText(SideContent, FText::FromString(TEXT("等待开始一局游戏。")), FLinearColor(0.72f, 0.76f, 0.84f), 15);
        return;
    }

    AddText(SideContent, FText::FromString(TEXT("本局状态")), FLinearColor(0.35f, 0.90f, 1.0f), 20, 10.0f);
    const int32 CurrentHp = GameMode->IsCombatActive() ? GameMode->GetCombatSnapshot().PlayerHp : GameMode->GetPlayerHp();
    const int32 CurrentMaxHp = GameMode->IsCombatActive() ? GameMode->GetCombatSnapshot().PlayerMaxHp : GameMode->GetPlayerMaxHp();
    const int32 CurrentPotionCount = GameMode->IsCombatActive() ? GameMode->GetCombatSnapshot().Potions.Num() : GameMode->GetPotions().Num();
    AddText(SideContent, FText::FromString(FString::Printf(TEXT("奥黛塔\n生命 %d / %d\n金币 %d\n卡牌 %d\n藏品 %d\n药水 %d"),
        CurrentHp, CurrentMaxHp, GameMode->GetGold(), GameMode->GetDeck().Num(), GameMode->GetRelics().Num(), CurrentPotionCount)),
        FLinearColor(0.85f, 0.88f, 0.94f), 16, 18.0f);

    if (GameMode->IsCombatActive())
    {
        const FCombatSnapshot& Combat = GameMode->GetCombatSnapshot();
        AddText(SideContent, FText::FromString(FString::Printf(TEXT("战斗回合 %d\n能量 %d / %d\n力量 %d\n敌人护盾 %d"),
            Combat.Turn, Combat.Energy, Combat.MaxEnergy, Combat.PlayerStrength, Combat.EnemyBlock)), FLinearColor(0.95f, 0.78f, 0.45f), 15, 14.0f);
        AddText(SideContent, Combat.LastAction, FLinearColor(0.65f, 0.72f, 0.82f), 13);
    }
    else
    {
        AddText(SideContent, FText::FromString(TEXT("选择地图中的下一节点继续。")), FLinearColor(0.65f, 0.72f, 0.82f), 14);
    }
}

void USBattleHUD::ShowHome()
{
    AddText(MainContent, FText::FromString(TEXT("开始你的第一局")), FLinearColor(0.35f, 0.90f, 1.0f), 30, 12.0f);
    AddText(MainContent, FText::FromString(TEXT("奥黛塔 / 战士\n初始卡组：打击 x5、防御 x4、痛击 x1\n每回合 3 点能量，登上三层尖塔即可离开。")), FLinearColor(0.82f, 0.86f, 0.92f), 17, 18.0f);
    AddText(MainContent, FText::FromString(TEXT("地图种子")), FLinearColor(0.65f, 0.72f, 0.82f), 14, 4.0f);
    SAssignNew(SeedInput, SEditableTextBox)
        .Text(FText::FromString(TEXT("1337")))
        .HintText(FText::FromString(TEXT("输入数字种子")));
    MainContent->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 14.0f)[SeedInput.ToSharedRef()];
    AddButton(MainContent, FText::FromString(TEXT("选择奥黛塔并开始游戏")), [this]() { StartOdette(); }, FLinearColor(0.08f, 0.42f, 0.48f, 1.0f));
    AddText(MainContent, FText::FromString(TEXT("也可以使用控制台命令：SS3D character odette / SS3D new 1337")), FLinearColor(0.48f, 0.56f, 0.68f), 13, 4.0f);
}

void USBattleHUD::ShowMap()
{
    AddText(MainContent, FText::FromString(TEXT("规划路线")), FLinearColor(0.35f, 0.90f, 1.0f), 28, 8.0f);
    AddText(MainContent, FText::FromString(TEXT("选择当前节点连接的下一节点。每一条路线都会影响你的资源和战斗节奏。")), FLinearColor(0.75f, 0.82f, 0.90f), 15, 16.0f);

    const TArray<FMapNodeData> Available = GameMode->GetMapManager()->GetAvailableNextNodes();
    if (Available.Num() == 0)
    {
        AddText(MainContent, FText::FromString(TEXT("当前没有可选择节点。")), FLinearColor(0.90f, 0.45f, 0.45f), 16);
        return;
    }
    for (const FMapNodeData& Node : Available)
    {
        const int32 NodeId = Node.NodeId;
        const FText Label = FText::FromString(FString::Printf(TEXT("[%d] %s"), NodeId, *NodeTypeLabel(Node.NodeType).ToString()));
        AddButton(MainContent, Label, [this, NodeId]() { ExecuteCommand(FString::Printf(TEXT("select %d"), NodeId)); });
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

    AddText(MainContent, FText::FromString(TEXT("手牌")), FLinearColor(0.35f, 0.90f, 1.0f), 20, 8.0f);
    for (int32 Index = 0; Index < Combat.Hand.Num(); ++Index)
    {
        const FCardData Card = Combat.Hand[Index];
        const int32 CardIndex = Index;
        const FText Label = FText::FromString(FString::Printf(TEXT("[%d] %s  |  %d 能量\n%s"), CardIndex, *Card.Name.ToString(), Card.Cost, *Card.Description.ToString()));
        const FLinearColor Color = Card.Type == ECardType::Attack ? FLinearColor(0.38f, 0.16f, 0.16f, 1.0f) : FLinearColor(0.12f, 0.25f, 0.34f, 1.0f);
        AddButton(MainContent, Label, [this, CardIndex]() { ExecuteCommand(FString::Printf(TEXT("play %d"), CardIndex)); }, Color);
    }
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
    AddText(MainContent, FText::FromString(TEXT("战斗奖励")), FLinearColor(0.95f, 0.78f, 0.35f), 28, 8.0f);
    AddText(MainContent, FText::FromString(TEXT("选择一张卡牌加入卡组，也可以跳过。")), FLinearColor(0.75f, 0.82f, 0.90f), 15, 14.0f);
    const TArray<FCardData>& Rewards = GameMode->GetPendingRewards();
    for (int32 Index = 0; Index < Rewards.Num(); ++Index)
    {
        const int32 RewardIndex = Index;
        AddButton(MainContent, FText::FromString(FString::Printf(TEXT("%s\n%s"), *Rewards[Index].Name.ToString(), *Rewards[Index].Description.ToString())), [this, RewardIndex]()
        {
            ExecuteCommand(FString::Printf(TEXT("reward %d"), RewardIndex));
        }, FLinearColor(0.28f, 0.22f, 0.10f, 1.0f));
    }
    AddButton(MainContent, FText::FromString(TEXT("跳过奖励")), [this]() { ExecuteCommand(TEXT("reward -1")); }, FLinearColor(0.18f, 0.20f, 0.25f, 1.0f));
}

void USBattleHUD::ShowRest()
{
    AddText(MainContent, FText::FromString(TEXT("休息点")), FLinearColor(0.45f, 0.88f, 0.60f), 28, 8.0f);
    AddText(MainContent, FText::FromString(TEXT("在这里恢复生命，或升级当前卡组的第一张可升级卡牌。")), FLinearColor(0.75f, 0.82f, 0.90f), 16, 16.0f);
    AddButton(MainContent, FText::FromString(TEXT("休息并恢复生命")), [this]() { ExecuteCommand(TEXT("rest heal")); }, FLinearColor(0.15f, 0.38f, 0.24f, 1.0f));
    AddButton(MainContent, FText::FromString(TEXT("升级卡牌")), [this]() { ExecuteCommand(TEXT("rest upgrade")); }, FLinearColor(0.24f, 0.30f, 0.18f, 1.0f));
}

void USBattleHUD::ShowEvent()
{
    AddText(MainContent, FText::FromString(TEXT("记忆事件")), FLinearColor(0.70f, 0.55f, 0.95f), 28, 8.0f);
    AddText(MainContent, FText::FromString(TEXT("一块记忆碎片悬浮在面前，你可以吸收它，或将它兑换成金币。")), FLinearColor(0.75f, 0.82f, 0.90f), 16, 16.0f);
    AddButton(MainContent, FText::FromString(TEXT("接受记忆碎片：恢复 12 点生命")), [this]() { ExecuteCommand(TEXT("event 0")); }, FLinearColor(0.28f, 0.20f, 0.40f, 1.0f));
    AddButton(MainContent, FText::FromString(TEXT("拒绝记忆碎片：获得 40 金币")), [this]() { ExecuteCommand(TEXT("event 1")); }, FLinearColor(0.32f, 0.24f, 0.12f, 1.0f));
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
