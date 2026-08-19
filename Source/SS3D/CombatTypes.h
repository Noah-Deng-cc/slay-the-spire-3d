#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.generated.h"

UENUM(BlueprintType)
enum class ECardType : uint8
{
    Attack,
    Skill,
    Power
};

UENUM(BlueprintType)
enum class ECardRarity : uint8
{
    Common,
    Uncommon,
    Rare
};

UENUM(BlueprintType)
enum class ECardEffectType : uint8
{
    Damage,
    Block,
    Draw,
    Heal,
    Energy,
    Strength,
    Vulnerable,
    Weak,
    Poison
};

UENUM(BlueprintType)
enum class ECombatPhase : uint8
{
    PlayerTurn,
    EnemyTurn,
    Victory,
    Defeat
};

USTRUCT(BlueprintType)
struct FCardEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECardEffectType Type = ECardEffectType::Damage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Value = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bExhaust = false;
};

USTRUCT(BlueprintType)
struct FCardData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Cost = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECardType Type = ECardType::Attack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECardRarity Rarity = ECardRarity::Common;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FCardEffect> Effects;
};

UENUM(BlueprintType)
enum class ERelicTrigger : uint8
{
    None,
    CombatStart,
    TurnStart,
    CardPlayed,
    CombatVictory
};

USTRUCT(BlueprintType)
struct FRelicData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ERelicTrigger Trigger = ERelicTrigger::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Value = 0;
};

UENUM(BlueprintType)
enum class EPotionType : uint8
{
    Damage,
    Block,
    Heal,
    Energy,
    Strength
};

USTRUCT(BlueprintType)
struct FPotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPotionType Type = EPotionType::Damage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Value = 0;
};

UENUM(BlueprintType)
enum class EEnemyActionType : uint8
{
    Attack,
    Block,
    AttackAndBlock,
    Buff
};

USTRUCT(BlueprintType)
struct FEnemyAction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnemyActionType Type = EEnemyActionType::Attack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Value = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Label;
};

USTRUCT(BlueprintType)
struct FEnemyDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxHp = 40;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GoldReward = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FEnemyAction> Actions;
};

USTRUCT(BlueprintType)
struct FCombatSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    ECombatPhase Phase = ECombatPhase::PlayerTurn;

    UPROPERTY(BlueprintReadOnly)
    int32 Turn = 1;

    UPROPERTY(BlueprintReadOnly)
    int32 PlayerHp = 80;

    UPROPERTY(BlueprintReadOnly)
    int32 PlayerMaxHp = 80;

    UPROPERTY(BlueprintReadOnly)
    int32 PlayerBlock = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 Energy = 3;

    UPROPERTY(BlueprintReadOnly)
    int32 MaxEnergy = 3;

    UPROPERTY(BlueprintReadOnly)
    int32 EnemyHp = 50;

    UPROPERTY(BlueprintReadOnly)
    int32 EnemyMaxHp = 50;

    UPROPERTY(BlueprintReadOnly)
    int32 EnemyBlock = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 EnemyIntentDamage = 8;

    UPROPERTY(BlueprintReadOnly)
    FText EnemyIntentLabel;

    UPROPERTY(BlueprintReadOnly)
    TArray<FCardData> Hand;

    UPROPERTY(BlueprintReadOnly)
    int32 DrawPileCount = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 DiscardPileCount = 0;

    UPROPERTY(BlueprintReadOnly)
    FText LastAction;

    UPROPERTY(BlueprintReadOnly)
    FString EnemyId;

    UPROPERTY(BlueprintReadOnly)
    int32 PlayerStrength = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 EnemyVulnerable = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 EnemyWeak = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 EnemyPoison = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 Gold = 0;

    UPROPERTY(BlueprintReadOnly)
    TArray<FRelicData> Relics;

    UPROPERTY(BlueprintReadOnly)
    TArray<FPotionData> Potions;
};
