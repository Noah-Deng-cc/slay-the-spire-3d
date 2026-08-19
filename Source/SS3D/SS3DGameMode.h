#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CardLibrary.h"
#include "EnemyLibrary.h"
#include "MapManager.h"
#include "CombatManager.h"
#include "PotionLibrary.h"
#include "RelicLibrary.h"
#include "SS3DGameMode.generated.h"

UCLASS()
class SS3D_API ASS3DGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ASS3DGameMode();
    virtual void BeginPlay() override;
    void ExecuteCommand(const FString& CommandLine);
    void PrintHelp() const;
    void RunDemo();

    UMapManager* GetMapManager() const { return MapManager; }

private:
    UPROPERTY()
    TObjectPtr<UMapManager> MapManager;

    UPROPERTY()
    TObjectPtr<UCombatManager> CombatManager;

    TArray<FCardData> Deck;
    TArray<FRelicData> Relics;
    TArray<FPotionData> Potions;
    TArray<FCardData> PendingRewards;
    TArray<FCardData> ShopCards;
    TArray<FRelicData> ShopRelics;
    TArray<FPotionData> ShopPotions;
    int32 PlayerHp = 80;
    int32 PlayerMaxHp = 80;
    int32 Gold = 99;
    int32 RunSeed = 1337;
    FString SelectedCharacter;
    bool bRunStarted = false;
    bool bCombatActive = false;

    void StartRun(int32 Seed);
    void PrintStatus() const;
    void PrintMap() const;
    void PrintHand() const;
    void PrintAvailableNodes() const;
    void SelectNode(int32 NodeId);
    void StartNodeCombat(const FMapNodeData& Node);
    void HandleCombatResult();
    void PrintRewards() const;
    void SelectReward(int32 Index);
    void PrintShop() const;
    void BuyShopItem(const FString& Args);
    void HandleRest(const FString& Args);
    void HandleEvent(const FString& Args);
    void CompleteCurrentNode();
    void GrantRelic(const FRelicData& Relic);
    void Log(const FString& Message) const;
    static FString NodeTypeName(EMapNodeType Type);
    static FString PhaseName(ECombatPhase Phase);
};
