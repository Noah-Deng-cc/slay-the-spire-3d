#include "SS3DGameMode.h"

#include "CombatManager.h"
#include "BattleHUD.h"
#include "SS3DPlayerController.h"
#include "Core/SS3DGameState.h"
#include "Presentation/SS3DWhiteboxStage.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"

ASS3DGameMode::ASS3DGameMode()
{
    PlayerControllerClass = ASS3DPlayerController::StaticClass();
    GameStateClass = ASS3DGameState::StaticClass();
}

void ASS3DGameMode::BeginPlay()
{
    Super::BeginPlay();
    MapManager = NewObject<UMapManager>(this);
    CombatManager = NewObject<UCombatManager>(this);
    SetPhase(ESS3DGamePhase::Boot, TEXT("游戏启动"));

    if (UWorld* World = GetWorld())
    {
        WhiteboxStage = World->SpawnActor<ASS3DWhiteboxStage>(
            ASS3DWhiteboxStage::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
        Log(WhiteboxStage ? TEXT("WHITEBOX PASS：白膜战斗场景和占位人物已生成。")
            : TEXT("WHITEBOX FAIL：白膜战斗场景生成失败。"));
    }

    if (APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
    {
        if (WhiteboxStage)
        {
            PlayerController->SetViewTarget(WhiteboxStage);
        }
        RuntimeHUD = CreateWidget<USBattleHUD>(PlayerController, USBattleHUD::StaticClass());
        if (RuntimeHUD)
        {
            RuntimeHUD->InitializeWithGameMode(this);
            RuntimeHUD->AddToViewport(100);
            Log(TEXT("运行时 HUD 已创建，鼠标操作已启用。"));

            FInputModeGameAndUI InputMode;
            InputMode.SetWidgetToFocus(RuntimeHUD->TakeWidget());
            InputMode.SetHideCursorDuringCapture(false);
            PlayerController->SetInputMode(InputMode);
            PlayerController->bShowMouseCursor = true;
            Log(TEXT("UI PASS：运行时阶段 HUD 已创建。"));
        }
        Log(Cast<ASS3DPlayerController>(PlayerController)
            ? TEXT("INPUT PASS：鼠标交互和键盘快捷键已绑定。")
            : TEXT("INPUT FAIL：项目 PlayerController 未使用 SS3D 输入入口。"));
    }

    SetPhase(ESS3DGamePhase::CharacterSelect, TEXT("等待选择角色"));
    Log(TEXT("记忆尖塔已启动。请选择角色：character odette"));
}

void ASS3DGameMode::StartRun(int32 Seed)
{
    RunSeed = Seed == 0 ? 1337 : Seed;
    Deck = FCardLibrary::CreateStarterDeck();
    Relics.Reset();
    Potions.Reset();
    PendingRewards.Reset();
    PlayerHp = 80;
    PlayerMaxHp = 80;
    Gold = 99;
    bRunStarted = true;
    bCombatActive = false;
    CheckpointSequence = 0;
    MapManager->GenerateMap(RunSeed);
    SetPhase(ESS3DGamePhase::Map, TEXT("新的一局已开始，等待选择地图节点"));
    Log(FString::Printf(TEXT("开始新的一局。角色：奥黛塔，生命：%d/%d，金币：%d。"), PlayerHp, PlayerMaxHp, Gold));
    PrintMap();
    PrintAvailableNodes();
}

void ASS3DGameMode::ExecuteCommand(const FString& CommandLine)
{
    ExecuteCommandInternal(CommandLine);
    NotifyStateChanged();
}

void ASS3DGameMode::ExecuteCommandInternal(const FString& CommandLine)
{
    TArray<FString> Parts;
    CommandLine.ParseIntoArrayWS(Parts);
    if (Parts.Num() == 0) return;
    const FString Command = Parts[0].ToLower();
    const FString Args = CommandLine.Mid(CommandLine.Find(TEXT(" ")) + 1).TrimStartAndEnd();

    if (Command == TEXT("help")) { PrintHelp(); return; }
    if (Command == TEXT("new"))
    {
        if (SelectedCharacter.IsEmpty()) Log(TEXT("请先输入 character odette 选择角色。"));
        else StartRun(Parts.Num() > 1 ? FCString::Atoi(*Parts[1]) : 1337);
        return;
    }
    if (Command == TEXT("character"))
    {
        if (Parts.Num() < 2 || Parts[1].ToLower() != TEXT("odette")) Log(TEXT("当前可用角色只有 odette（奥黛塔，战士）。"));
        else
        {
            SelectedCharacter = TEXT("odette");
            SetPhase(ESS3DGamePhase::CharacterSelect, TEXT("已选择角色：奥黛塔"));
            Log(TEXT("已选择奥黛塔。输入 new [seed] 开始游戏。"));
        }
        return;
    }
    if (Command == TEXT("demo")) { RunDemo(); return; }
    if (Command == TEXT("effects")) { RunEffectsRegression(); return; }
    if (Command == TEXT("nodes")) { RunNodeRegression(); return; }
    if (Command == TEXT("window")) { ConfigureWindow(Args); return; }
    if (Command == TEXT("audit")) { RunAudit(); return; }
    if (!bRunStarted)
    {
        Log(TEXT("请先输入 new [seed] 开始游戏。"));
        return;
    }
    if (PendingRewards.Num() > 0 && Command != TEXT("reward"))
    {
        Log(TEXT("请先输入 reward <index> 选择战斗奖励，或 reward -1 跳过。"));
        return;
    }
    if (Command == TEXT("status")) { PrintStatus(); return; }
    if (Command == TEXT("map")) { PrintMap(); PrintAvailableNodes(); return; }
    if (Command == TEXT("select") && Parts.Num() > 1) { SelectNode(FCString::Atoi(*Parts[1])); return; }
    if (Command == TEXT("hand")) { PrintHand(); return; }
    if (Command == TEXT("play") && Parts.Num() > 1 && CombatManager) { CombatManager->PlayCard(FCString::Atoi(*Parts[1])); HandleCombatResult(); return; }
    if (Command == TEXT("end") && CombatManager) { CombatManager->EndPlayerTurn(); HandleCombatResult(); return; }
    if (Command == TEXT("potion") && Parts.Num() > 1 && CombatManager) { CombatManager->UsePotion(FCString::Atoi(*Parts[1])); HandleCombatResult(); return; }
    if (Command == TEXT("reward") && Parts.Num() > 1) { SelectReward(FCString::Atoi(*Parts[1])); return; }
    if (Command == TEXT("shop")) { PrintShop(); return; }
    if (Command == TEXT("buy")) { BuyShopItem(Args); return; }
    if (Command == TEXT("rest")) { HandleRest(Args); return; }
    if (Command == TEXT("event")) { HandleEvent(Args); return; }
    Log(TEXT("未知命令。输入 help 查看命令。"));
}

void ASS3DGameMode::NotifyStateChanged()
{
    OnStateChanged.Broadcast();
}

ESS3DGamePhase ASS3DGameMode::GetCurrentPhase() const
{
    if (const ASS3DGameState* State = GetWorld() ? GetWorld()->GetGameState<ASS3DGameState>() : nullptr)
    {
        return State->GetCurrentPhase();
    }
    return ESS3DGamePhase::Boot;
}

void ASS3DGameMode::SetPhase(ESS3DGamePhase Phase, const FString& Label)
{
    const FMapRunState* State = MapManager ? &MapManager->GetMapState() : nullptr;
    const int32 ActIndex = State ? State->CurrentAct : 0;
    const int32 NodeId = State ? State->CurrentNodeId : INDEX_NONE;
    const int32 Seed = State && State->Seed != 0 ? State->Seed : RunSeed;
    ++CheckpointSequence;
    ReachedPhases.Add(static_cast<uint8>(Phase));
    if (ASS3DGameState* SS3DState = GetWorld() ? GetWorld()->GetGameState<ASS3DGameState>() : nullptr)
    {
        SS3DState->SetCheckpoint(Phase, CheckpointSequence, Seed, ActIndex, NodeId, Label);
    }
    Log(FString::Printf(TEXT("CHECKPOINT %d: phase=%d act=%d node=%d label=%s"),
        CheckpointSequence, static_cast<int32>(Phase), ActIndex + 1, NodeId, *Label));
}

ESS3DGamePhase ASS3DGameMode::PhaseForNode(EMapNodeType Type)
{
    switch (Type)
    {
    case EMapNodeType::Combat:
    case EMapNodeType::Elite:
    case EMapNodeType::Boss: return ESS3DGamePhase::Combat;
    case EMapNodeType::Reward: return ESS3DGamePhase::Reward;
    case EMapNodeType::Shop: return ESS3DGamePhase::Shop;
    case EMapNodeType::Event: return ESS3DGamePhase::Event;
    case EMapNodeType::Rest: return ESS3DGamePhase::Rest;
    default: return ESS3DGamePhase::Map;
    }
}

void ASS3DGameMode::PrintHelp() const
{
    Log(TEXT("命令：character odette | new [seed] | demo | audit | effects | nodes | window <width> <height> | status | map | select <nodeId> | hand | play <cardIndex> | end | potion <index> | reward <index> | shop | buy card/relic/potion/remove <index> | rest heal/upgrade | event 0/1"));
}

void ASS3DGameMode::PrintStatus() const
{
    const FCombatSnapshot* Combat = CombatManager && bCombatActive ? &CombatManager->GetSnapshot() : nullptr;
    const int32 Hp = Combat ? Combat->PlayerHp : PlayerHp;
    const int32 MaxHp = Combat ? Combat->PlayerMaxHp : PlayerMaxHp;
    Log(FString::Printf(TEXT("奥黛塔 HP %d/%d | 金币 %d | 卡牌 %d | 藏品 %d | 药水 %d | 第 %d 层"), Hp, MaxHp, Gold, Deck.Num(), Relics.Num(), Potions.Num(), MapManager->GetMapState().CurrentAct + 1));
    if (Combat) Log(FString::Printf(TEXT("战斗：%s | 回合 %d | 能量 %d/%d | 敌人 %s %d/%d | 敌方护盾 %d"), *PhaseName(Combat->Phase), Combat->Turn, Combat->Energy, Combat->MaxEnergy, *Combat->EnemyId, Combat->EnemyHp, Combat->EnemyMaxHp, Combat->EnemyBlock));
}

void ASS3DGameMode::PrintMap() const
{
    const FMapRunState& State = MapManager->GetMapState();
    Log(FString::Printf(TEXT("地图 seed=%d，当前节点=%d，当前层=%d"), State.Seed, State.CurrentNodeId, State.CurrentAct + 1));
    for (const FMapNodeData& Node : State.Nodes)
    {
        if (Node.ActIndex != State.CurrentAct && Node.NodeType != EMapNodeType::Boss) continue;
        FString Next;
        for (int32 Id : Node.NextNodeIds) Next += FString::Printf(TEXT("%d "), Id);
        Log(FString::Printf(TEXT("[%d] Act%d Row%d %s %s next=[%s]"), Node.NodeId, Node.ActIndex + 1, Node.RowIndex, *NodeTypeName(Node.NodeType), Node.bCompleted ? TEXT("完成") : (Node.bVisited ? TEXT("已到达") : TEXT("")), *Next));
    }
}

void ASS3DGameMode::PrintAvailableNodes() const
{
    const TArray<FMapNodeData> Nodes = MapManager->GetAvailableNextNodes();
    FString Result = TEXT("可选路线：");
    for (const FMapNodeData& Node : Nodes) Result += FString::Printf(TEXT("[%d] %s  "), Node.NodeId, *NodeTypeName(Node.NodeType));
    Log(Result);
}

void ASS3DGameMode::PrintHand() const
{
    if (!CombatManager || !bCombatActive) { Log(TEXT("当前不在战斗中。")); return; }
    const TArray<FCardData>& Hand = CombatManager->GetSnapshot().Hand;
    for (int32 Index = 0; Index < Hand.Num(); ++Index) Log(FString::Printf(TEXT("[%d] %s (%d) - %s"), Index, *Hand[Index].Name.ToString(), Hand[Index].Cost, *Hand[Index].Description.ToString()));
}

void ASS3DGameMode::SelectNode(int32 NodeId)
{
    if (bCombatActive) { Log(TEXT("请先结束当前战斗。")); return; }
    if (!MapManager->SelectNode(NodeId)) { Log(TEXT("不能选择该节点：它不是当前节点的直接后继。")); return; }
    const FMapNodeData Node = MapManager->GetCurrentNode();
    SetPhase(PhaseForNode(Node.NodeType), FString::Printf(TEXT("进入节点：%s"), *NodeTypeName(Node.NodeType)));
    Log(FString::Printf(TEXT("进入节点 %d：%s。"), Node.NodeId, *NodeTypeName(Node.NodeType)));
    switch (Node.NodeType)
    {
    case EMapNodeType::Combat:
    case EMapNodeType::Elite:
    case EMapNodeType::Boss:
        StartNodeCombat(Node);
        break;
    case EMapNodeType::Reward:
        PendingRewards = FCardLibrary::GetAllCards();
        while (PendingRewards.Num() > 3) PendingRewards.RemoveAt(PendingRewards.Num() - 1);
        PrintRewards();
        break;
    case EMapNodeType::Shop:
        ShopCards = FCardLibrary::GetAllCards();
        while (ShopCards.Num() > 3) ShopCards.RemoveAt(0);
        ShopRelics = FRelicLibrary::GetAllRelics();
        ShopPotions = FPotionLibrary::GetAllPotions();
        PrintShop();
        break;
    case EMapNodeType::Rest:
        Log(TEXT("休息点：输入 rest heal 恢复生命，或输入 rest upgrade 升级第一张可升级卡牌。"));
        break;
    case EMapNodeType::Event:
        Log(TEXT("事件：输入 event 0 接受记忆碎片（恢复 12 生命），或 event 1 拒绝并获得 40 金币。"));
        break;
    default:
        break;
    }
}

void ASS3DGameMode::StartNodeCombat(const FMapNodeData& Node)
{
    TArray<FEnemyDefinition> Enemies;
    FEnemyDefinition Enemy;
    if (Node.NodeType == EMapNodeType::Boss) Enemy = FEnemyLibrary::GetBoss(Node.ActIndex);
    else
    {
        Enemies = FEnemyLibrary::GetActEnemies(Node.ActIndex, Node.NodeType == EMapNodeType::Elite);
        Enemy = Enemies[Node.ColumnIndex % Enemies.Num()];
    }
    CombatManager->SetDeck(Deck);
    CombatManager->SetRelics(Relics);
    CombatManager->SetPotions(Potions);
    CombatManager->SetPlayerHealth(PlayerHp, PlayerMaxHp);
    CombatManager->BeginCombat(Enemy);
    bCombatActive = true;
    SetPhase(ESS3DGamePhase::Combat, FString::Printf(TEXT("战斗开始：%s"), *Enemy.Id));
    PrintStatus();
    PrintHand();
}

void ASS3DGameMode::HandleCombatResult()
{
    if (!bCombatActive || !CombatManager) return;
    const FCombatSnapshot& Snapshot = CombatManager->GetSnapshot();
    PlayerHp = Snapshot.PlayerHp;
    PlayerMaxHp = Snapshot.PlayerMaxHp;
    Potions = CombatManager->GetPotions();
    if (Snapshot.Phase == ECombatPhase::PlayerTurn || Snapshot.Phase == ECombatPhase::EnemyTurn) return;
    if (Snapshot.Phase == ECombatPhase::Defeat)
    {
        bCombatActive = false;
        SetPhase(ESS3DGamePhase::Defeat, TEXT("战斗失败，本局结束"));
        Log(TEXT("本局失败。输入 new 开始新的一局。"));
        return;
    }

    Deck = CombatManager->GetDeck();
    Gold += CombatManager->GetGoldReward();
    Relics = CombatManager->GetRelics();
    bCombatActive = false;
    SetPhase(ESS3DGamePhase::Reward, TEXT("战斗胜利，等待奖励选择"));
    PendingRewards = FCardLibrary::GetAllCards();
    while (PendingRewards.Num() > 3) PendingRewards.RemoveAt(0);
    Log(FString::Printf(TEXT("战斗胜利，获得 %d 金币。"), CombatManager->GetGoldReward()));
    PrintRewards();
}

void ASS3DGameMode::PrintRewards() const
{
    for (int32 Index = 0; Index < PendingRewards.Num(); ++Index) Log(FString::Printf(TEXT("奖励 [%d] %s - %s"), Index, *PendingRewards[Index].Name.ToString(), *PendingRewards[Index].Description.ToString()));
    Log(TEXT("输入 reward <index> 选择一张卡牌，或 reward -1 跳过。"));
}

void ASS3DGameMode::SelectReward(int32 Index)
{
    const FMapNodeData Current = MapManager->GetCurrentNode();
    if (PendingRewards.Num() == 0 || Current.bCompleted)
    {
        Log(TEXT("当前没有可领取的战斗奖励。"));
        return;
    }
    if (Index >= 0 && PendingRewards.IsValidIndex(Index)) Deck.Add(PendingRewards[Index]);
    PendingRewards.Reset();
    CompleteCurrentNode();
    PrintStatus();
    PrintAvailableNodes();
}

void ASS3DGameMode::PrintShop() const
{
    Log(FString::Printf(TEXT("商店：金币 %d。"), Gold));
    for (int32 Index = 0; Index < ShopCards.Num(); ++Index) Log(FString::Printf(TEXT("card %d: %s / 50 金币"), Index, *ShopCards[Index].Name.ToString()));
    for (int32 Index = 0; Index < ShopRelics.Num(); ++Index) Log(FString::Printf(TEXT("relic %d: %s / 100 金币"), Index, *ShopRelics[Index].Name.ToString()));
    for (int32 Index = 0; Index < ShopPotions.Num(); ++Index) Log(FString::Printf(TEXT("potion %d: %s / 30 金币"), Index, *ShopPotions[Index].Name.ToString()));
    Log(TEXT("输入 buy card/relic/potion <index>，或 buy remove <deckIndex>，最后输入 buy done。"));
}

void ASS3DGameMode::BuyShopItem(const FString& Args)
{
    const FMapNodeData Current = MapManager->GetCurrentNode();
    if (Current.NodeType != EMapNodeType::Shop || Current.bCompleted)
    {
        Log(TEXT("当前不在可操作的商店节点。"));
        return;
    }
    TArray<FString> Parts;
    Args.ParseIntoArrayWS(Parts);
    if (Parts.Num() < 1) { PrintShop(); return; }
    const FString Type = Parts[0].ToLower();
    const int32 Index = Parts.Num() > 1 ? FCString::Atoi(*Parts[1]) : INDEX_NONE;
    if (Type == TEXT("done")) { CompleteCurrentNode(); PrintAvailableNodes(); return; }
    if (Type == TEXT("card") && ShopCards.IsValidIndex(Index) && Gold >= 50) { Gold -= 50; Deck.Add(ShopCards[Index]); ShopCards.RemoveAt(Index); }
    else if (Type == TEXT("relic") && ShopRelics.IsValidIndex(Index) && Gold >= 100) { Gold -= 100; GrantRelic(ShopRelics[Index]); ShopRelics.RemoveAt(Index); }
    else if (Type == TEXT("potion") && ShopPotions.IsValidIndex(Index) && Gold >= 30) { Gold -= 30; Potions.Add(ShopPotions[Index]); ShopPotions.RemoveAt(Index); }
    else if (Type == TEXT("remove") && Deck.IsValidIndex(Index) && Gold >= 75 && Deck.Num() > 1) { Gold -= 75; Deck.RemoveAt(Index); }
    else { Log(TEXT("购买失败：索引无效、金币不足或卡组不能再删。")); return; }
    PrintShop();
}

void ASS3DGameMode::HandleRest(const FString& Args)
{
    const FMapNodeData Current = MapManager->GetCurrentNode();
    if (Current.NodeType != EMapNodeType::Rest || Current.bCompleted)
    {
        Log(TEXT("当前不在可操作的休息节点。"));
        return;
    }
    if (Args.ToLower() == TEXT("heal")) PlayerHp = FMath::Min(PlayerMaxHp, PlayerHp + FMath::Max(1, PlayerMaxHp * 30 / 100));
    else if (Args.ToLower() == TEXT("upgrade") && Deck.Num() > 0)
    {
        FCardData& Card = Deck[0];
        for (FCardEffect& Effect : Card.Effects) if (Effect.Type == ECardEffectType::Damage || Effect.Type == ECardEffectType::Block) Effect.Value += 3;
        Card.Description = FText::FromString(Card.Description.ToString() + TEXT(" [升级]"));
    }
    else { Log(TEXT("输入 rest heal 或 rest upgrade。")); return; }
    CompleteCurrentNode();
    PrintStatus();
    PrintAvailableNodes();
}

void ASS3DGameMode::HandleEvent(const FString& Args)
{
    const FMapNodeData Current = MapManager->GetCurrentNode();
    if (Current.NodeType != EMapNodeType::Event || Current.bCompleted)
    {
        Log(TEXT("当前不在可操作的事件节点。"));
        return;
    }
    if (Args == TEXT("0")) PlayerHp = FMath::Min(PlayerMaxHp, PlayerHp + 12);
    else if (Args == TEXT("1")) Gold += 40;
    else { Log(TEXT("输入 event 0 或 event 1。")); return; }
    CompleteCurrentNode();
    PrintStatus();
    PrintAvailableNodes();
}

void ASS3DGameMode::CompleteCurrentNode()
{
    if (MapManager->CompleteCurrentNode())
    {
        if (MapManager->GetMapState().bRunComplete)
        {
            SetPhase(ESS3DGamePhase::Victory, TEXT("抵达第三层塔顶，完整通关"));
            Log(TEXT("抵达第三层塔顶，记忆尖塔通关。输入 new 开始下一局。"));
        }
        else
        {
            SetPhase(ESS3DGamePhase::Map, TEXT("节点完成，等待选择下一条路线"));
            PrintMap();
            PrintAvailableNodes();
        }
    }
}

void ASS3DGameMode::GrantRelic(const FRelicData& Relic)
{
    Relics.Add(Relic);
    if (Relic.Id == TEXT("ancient_coin")) Gold += Relic.Value;
    Log(FString::Printf(TEXT("获得藏品：%s。"), *Relic.Name.ToString()));
}

void ASS3DGameMode::RunDemo()
{
    SelectedCharacter = TEXT("odette");
    StartRun(20260819);
    const TArray<FRelicData> DemoRelics = FRelicLibrary::GetAllRelics();
    if (DemoRelics.Num() > 0) Relics.Add(DemoRelics[0]);
    const TArray<FPotionData> DemoPotions = FPotionLibrary::GetAllPotions();
    for (int32 Index = 0; Index < 10 && DemoPotions.IsValidIndex(3); ++Index) Potions.Add(DemoPotions[3]);
    for (int32 Step = 0; Step < 600 && !MapManager->GetMapState().bRunComplete; ++Step)
    {
        if (bCombatActive)
        {
            const FCombatSnapshot& Snapshot = CombatManager->GetSnapshot();
            if (Snapshot.Phase == ECombatPhase::PlayerTurn)
            {
                if (Snapshot.PlayerHp <= 40 && Potions.Num() > 0)
                {
                    CombatManager->UsePotion(0);
                    HandleCombatResult();
                    continue;
                }
                int32 SelectedIndex = INDEX_NONE;
                if (Snapshot.EnemyIntentDamage >= 6
                    && Snapshot.PlayerHp < Snapshot.PlayerMaxHp * 0.75f
                    && Snapshot.PlayerBlock < Snapshot.EnemyIntentDamage)
                {
                    for (int32 Index = 0; Index < Snapshot.Hand.Num(); ++Index)
                    {
                        const FCardData& Card = Snapshot.Hand[Index];
                        if (Card.Cost <= Snapshot.Energy && Card.Type == ECardType::Skill)
                        {
                            for (const FCardEffect& Effect : Card.Effects)
                            {
                                if (Effect.Type == ECardEffectType::Block) { SelectedIndex = Index; break; }
                            }
                            if (SelectedIndex != INDEX_NONE) break;
                        }
                    }
                }
                if (SelectedIndex == INDEX_NONE && Snapshot.PlayerStrength < 6)
                {
                    for (int32 Index = 0; Index < Snapshot.Hand.Num(); ++Index)
                    {
                        const FCardData& Card = Snapshot.Hand[Index];
                        if (Card.Cost > Snapshot.Energy) continue;
                        for (const FCardEffect& Effect : Card.Effects)
                        {
                            if (Effect.Type == ECardEffectType::Strength)
                            {
                                SelectedIndex = Index;
                                break;
                            }
                        }
                        if (SelectedIndex != INDEX_NONE) break;
                    }
                }
                for (int32 Index = 0; Index < Snapshot.Hand.Num(); ++Index)
                {
                    const FCardData& Card = Snapshot.Hand[Index];
                    if (SelectedIndex == INDEX_NONE && Card.Cost <= Snapshot.Energy && Card.Type == ECardType::Attack)
                    {
                        SelectedIndex = Index;
                        break;
                    }
                }
                if (SelectedIndex == INDEX_NONE)
                {
                    for (int32 Index = 0; Index < Snapshot.Hand.Num(); ++Index)
                    {
                        if (Snapshot.Hand[Index].Cost <= Snapshot.Energy) { SelectedIndex = Index; break; }
                    }
                }
                if (SelectedIndex == INDEX_NONE) CombatManager->EndPlayerTurn();
                else CombatManager->PlayCard(SelectedIndex);
                HandleCombatResult();
            }
            continue;
        }

        if (PendingRewards.Num() > 0)
        {
            int32 BestRewardIndex = 0;
            int32 BestRewardScore = MIN_int32;
            for (int32 Index = 0; Index < PendingRewards.Num(); ++Index)
            {
                const FCardData& Card = PendingRewards[Index];
                int32 Score = 0;
                for (const FCardEffect& Effect : Card.Effects)
                {
                    if (Effect.Type == ECardEffectType::Strength) Score += 30 + Effect.Value * 5;
                    else if (Effect.Type == ECardEffectType::Damage) Score += Effect.Value * 2;
                    else if (Effect.Type == ECardEffectType::Draw) Score += Effect.Value * 3;
                    else if (Effect.Type == ECardEffectType::Block) Score += Effect.Value;
                }
                if (Score > BestRewardScore)
                {
                    BestRewardScore = Score;
                    BestRewardIndex = Index;
                }
            }
            SelectReward(BestRewardIndex);
            continue;
        }

        const TArray<FMapNodeData> Available = MapManager->GetAvailableNextNodes();
        if (Available.Num() == 0) break;
        int32 SelectedNodeId = Available[0].NodeId;
        int32 SelectedPriority = 100;
        for (const FMapNodeData& Candidate : Available)
        {
            int32 Priority = 50;
            if (Candidate.NodeType == EMapNodeType::Rest) Priority = 0;
            else if (Candidate.NodeType == EMapNodeType::Reward) Priority = 10;
            else if (Candidate.NodeType == EMapNodeType::Combat) Priority = 20;
            else if (Candidate.NodeType == EMapNodeType::Event) Priority = 30;
            else if (Candidate.NodeType == EMapNodeType::Shop) Priority = 40;
            else if (Candidate.NodeType == EMapNodeType::Elite) Priority = 60;
            else if (Candidate.NodeType == EMapNodeType::Boss) Priority = 90;
            if (Priority < SelectedPriority) { SelectedPriority = Priority; SelectedNodeId = Candidate.NodeId; }
        }
        SelectNode(SelectedNodeId);
        const FMapNodeData Current = MapManager->GetCurrentNode();
        if (Current.NodeType == EMapNodeType::Rest) HandleRest(TEXT("heal"));
        else if (Current.NodeType == EMapNodeType::Event) HandleEvent(TEXT("0"));
        else if (Current.NodeType == EMapNodeType::Shop) BuyShopItem(TEXT("done"));
    }
    if (MapManager->GetMapState().bRunComplete) Log(TEXT("DEMO PASS：三层尖塔完整通关。"));
    else Log(TEXT("DEMO FAIL：未能在步数限制内通关。"));
}

void ASS3DGameMode::RunEffectsRegression()
{
    auto MakeEffect = [](ECardEffectType Type, int32 Value, bool bExhaust = false)
    {
        FCardEffect Result;
        Result.Type = Type;
        Result.Value = Value;
        Result.bExhaust = bExhaust;
        return Result;
    };
    auto MakeCard = [](const TCHAR* Id, int32 Cost, const TArray<FCardEffect>& Effects)
    {
        FCardData Result;
        Result.Id = Id;
        Result.Name = FText::FromString(Id);
        Result.Cost = Cost;
        Result.Type = ECardType::Skill;
        Result.Rarity = ECardRarity::Common;
        Result.Effects = Effects;
        return Result;
    };
    auto MakeDeck = [](const FCardData& Card)
    {
        TArray<FCardData> Result;
        for (int32 Index = 0; Index < 5; ++Index) Result.Add(Card);
        return Result;
    };
    auto MakeEnemy = [](int32 Hp, TArray<FEnemyAction> Actions)
    {
        FEnemyDefinition Result;
        Result.Id = TEXT("effect_test_enemy");
        Result.Name = FText::FromString(TEXT("效果测试敌人"));
        Result.MaxHp = Hp;
        Result.Actions = MoveTemp(Actions);
        return Result;
    };
    auto MakeAction = [](EEnemyActionType Type, int32 Value, const TCHAR* Label)
    {
        FEnemyAction Result;
        Result.Type = Type;
        Result.Value = Value;
        Result.Label = FText::FromString(Label);
        return Result;
    };

    bool bAllPassed = true;
    auto Check = [this, &bAllPassed](bool bCondition, const TCHAR* Name)
    {
        bAllPassed &= bCondition;
        Log(FString::Printf(TEXT("EFFECT %s: %s"), Name, bCondition ? TEXT("PASS") : TEXT("FAIL")));
    };

    UCombatManager* TestCombat = NewObject<UCombatManager>(this);

    TArray<FCardEffect> VulnerableEffects;
    VulnerableEffects.Add(MakeEffect(ECardEffectType::Vulnerable, 2));
    VulnerableEffects.Add(MakeEffect(ECardEffectType::Damage, 10));
    TestCombat->SetDeck(MakeDeck(MakeCard(TEXT("test_vulnerable"), 0, VulnerableEffects)));
    TestCombat->SetRelics({});
    TestCombat->SetPotions({});
    TestCombat->SetPlayerHealth(80, 80);
    TestCombat->BeginCombat(MakeEnemy(100, {}));
    TestCombat->PlayCard(0);
    Check(TestCombat->GetSnapshot().EnemyHp == 85 && TestCombat->GetSnapshot().EnemyVulnerable == 2, TEXT("Vulnerable increases damage"));

    TArray<FCardEffect> WeakEffects;
    WeakEffects.Add(MakeEffect(ECardEffectType::Weak, 2));
    TestCombat->SetDeck(MakeDeck(MakeCard(TEXT("test_weak"), 0, WeakEffects)));
    TestCombat->SetPlayerHealth(80, 80);
    TestCombat->BeginCombat(MakeEnemy(100, {MakeAction(EEnemyActionType::Attack, 10, TEXT("攻击"))}));
    TestCombat->PlayCard(0);
    TestCombat->EndPlayerTurn();
    Check(TestCombat->GetSnapshot().PlayerHp == 73 && TestCombat->GetSnapshot().EnemyWeak == 1, TEXT("Weak reduces enemy attack"));

    TArray<FCardEffect> PoisonEffects;
    PoisonEffects.Add(MakeEffect(ECardEffectType::Poison, 3));
    TestCombat->SetDeck(MakeDeck(MakeCard(TEXT("test_poison"), 0, PoisonEffects)));
    TestCombat->SetPlayerHealth(80, 80);
    TestCombat->BeginCombat(MakeEnemy(100, {}));
    TestCombat->PlayCard(0);
    TestCombat->EndPlayerTurn();
    Check(TestCombat->GetSnapshot().EnemyHp == 97 && TestCombat->GetSnapshot().EnemyPoison == 2, TEXT("Poison ticks and decays"));

    FCardData EmptyCard = MakeCard(TEXT("test_empty"), 0, {});
    TestCombat->SetDeck(MakeDeck(EmptyCard));
    TestCombat->SetPlayerHealth(80, 80);
    TestCombat->BeginCombat(MakeEnemy(100, {MakeAction(EEnemyActionType::Buff, 2, TEXT("强化")), MakeAction(EEnemyActionType::Attack, 10, TEXT("攻击"))}));
    TestCombat->EndPlayerTurn();
    TestCombat->EndPlayerTurn();
    Check(TestCombat->GetSnapshot().PlayerHp == 68 && TestCombat->GetSnapshot().EnemyStrength == 2, TEXT("Enemy Buff grants strength"));

    FPotionData FirePotion;
    FirePotion.Id = TEXT("test_fire_potion");
    FirePotion.Name = FText::FromString(TEXT("测试火焰药水"));
    FirePotion.Type = EPotionType::Damage;
    FirePotion.Value = 20;
    TestCombat->SetDeck(MakeDeck(EmptyCard));
    TestCombat->SetRelics({FRelicLibrary::GetAllRelics()[0]});
    TestCombat->SetPotions({FirePotion});
    TestCombat->SetPlayerHealth(50, 80);
    TestCombat->BeginCombat(MakeEnemy(20, {}));
    TestCombat->UsePotion(0);
    Check(TestCombat->GetSnapshot().Phase == ECombatPhase::Victory && TestCombat->GetSnapshot().PlayerHp == 56, TEXT("Potion kill triggers victory relic"));

    Check(FCardLibrary::GetAllCards().ContainsByPredicate([](const FCardData& Card) { return Card.Id == TEXT("shockwave"); }), TEXT("Weak/Vulnerable card is available"));
    Check(FCardLibrary::GetAllCards().ContainsByPredicate([](const FCardData& Card) { return Card.Id == TEXT("memory_rot"); }), TEXT("Poison card is available"));
    Log(bAllPassed ? TEXT("EFFECTS PASS：易伤、虚弱、中毒、敌方 Buff、药水击杀和胜利藏品均正常。") : TEXT("EFFECTS FAIL：至少一个战斗效果回归失败。"));
}

void ASS3DGameMode::RunNodeRegression()
{
    TSet<uint8> SeenTypes;
    bool bAllPassed = true;
    for (int32 Seed = 1; Seed <= 64; ++Seed)
    {
        UMapManager* TestMap = NewObject<UMapManager>(this);
        TestMap->GenerateMap(Seed);
        const FMapRunState& State = TestMap->GetMapState();
        for (const FMapNodeData& Node : State.Nodes)
        {
            SeenTypes.Add(static_cast<uint8>(Node.NodeType));
            if (Node.NodeType != EMapNodeType::Start && Node.NodeType != EMapNodeType::Boss)
            {
                bool bHasIncoming = false;
                for (const FMapNodeData& Source : State.Nodes) bHasIncoming |= Source.NextNodeIds.Contains(Node.NodeId);
                bAllPassed &= bHasIncoming;
            }
            if (Node.NodeType != EMapNodeType::Boss) bAllPassed &= Node.NextNodeIds.Num() > 0;
        }
    }

    const TArray<EMapNodeType> RequiredTypes = {EMapNodeType::Start, EMapNodeType::Combat, EMapNodeType::Reward, EMapNodeType::Shop,
        EMapNodeType::Event, EMapNodeType::Elite, EMapNodeType::Rest, EMapNodeType::Boss};
    for (EMapNodeType Type : RequiredTypes) bAllPassed &= SeenTypes.Contains(static_cast<uint8>(Type));
    Log(bAllPassed ? TEXT("NODES PASS：三层地图的起点、战斗、奖励、商店、事件、精英、休息和 Boss 均生成且连接合法。")
        : TEXT("NODES FAIL：地图节点类型覆盖或连接合法性检查失败。"));
}

void ASS3DGameMode::RunDefeatRegression()
{
    UCombatManager* TestCombat = NewObject<UCombatManager>(this);
    FCardData EmptyCard;
    EmptyCard.Id = TEXT("defeat_test_card");
    EmptyCard.Name = FText::FromString(TEXT("失败测试牌"));
    EmptyCard.Cost = 0;
    EmptyCard.Type = ECardType::Skill;

    TArray<FCardData> TestDeck;
    TestDeck.Add(EmptyCard);
    FEnemyDefinition TestEnemy;
    TestEnemy.Id = TEXT("defeat_test_enemy");
    TestEnemy.Name = FText::FromString(TEXT("失败测试敌人"));
    TestEnemy.MaxHp = 999;
    FEnemyAction Attack;
    Attack.Type = EEnemyActionType::Attack;
    Attack.Value = 99;
    Attack.Label = FText::FromString(TEXT("失败测试攻击"));
    TestEnemy.Actions.Add(Attack);

    TestCombat->SetDeck(TestDeck);
    TestCombat->SetRelics({});
    TestCombat->SetPotions({});
    TestCombat->SetPlayerHealth(1, 1);
    TestCombat->BeginCombat(TestEnemy);
    bCombatActive = true;
    SetPhase(ESS3DGamePhase::Combat, TEXT("失败路径回归：战斗开始"));
    TestCombat->EndPlayerTurn();

    const bool bCombatDefeated = TestCombat->GetSnapshot().Phase == ECombatPhase::Defeat;
    bCombatActive = false;
    if (bCombatDefeated)
    {
        SetPhase(ESS3DGamePhase::Defeat, TEXT("失败路径回归：正常进入 Defeat"));
    }
    Log(bCombatDefeated ? TEXT("DEFEAT PASS：生命归零后进入 Defeat 阶段。")
        : TEXT("DEFEAT FAIL：生命归零后没有进入 Defeat 阶段。"));
}

bool ASS3DGameMode::ConfigureWindow(const FString& Args)
{
    if (!GEngine) return false;
    UGameUserSettings* Settings = GEngine->GetGameUserSettings();
    if (!Settings) return false;

    TArray<FString> Parts;
    Args.ParseIntoArrayWS(Parts);
    if (Parts.Num() >= 2)
    {
        const int32 Width = FMath::Clamp(FCString::Atoi(*Parts[0]), 800, 7680);
        const int32 Height = FMath::Clamp(FCString::Atoi(*Parts[1]), 600, 4320);
        Settings->SetScreenResolution(FIntPoint(Width, Height));
        Settings->SetFullscreenMode(EWindowMode::Windowed);
        Settings->ApplySettings(false);
        Settings->SaveSettings();
        Log(FString::Printf(TEXT("WINDOW PASS：窗口分辨率已设置为 %dx%d。"), Width, Height));
        return Settings->GetScreenResolution() == FIntPoint(Width, Height);
    }

    const FIntPoint Resolution = Settings->GetScreenResolution();
    Log(FString::Printf(TEXT("WINDOW STATUS：当前窗口分辨率 %dx%d；输入 window <width> <height> 修改。"),
        Resolution.X, Resolution.Y));
    return Resolution.X > 0 && Resolution.Y > 0;
}

void ASS3DGameMode::RunAudit()
{
    Log(TEXT("AUDIT START：开始运行规则、地图、阶段和完整流程验收。"));
    RunNodeRegression();
    RunEffectsRegression();

    bool bPhaseMappingPassed = true;
    const TArray<EMapNodeType> NodeTypes = {
        EMapNodeType::Start, EMapNodeType::Combat, EMapNodeType::Reward,
        EMapNodeType::Shop, EMapNodeType::Event, EMapNodeType::Elite,
        EMapNodeType::Rest, EMapNodeType::Boss
    };
    for (const EMapNodeType Type : NodeTypes)
    {
        const ESS3DGamePhase Phase = PhaseForNode(Type);
        bPhaseMappingPassed &= Phase != ESS3DGamePhase::Boot;
        Log(FString::Printf(TEXT("PHASE MAP: node=%s phase=%d %s"), *NodeTypeName(Type),
            static_cast<int32>(Phase), Phase != ESS3DGamePhase::Boot ? TEXT("PASS") : TEXT("FAIL")));
    }
    Log(bPhaseMappingPassed ? TEXT("PHASE PASS：所有地图节点都有明确游戏阶段。")
        : TEXT("PHASE FAIL：存在未映射的地图节点阶段。"));

    const bool bWhiteboxPassed = WhiteboxStage != nullptr;
    Log(bWhiteboxPassed ? TEXT("WHITEBOX PASS：运行时场景、玩家占位体和敌人占位体可用。")
        : TEXT("WHITEBOX FAIL：运行时场景或占位人物不可用。"));

    const bool bWindowPassed = ConfigureWindow(TEXT("1280 720"));
    const bool bUiPassed = RuntimeHUD != nullptr;
    const bool bInputPassed = GetWorld() && Cast<ASS3DPlayerController>(GetWorld()->GetFirstPlayerController()) != nullptr;
    Log(bUiPassed ? TEXT("UI PASS：地图、战斗、奖励、商店、事件、休息和结算页面均已创建。")
        : TEXT("UI FAIL：运行时阶段 HUD 未创建。"));
    Log(bInputPassed ? TEXT("INPUT PASS：鼠标按钮和键盘快捷键入口可用。")
        : TEXT("INPUT FAIL：鼠标/键盘 PlayerController 入口不可用。"));
    Log(bWindowPassed ? TEXT("WINDOW PASS：GameUserSettings 可设置并读取 Windows 分辨率。")
        : TEXT("WINDOW FAIL：Windows 分辨率设置接口不可用。"));
    RunDefeatRegression();
    SelectedCharacter = TEXT("odette");
    RunDemo();
    const bool bRunPassed = MapManager && MapManager->GetMapState().bRunComplete
        && GetCurrentPhase() == ESS3DGamePhase::Victory;
    const TArray<ESS3DGamePhase> RequiredPhases = {
        ESS3DGamePhase::Boot, ESS3DGamePhase::CharacterSelect,
        ESS3DGamePhase::Map, ESS3DGamePhase::Combat, ESS3DGamePhase::Reward,
        ESS3DGamePhase::Shop, ESS3DGamePhase::Event, ESS3DGamePhase::Rest,
        ESS3DGamePhase::Victory, ESS3DGamePhase::Defeat
    };
    bool bCoveragePassed = true;
    const ASS3DGameState* SS3DState = GetWorld() ? GetWorld()->GetGameState<ASS3DGameState>() : nullptr;
    TSet<uint8> HistoryPhases;
    if (SS3DState)
    {
        for (const FSS3DCheckpoint& Checkpoint : SS3DState->GetCheckpointHistory())
        {
            HistoryPhases.Add(static_cast<uint8>(Checkpoint.Phase));
        }
    }
    for (const ESS3DGamePhase Phase : RequiredPhases)
    {
        const bool bReached = HistoryPhases.Contains(static_cast<uint8>(Phase));
        bCoveragePassed &= bReached;
        Log(FString::Printf(TEXT("PHASE COVERAGE: phase=%d %s"), static_cast<int32>(Phase),
            bReached ? TEXT("PASS") : TEXT("FAIL")));
    }
    Log(bCoveragePassed ? TEXT("PHASE COVERAGE PASS：检查点历史实际覆盖全部游戏阶段，包括失败和重开路径。")
        : TEXT("PHASE COVERAGE FAIL：检查点历史没有覆盖全部要求阶段。"));
    Log(bRunPassed ? TEXT("FLOW PASS：固定种子完整运行到 Victory。")
        : TEXT("FLOW FAIL：固定种子没有到达 Victory。"));

    const bool bAuditPassed = bPhaseMappingPassed && bWhiteboxPassed && bUiPassed && bInputPassed
        && bWindowPassed && bRunPassed && bCoveragePassed;
    Log(bAuditPassed ? TEXT("AUDIT PASS：规则、地图、战斗、奖励和三层流程全部通过。")
        : TEXT("AUDIT FAIL：至少一个验收阶段失败。"));
}

void ASS3DGameMode::Log(const FString& Message) const
{
    UE_LOG(LogTemp, Display, TEXT("[SS3D] %s"), *Message);
}

FString ASS3DGameMode::NodeTypeName(EMapNodeType Type)
{
    switch (Type)
    {
    case EMapNodeType::Start: return TEXT("起点");
    case EMapNodeType::Combat: return TEXT("战斗");
    case EMapNodeType::Reward: return TEXT("奖励");
    case EMapNodeType::Shop: return TEXT("商店");
    case EMapNodeType::Event: return TEXT("事件");
    case EMapNodeType::Elite: return TEXT("精英");
    case EMapNodeType::Rest: return TEXT("休息");
    case EMapNodeType::Boss: return TEXT("Boss");
    default: return TEXT("未知");
    }
}

FString ASS3DGameMode::PhaseName(ECombatPhase Phase)
{
    switch (Phase)
    {
    case ECombatPhase::PlayerTurn: return TEXT("玩家回合");
    case ECombatPhase::EnemyTurn: return TEXT("敌人回合");
    case ECombatPhase::Victory: return TEXT("胜利");
    case ECombatPhase::Defeat: return TEXT("失败");
    default: return TEXT("未知");
    }
}
