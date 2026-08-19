#include "PotionLibrary.h"

TArray<FPotionData> FPotionLibrary::GetAllPotions()
{
    return {
        {TEXT("fire_potion"), FText::FromString(TEXT("火焰药水")), FText::FromString(TEXT("造成 20 点伤害。")), EPotionType::Damage, 20},
        {TEXT("block_potion"), FText::FromString(TEXT("护盾药水")), FText::FromString(TEXT("获得 12 点护盾。")), EPotionType::Block, 12},
        {TEXT("strength_potion"), FText::FromString(TEXT("力量药水")), FText::FromString(TEXT("本场战斗获得 2 点力量。")), EPotionType::Strength, 2},
        {TEXT("healing_potion"), FText::FromString(TEXT("治疗药水")), FText::FromString(TEXT("恢复 20 点生命。")), EPotionType::Heal, 20},
        {TEXT("energy_potion"), FText::FromString(TEXT("能量药水")), FText::FromString(TEXT("获得 2 点能量。")), EPotionType::Energy, 2}
    };
}

bool FPotionLibrary::TryGetPotion(const FString& Id, FPotionData& OutPotion)
{
    for (const FPotionData& Potion : GetAllPotions())
    {
        if (Potion.Id == Id)
        {
            OutPotion = Potion;
            return true;
        }
    }
    return false;
}
