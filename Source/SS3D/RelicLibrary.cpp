#include "RelicLibrary.h"

TArray<FRelicData> FRelicLibrary::GetAllRelics()
{
    TArray<FRelicData> Relics;
    Relics.Add({TEXT("burning_blood"), FText::FromString(TEXT("燃烧之血")), FText::FromString(TEXT("战斗胜利后恢复 6 点生命。")), ERelicTrigger::CombatVictory, 6});
    Relics.Add({TEXT("lantern"), FText::FromString(TEXT("灯笼")), FText::FromString(TEXT("每场战斗第一回合获得 1 点额外能量。")), ERelicTrigger::CombatStart, 1});
    Relics.Add({TEXT("iron_ring"), FText::FromString(TEXT("铁环")), FText::FromString(TEXT("每场战斗开始获得 2 点护盾。")), ERelicTrigger::CombatStart, 2});
    Relics.Add({TEXT("ancient_coin"), FText::FromString(TEXT("古钱")), FText::FromString(TEXT("获得时增加 100 金币。")), ERelicTrigger::None, 100});
    Relics.Add({TEXT("shuriken"), FText::FromString(TEXT("手里剑")), FText::FromString(TEXT("每回合打出 3 张攻击牌后获得 1 点力量。")), ERelicTrigger::CardPlayed, 1});
    return Relics;
}

bool FRelicLibrary::TryGetRelic(const FString& Id, FRelicData& OutRelic)
{
    for (const FRelicData& Relic : GetAllRelics())
    {
        if (Relic.Id == Id)
        {
            OutRelic = Relic;
            return true;
        }
    }
    return false;
}
