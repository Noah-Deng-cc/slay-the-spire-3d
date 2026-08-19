#include "SS3DGameMode.h"

#include "CombatManager.h"
#include "SS3DPlayerController.h"

ASS3DGameMode::ASS3DGameMode()
{
    PlayerControllerClass = ASS3DPlayerController::StaticClass();
}

void ASS3DGameMode::BeginPlay()
{
    Super::BeginPlay();
    MapManager = NewObject<UMapManager>(this);
    CombatManager = NewObject<UCombatManager>(this);
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
    MapManager->GenerateMap(RunSeed);
    Log(FString::Printf(TEXT("开始新的一局。角色：奥黛塔，生命：%d/%d，金币：%d。"), PlayerHp, PlayerMaxHp, Gold));
    PrintMap();
    PrintAvailableNodes();
}

void ASS3DGameMode::ExecuteCommand(const FString& CommandLine)
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
        else { SelectedCharacter = TEXT("odette"); Log(TEXT("已选择奥黛塔。输入 new [seed] 开始游戏。")); }
        return;
    }
    if (Command == TEXT("demo")) { RunDemo(); return; }
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

void ASS3DGameMode::PrintHelp() const
{
    Log(TEXT("命令：character odette | new [seed] | demo | status | map | select <nodeId> | hand | play <cardIndex> | end | potion <index> | reward <index> | shop | buy card/relic/potion/remove <index> | rest heal/upgrade | event 0/1"));
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
        Log(TEXT("本局失败。输入 new 开始新的一局。"));
        return;
    }

    Deck = CombatManager->GetDeck();
    Gold += CombatManager->GetGoldReward();
    Relics = CombatManager->GetRelics();
    bCombatActive = false;
    PendingRewards = FCardLibrary::GetAllCards();
    while (PendingRewards.Num() > 3) PendingRewards.RemoveAt(0);
    CompleteCurrentNode();
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
        if (MapManager->GetMapState().bRunComplete) Log(TEXT("抵达第三层塔顶，记忆尖塔通关。输入 new 开始下一局。"));
        else { PrintMap(); PrintAvailableNodes(); }
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
    const TArray<FPotionData> DemoPotions = FPotionLibrary::GetAllPotions();
    if (DemoPotions.Num() > 0) Potions.Add(DemoPotions[3]);
    for (int32 Step = 0; Step < 600 && !MapManager->GetMapState().bRunComplete; ++Step)
    {
        if (bCombatActive)
        {
            const FCombatSnapshot& Snapshot = CombatManager->GetSnapshot();
            if (Snapshot.Phase == ECombatPhase::PlayerTurn)
            {
                if (Snapshot.PlayerHp < 30 && Potions.Num() > 0)
                {
                    CombatManager->UsePotion(0);
                    HandleCombatResult();
                    continue;
                }
                int32 SelectedIndex = INDEX_NONE;
                if (Snapshot.PlayerHp < Snapshot.PlayerMaxHp * 0.6f && Snapshot.EnemyIntentDamage >= 10)
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
            SelectReward(0);
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
