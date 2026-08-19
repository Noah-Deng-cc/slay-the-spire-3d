#include "CombatManager.h"

#include "CardLibrary.h"
#include "Algo/RandomShuffle.h"

void UCombatManager::SetDeck(const TArray<FCardData>& InDeck)
{
    Deck = InDeck;
}

void UCombatManager::SetRelics(const TArray<FRelicData>& InRelics)
{
    Relics = InRelics;
}

void UCombatManager::SetPotions(const TArray<FPotionData>& InPotions)
{
    Potions = InPotions;
}

void UCombatManager::SetPlayerHealth(int32 InHp, int32 InMaxHp)
{
    PersistentMaxHp = FMath::Max(1, InMaxHp);
    PersistentHp = FMath::Clamp(InHp, 0, PersistentMaxHp);
}

void UCombatManager::BeginCombat()
{
    FEnemyDefinition DefaultEnemy;
    DefaultEnemy.Id = TEXT("training_echo");
    DefaultEnemy.Name = FText::FromString(TEXT("训练回声"));
    DefaultEnemy.MaxHp = 50;
    DefaultEnemy.GoldReward = 20;
    FEnemyAction Action;
    Action.Type = EEnemyActionType::Attack;
    Action.Value = 8;
    Action.Label = FText::FromString(TEXT("攻击"));
    DefaultEnemy.Actions.Add(Action);
    BeginCombat(DefaultEnemy);
}

void UCombatManager::BeginCombat(const FEnemyDefinition& InEnemy)
{
    if (Deck.Num() == 0) Deck = FCardLibrary::CreateStarterDeck();
    Enemy = InEnemy;
    DrawPile = Deck;
    DiscardPile.Reset();
    ExhaustPile.Reset();
    Hand.Reset();
    ShuffleCards(DrawPile);
    Strength = 0;
    EnemyBlock = 0;
    EnemyActionIndex = 0;
    EnemyVulnerable = 0;
    EnemyWeak = 0;
    EnemyStrength = 0;
    EnemyPoison = 0;
    State = FCombatSnapshot();
    State.Phase = ECombatPhase::PlayerTurn;
    State.Turn = 1;
    State.PlayerHp = PersistentHp;
    State.PlayerMaxHp = PersistentMaxHp;
    State.Energy = 3;
    State.MaxEnergy = 3;
    State.EnemyId = Enemy.Id;
    State.EnemyHp = Enemy.MaxHp;
    State.EnemyMaxHp = Enemy.MaxHp;
    State.Gold = Enemy.GoldReward;
    State.Relics = Relics;
    State.Potions = Potions;
    State.LastAction = FText::FromString(TEXT("战斗开始"));
    UpdateEnemyIntent();
    ApplyRelicTrigger(ERelicTrigger::CombatStart);
    DrawCards(5);
    AddLog(FText::Format(NSLOCTEXT("Combat", "Begin", "遭遇 {0}，战斗开始。"), Enemy.Name), 1);
    PublishState();
}

bool UCombatManager::PlayCard(int32 HandIndex)
{
    if (State.Phase != ECombatPhase::PlayerTurn || !Hand.IsValidIndex(HandIndex)) return false;
    const FCardData Card = Hand[HandIndex];
    if (Card.Cost > State.Energy)
    {
        AddLog(FText::Format(NSLOCTEXT("Combat", "NotEnoughEnergy", "能量不足，无法使用「{0}」。"), Card.Name), 2);
        return false;
    }

    State.Energy -= Card.Cost;
    Hand.RemoveAt(HandIndex);
    bool bExhaust = false;
    for (const FCardEffect& Effect : Card.Effects)
    {
        switch (Effect.Type)
        {
        case ECardEffectType::Damage:
        {
            int32 Damage = Effect.Value + Strength;
            if (EnemyVulnerable > 0) Damage = FMath::CeilToInt(Damage * 1.5f);
            const int32 Blocked = FMath::Min(EnemyBlock, Damage);
            EnemyBlock -= Blocked;
            Damage -= Blocked;
            State.EnemyHp = FMath::Max(0, State.EnemyHp - Damage);
            AddLog(FText::Format(NSLOCTEXT("Combat", "DealDamage", "「{0}」造成 {1} 点伤害。"), Card.Name, Damage), 1);
            break;
        }
        case ECardEffectType::Block:
            State.PlayerBlock += Effect.Value;
            AddLog(FText::Format(NSLOCTEXT("Combat", "GainBlock", "「{0}」获得 {1} 点护盾。"), Card.Name, Effect.Value), 1);
            break;
        case ECardEffectType::Draw:
            DrawCards(Effect.Value);
            break;
        case ECardEffectType::Heal:
            State.PlayerHp = FMath::Clamp(State.PlayerHp + Effect.Value, 0, State.PlayerMaxHp);
            break;
        case ECardEffectType::Energy:
            State.Energy = FMath::Min(State.MaxEnergy, State.Energy + Effect.Value);
            break;
        case ECardEffectType::Strength:
            Strength += Effect.Value;
            break;
        case ECardEffectType::Vulnerable:
            EnemyVulnerable += Effect.Value;
            break;
        case ECardEffectType::Weak:
            EnemyWeak += Effect.Value;
            break;
        case ECardEffectType::Poison:
            EnemyPoison += Effect.Value;
            break;
        default:
            break;
        }
        bExhaust |= Effect.bExhaust;
    }

    if (bExhaust) ExhaustPile.Add(Card);
    else DiscardPile.Add(Card);
    State.LastAction = FText::Format(NSLOCTEXT("Combat", "PlayCard", "使用 {0}"), Card.Name);
    ApplyRelicTrigger(ERelicTrigger::CardPlayed);
    if (State.EnemyHp <= 0)
    {
        EnterVictory();
    }
    PublishState();
    return true;
}

bool UCombatManager::UsePotion(int32 PotionIndex)
{
    if (State.Phase != ECombatPhase::PlayerTurn || !Potions.IsValidIndex(PotionIndex)) return false;
    const FPotionData Potion = Potions[PotionIndex];
    switch (Potion.Type)
    {
    case EPotionType::Damage:
        State.EnemyHp = FMath::Max(0, State.EnemyHp - Potion.Value);
        break;
    case EPotionType::Block:
        State.PlayerBlock += Potion.Value;
        break;
    case EPotionType::Heal:
        State.PlayerHp = FMath::Min(State.PlayerMaxHp, State.PlayerHp + Potion.Value);
        break;
    case EPotionType::Energy:
        State.Energy = FMath::Min(State.MaxEnergy, State.Energy + Potion.Value);
        break;
    case EPotionType::Strength:
        Strength += Potion.Value;
        break;
    default:
        break;
    }
    Potions.RemoveAt(PotionIndex);
    State.Potions = Potions;
    AddLog(FText::Format(NSLOCTEXT("Combat", "UsePotion", "使用药水：{0}。"), Potion.Name), 1);
    if (State.EnemyHp <= 0) EnterVictory();
    PublishState();
    return true;
}

void UCombatManager::EndPlayerTurn()
{
    if (State.Phase != ECombatPhase::PlayerTurn) return;
    DiscardPile.Append(Hand);
    Hand.Reset();
    State.Phase = ECombatPhase::EnemyTurn;
    PublishState();
    ResolveEnemyTurn();
}

void UCombatManager::RestartCombat()
{
    BeginCombat(Enemy);
}

void UCombatManager::ResolveEnemyTurn()
{
    if (EnemyPoison > 0)
    {
        State.EnemyHp = FMath::Max(0, State.EnemyHp - EnemyPoison);
        AddLog(FText::Format(NSLOCTEXT("Combat", "Poison", "中毒造成 {0} 点伤害。"), EnemyPoison), 1);
        EnemyPoison = FMath::Max(0, EnemyPoison - 1);
    }
    if (State.EnemyHp <= 0)
    {
        EnterVictory();
        PublishState();
        return;
    }

    const FEnemyAction Action = Enemy.Actions.Num() > 0 ? Enemy.Actions[EnemyActionIndex % Enemy.Actions.Num()] : FEnemyAction();
    ++EnemyActionIndex;
    switch (Action.Type)
    {
    case EEnemyActionType::Attack:
        TakeDamage(GetEnemyAttackDamage(Action.Value));
        break;
    case EEnemyActionType::Block:
        EnemyBlock += Action.Value;
        AddLog(FText::Format(NSLOCTEXT("Combat", "EnemyBlock", "{0} 获得 {1} 点护盾。"), Enemy.Name, Action.Value), 1);
        break;
    case EEnemyActionType::AttackAndBlock:
        EnemyBlock += Action.Value / 2;
        TakeDamage(GetEnemyAttackDamage(Action.Value));
        break;
    case EEnemyActionType::Buff:
        EnemyStrength += Action.Value;
        AddLog(FText::Format(NSLOCTEXT("Combat", "EnemyBuff", "{0} 使用 {1}。"), Enemy.Name, Action.Label), 1);
        break;
    default:
        break;
    }

    EnemyVulnerable = FMath::Max(0, EnemyVulnerable - 1);
    EnemyWeak = FMath::Max(0, EnemyWeak - 1);
    if (State.Phase == ECombatPhase::Defeat)
    {
        PublishState();
        return;
    }
    UpdateEnemyIntent();
    PreparePlayerTurn();
}

void UCombatManager::PreparePlayerTurn()
{
    ++State.Turn;
    State.Phase = ECombatPhase::PlayerTurn;
    State.Energy = State.MaxEnergy;
    State.PlayerBlock = 0;
    ApplyRelicTrigger(ERelicTrigger::TurnStart);
    DrawCards(5);
    State.LastAction = FText::FromString(TEXT("轮到你行动"));
    PublishState();
}

void UCombatManager::ApplyRelicTrigger(ERelicTrigger Trigger)
{
    for (const FRelicData& Relic : Relics)
    {
        if (Relic.Trigger != Trigger) continue;
        if (Trigger == ERelicTrigger::CombatStart && Relic.Id == TEXT("iron_ring")) State.PlayerBlock += Relic.Value;
        if (Trigger == ERelicTrigger::CombatStart && Relic.Id == TEXT("lantern")) State.Energy += Relic.Value;
        if (Trigger == ERelicTrigger::CombatVictory && Relic.Id == TEXT("burning_blood")) State.PlayerHp = FMath::Min(State.PlayerMaxHp, State.PlayerHp + Relic.Value);
        if (Trigger == ERelicTrigger::TurnStart && Relic.Id == TEXT("lantern") && State.Turn == 1) State.Energy += Relic.Value;
        if (Trigger == ERelicTrigger::CardPlayed && Relic.Id == TEXT("shuriken")) Strength += Relic.Value;
    }
}

void UCombatManager::TakeDamage(int32 Amount)
{
    const int32 Blocked = FMath::Min(State.PlayerBlock, Amount);
    State.PlayerBlock -= Blocked;
    const int32 Damage = Amount - Blocked;
    State.PlayerHp = FMath::Max(0, State.PlayerHp - Damage);
    AddLog(FText::Format(NSLOCTEXT("Combat", "EnemyAttack", "敌人攻击，护盾抵消 {0}，受到 {1} 点伤害。"), Blocked, Damage), Damage > 0 ? 2 : 1);
    if (State.PlayerHp <= 0)
    {
        State.Phase = ECombatPhase::Defeat;
        AddLog(FText::FromString(TEXT("生命值归零，本局失败。")), 2);
    }
}

int32 UCombatManager::GetEnemyAttackDamage(int32 BaseDamage) const
{
    int32 Damage = FMath::Max(0, BaseDamage + EnemyStrength);
    if (EnemyWeak > 0)
    {
        Damage = FMath::FloorToInt(Damage * 0.75f);
    }
    return Damage;
}

void UCombatManager::UpdateEnemyIntent()
{
    if (Enemy.Actions.Num() == 0)
    {
        State.EnemyIntentDamage = 0;
        State.EnemyIntentLabel = FText::FromString(TEXT("无行动"));
        return;
    }
    const FEnemyAction& Action = Enemy.Actions[EnemyActionIndex % Enemy.Actions.Num()];
    State.EnemyIntentDamage = Action.Type == EEnemyActionType::Block ? 0 : GetEnemyAttackDamage(Action.Value);
    State.EnemyIntentLabel = Action.Label;
}

void UCombatManager::DrawCards(int32 Amount)
{
    for (int32 Index = 0; Index < Amount; ++Index)
    {
        if (DrawPile.Num() == 0)
        {
            if (DiscardPile.Num() == 0) return;
            DrawPile.Append(DiscardPile);
            DiscardPile.Reset();
            ShuffleCards(DrawPile);
        }
        Hand.Add(DrawPile.Pop());
    }
}

void UCombatManager::PublishState()
{
    State.Hand = Hand;
    State.DrawPileCount = DrawPile.Num();
    State.DiscardPileCount = DiscardPile.Num();
    State.PlayerStrength = Strength;
    State.EnemyBlock = EnemyBlock;
    State.EnemyVulnerable = EnemyVulnerable;
    State.EnemyWeak = EnemyWeak;
    State.EnemyStrength = EnemyStrength;
    State.EnemyPoison = EnemyPoison;
    State.Relics = Relics;
    State.Potions = Potions;
    OnCombatStateChanged.Broadcast(State);
}

void UCombatManager::EnterVictory()
{
    if (State.Phase == ECombatPhase::Victory) return;
    State.Phase = ECombatPhase::Victory;
    ApplyRelicTrigger(ERelicTrigger::CombatVictory);
    AddLog(FText::Format(NSLOCTEXT("Combat", "Victory", "击败 {0}。"), Enemy.Name), 1);
}

void UCombatManager::AddLog(const FText& Message, int32 Tone)
{
    State.LastAction = Message;
    OnCombatLogAdded.Broadcast(Message, Tone);
    UE_LOG(LogTemp, Display, TEXT("[SS3D] %s"), *Message.ToString());
}

void UCombatManager::ShuffleCards(TArray<FCardData>& Cards)
{
    Algo::RandomShuffle(Cards);
}
