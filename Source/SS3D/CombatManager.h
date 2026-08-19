#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.h"
#include "UObject/Object.h"
#include "CombatManager.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCombatStateChanged, const FCombatSnapshot&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCombatLogAdded, const FText&, int32);

UCLASS(BlueprintType)
class SS3D_API UCombatManager : public UObject
{
    GENERATED_BODY()

public:
    void BeginCombat(const FEnemyDefinition& InEnemy);
    void BeginCombat();
    void SetDeck(const TArray<FCardData>& InDeck);
    void SetRelics(const TArray<FRelicData>& InRelics);
    void SetPotions(const TArray<FPotionData>& InPotions);
    void SetPlayerHealth(int32 InHp, int32 InMaxHp);

    bool PlayCard(int32 HandIndex);
    bool UsePotion(int32 PotionIndex);
    void EndPlayerTurn();
    void RestartCombat();

    const FCombatSnapshot& GetSnapshot() const { return State; }
    const TArray<FCardData>& GetDeck() const { return Deck; }
    const TArray<FRelicData>& GetRelics() const { return Relics; }
    const TArray<FPotionData>& GetPotions() const { return Potions; }
    const FEnemyDefinition& GetEnemy() const { return Enemy; }
    int32 GetGoldReward() const { return Enemy.GoldReward; }

    FOnCombatStateChanged OnCombatStateChanged;
    FOnCombatLogAdded OnCombatLogAdded;

private:
    TArray<FCardData> Deck;
    TArray<FCardData> DrawPile;
    TArray<FCardData> DiscardPile;
    TArray<FCardData> ExhaustPile;
    TArray<FCardData> Hand;
    TArray<FRelicData> Relics;
    TArray<FPotionData> Potions;
    FEnemyDefinition Enemy;
    FCombatSnapshot State;
    int32 Strength = 0;
    int32 PersistentHp = 80;
    int32 PersistentMaxHp = 80;
    int32 EnemyActionIndex = 0;
    int32 EnemyBlock = 0;
    int32 EnemyVulnerable = 0;
    int32 EnemyWeak = 0;
    int32 EnemyStrength = 0;
    int32 EnemyPoison = 0;

    void DrawCards(int32 Amount);
    void PublishState();
    void AddLog(const FText& Message, int32 Tone);
    void ResolveEnemyTurn();
    void PreparePlayerTurn();
    void ApplyRelicTrigger(ERelicTrigger Trigger);
    void EnterVictory();
    void TakeDamage(int32 Amount);
    int32 GetEnemyAttackDamage(int32 BaseDamage) const;
    void UpdateEnemyIntent();
    static void ShuffleCards(TArray<FCardData>& Cards);
};
