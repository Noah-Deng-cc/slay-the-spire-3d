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

DECLARE_MULTICAST_DELEGATE(FOnSS3DGameStateChanged);

class USBattleHUD;

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
    void RunEffectsRegression();
    void RunNodeRegression();

    UMapManager* GetMapManager() const { return MapManager; }
    const FMapRunState& GetMapState() const { return MapManager->GetMapState(); }
    const FCombatSnapshot& GetCombatSnapshot() const { return CombatManager->GetSnapshot(); }
    const TArray<FCardData>& GetDeck() const { return Deck; }
    const TArray<FCardData>& GetPendingRewards() const { return PendingRewards; }
    const TArray<FCardData>& GetShopCards() const { return ShopCards; }
    const TArray<FRelicData>& GetShopRelics() const { return ShopRelics; }
    const TArray<FPotionData>& GetShopPotions() const { return ShopPotions; }
    const TArray<FRelicData>& GetRelics() const { return Relics; }
    const TArray<FPotionData>& GetPotions() const { return Potions; }
    int32 GetPlayerHp() const { return PlayerHp; }
    int32 GetPlayerMaxHp() const { return PlayerMaxHp; }
    int32 GetGold() const { return Gold; }
    bool IsRunStarted() const { return bRunStarted; }
    bool IsCombatActive() const { return bCombatActive; }

    FOnSS3DGameStateChanged OnStateChanged;

private:
    UPROPERTY()
    TObjectPtr<UMapManager> MapManager;

    UPROPERTY()
    TObjectPtr<UCombatManager> CombatManager;

    UPROPERTY()
    TObjectPtr<USBattleHUD> RuntimeHUD;

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

    void ExecuteCommandInternal(const FString& CommandLine);
    void NotifyStateChanged();
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
