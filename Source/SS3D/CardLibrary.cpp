#include "CardLibrary.h"

FCardEffect FCardLibrary::Effect(ECardEffectType Type, int32 Value, bool bExhaust)
{
    FCardEffect Result;
    Result.Type = Type;
    Result.Value = Value;
    Result.bExhaust = bExhaust;
    return Result;
}

FCardData FCardLibrary::Card(const TCHAR* Id, const TCHAR* Name, int32 Cost, ECardType Type,
    ECardRarity Rarity, const TCHAR* Description, TArray<FCardEffect> Effects)
{
    FCardData Result;
    Result.Id = Id;
    Result.Name = FText::FromString(Name);
    Result.Cost = Cost;
    Result.Type = Type;
    Result.Rarity = Rarity;
    Result.Description = FText::FromString(Description);
    Result.Effects = MoveTemp(Effects);
    return Result;
}

TArray<FCardData> FCardLibrary::CreateStarterDeck()
{
    TArray<FCardData> Deck;
    for (int32 Index = 1; Index <= 5; ++Index)
    {
        Deck.Add(Card(*FString::Printf(TEXT("strike_%02d"), Index), TEXT("打击"), 1, ECardType::Attack,
            ECardRarity::Common, TEXT("造成 6 点伤害。"), {Effect(ECardEffectType::Damage, 6)}));
    }
    for (int32 Index = 1; Index <= 4; ++Index)
    {
        Deck.Add(Card(*FString::Printf(TEXT("defend_%02d"), Index), TEXT("防御"), 1, ECardType::Skill,
            ECardRarity::Common, TEXT("获得 5 点护盾。"), {Effect(ECardEffectType::Block, 5)}));
    }
    Deck.Add(Card(TEXT("bash"), TEXT("痛击"), 2, ECardType::Attack, ECardRarity::Common,
        TEXT("造成 8 点伤害，施加 2 层易伤。"), {Effect(ECardEffectType::Damage, 8), Effect(ECardEffectType::Vulnerable, 2)}));
    return Deck;
}

TArray<FCardData> FCardLibrary::GetAllCards()
{
    TArray<FCardData> Cards = CreateStarterDeck();
    Cards.Add(Card(TEXT("heavy_strike"), TEXT("重击"), 2, ECardType::Attack, ECardRarity::Common,
        TEXT("造成 14 点伤害。"), {Effect(ECardEffectType::Damage, 14)}));
    Cards.Add(Card(TEXT("pommel_strike"), TEXT("剑柄打击"), 1, ECardType::Attack, ECardRarity::Common,
        TEXT("造成 9 点伤害，抽 1 张牌。"), {Effect(ECardEffectType::Damage, 9), Effect(ECardEffectType::Draw, 1)}));
    Cards.Add(Card(TEXT("iron_wave"), TEXT("铁壁"), 1, ECardType::Attack, ECardRarity::Common,
        TEXT("造成 5 点伤害，获得 5 点护盾。"), {Effect(ECardEffectType::Damage, 5), Effect(ECardEffectType::Block, 5)}));
    Cards.Add(Card(TEXT("shrug_it_off"), TEXT("耸肩无视"), 1, ECardType::Skill, ECardRarity::Common,
        TEXT("获得 8 点护盾，抽 1 张牌。"), {Effect(ECardEffectType::Block, 8), Effect(ECardEffectType::Draw, 1)}));
    Cards.Add(Card(TEXT("true_grit"), TEXT("坚毅"), 1, ECardType::Skill, ECardRarity::Common,
        TEXT("获得 7 点护盾。消耗。"), {Effect(ECardEffectType::Block, 7, true)}));
    Cards.Add(Card(TEXT("anger"), TEXT("愤怒"), 0, ECardType::Attack, ECardRarity::Uncommon,
        TEXT("造成 6 点伤害。"), {Effect(ECardEffectType::Damage, 6)}));
    Cards.Add(Card(TEXT("twin_strike"), TEXT("双重打击"), 1, ECardType::Attack, ECardRarity::Uncommon,
        TEXT("造成 5 点伤害两次。"), {Effect(ECardEffectType::Damage, 5), Effect(ECardEffectType::Damage, 5)}));
    Cards.Add(Card(TEXT("inflame"), TEXT("燃烧"), 1, ECardType::Power, ECardRarity::Uncommon,
        TEXT("获得 2 点力量。"), {Effect(ECardEffectType::Strength, 2)}));
    Cards.Add(Card(TEXT("battle_trance"), TEXT("战斗专注"), 0, ECardType::Skill, ECardRarity::Uncommon,
        TEXT("抽 3 张牌。消耗。"), {Effect(ECardEffectType::Draw, 3, true)}));
    Cards.Add(Card(TEXT("bloodletting"), TEXT("放血"), 0, ECardType::Skill, ECardRarity::Uncommon,
        TEXT("失去 3 点生命，获得 2 点能量。"), {Effect(ECardEffectType::Heal, -3), Effect(ECardEffectType::Energy, 2)}));
    Cards.Add(Card(TEXT("impervious"), TEXT("不动如山"), 2, ECardType::Skill, ECardRarity::Rare,
        TEXT("获得 30 点护盾。消耗。"), {Effect(ECardEffectType::Block, 30, true)}));
    Cards.Add(Card(TEXT("limit_break"), TEXT("突破极限"), 1, ECardType::Skill, ECardRarity::Rare,
        TEXT("获得 3 点力量。"), {Effect(ECardEffectType::Strength, 3)}));
    Cards.Add(Card(TEXT("shockwave"), TEXT("震荡波"), 2, ECardType::Skill, ECardRarity::Uncommon,
        TEXT("施加 2 层易伤和 2 层虚弱。消耗。"), {Effect(ECardEffectType::Vulnerable, 2), Effect(ECardEffectType::Weak, 2, true)}));
    Cards.Add(Card(TEXT("memory_rot"), TEXT("记忆腐蚀"), 1, ECardType::Skill, ECardRarity::Uncommon,
        TEXT("施加 5 层中毒。"), {Effect(ECardEffectType::Poison, 5)}));
    Cards.Add(Card(TEXT("whirlwind"), TEXT("旋风斩"), 0, ECardType::Attack, ECardRarity::Rare,
        TEXT("消耗所有能量，每点造成 5 点伤害。"), {Effect(ECardEffectType::Damage, 5)}));
    return Cards;
}

bool FCardLibrary::TryGetCard(const FString& Id, FCardData& OutCard)
{
    for (const FCardData& CardData : GetAllCards())
    {
        if (CardData.Id == Id)
        {
            OutCard = CardData;
            return true;
        }
    }
    return false;
}
