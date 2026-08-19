#include "EnemyLibrary.h"

namespace
{
    FEnemyAction Action(EEnemyActionType Type, int32 Value, const TCHAR* Label)
    {
        FEnemyAction Result;
        Result.Type = Type;
        Result.Value = Value;
        Result.Label = FText::FromString(Label);
        return Result;
    }

    FEnemyDefinition Enemy(const TCHAR* Id, const TCHAR* Name, int32 Hp, int32 Gold, TArray<FEnemyAction> Actions)
    {
        FEnemyDefinition Result;
        Result.Id = Id;
        Result.Name = FText::FromString(Name);
        Result.MaxHp = Hp;
        Result.GoldReward = Gold;
        Result.Actions = MoveTemp(Actions);
        return Result;
    }
}

TArray<FEnemyDefinition> FEnemyLibrary::GetActEnemies(int32 ActIndex, bool bElite)
{
    const int32 Scale = ActIndex * 12;
    if (bElite)
    {
        return {
            Enemy(TEXT("sentinel"), TEXT("记忆守卫"), 70 + Scale, 45 + ActIndex * 10,
                {Action(EEnemyActionType::Attack, 12 + ActIndex * 2, TEXT("重击")), Action(EEnemyActionType::Block, 10, TEXT("构筑护盾")), Action(EEnemyActionType::AttackAndBlock, 8 + ActIndex * 2, TEXT("冲撞"))}),
            Enemy(TEXT("collector"), TEXT("残响收集者"), 62 + Scale, 50 + ActIndex * 10,
                {Action(EEnemyActionType::Attack, 8 + ActIndex * 2, TEXT("抽取")), Action(EEnemyActionType::Buff, 2, TEXT("强化")), Action(EEnemyActionType::Attack, 16 + ActIndex * 2, TEXT("释放残响"))})
        };
    }

    return {
        Enemy(TEXT("echo"), TEXT("记忆回声"), 35 + Scale, 20 + ActIndex * 5,
            {Action(EEnemyActionType::Attack, 6 + ActIndex * 2, TEXT("回声冲击")), Action(EEnemyActionType::Block, 5, TEXT("回声凝聚"))}),
        Enemy(TEXT("shard"), TEXT("碎片兽"), 42 + Scale, 22 + ActIndex * 5,
            {Action(EEnemyActionType::Attack, 5 + ActIndex * 2, TEXT("撕裂")), Action(EEnemyActionType::Attack, 8 + ActIndex * 2, TEXT("突袭"))}),
        Enemy(TEXT("puppet_fragment"), TEXT("失控木偶碎片"), 30 + Scale, 24 + ActIndex * 5,
            {Action(EEnemyActionType::Block, 8 + ActIndex, TEXT("故障护盾")), Action(EEnemyActionType::Attack, 10 + ActIndex * 2, TEXT("机械挥击"))})
    };
}

FEnemyDefinition FEnemyLibrary::GetBoss(int32 ActIndex)
{
    if (ActIndex == 0)
    {
        return Enemy(TEXT("memory_keeper"), TEXT("记忆守门人"), 100, 100,
            {Action(EEnemyActionType::Attack, 10, TEXT("审视")), Action(EEnemyActionType::Block, 15, TEXT("封锁记忆")), Action(EEnemyActionType::AttackAndBlock, 14, TEXT("记忆重压"))});
    }
    if (ActIndex == 1)
    {
        return Enemy(TEXT("broken_clock"), TEXT("破碎时钟"), 135, 150,
            {Action(EEnemyActionType::Attack, 14, TEXT("时间冲击")), Action(EEnemyActionType::Buff, 2, TEXT("时间加速")), Action(EEnemyActionType::Attack, 20, TEXT("时针坠落"))});
    }
    return Enemy(TEXT("tower_core"), TEXT("尖塔核心"), 180, 250,
        {Action(EEnemyActionType::Attack, 18, TEXT("记忆湮灭")), Action(EEnemyActionType::Block, 20, TEXT("核心防御")), Action(EEnemyActionType::AttackAndBlock, 24, TEXT("空间崩塌"))});
}
