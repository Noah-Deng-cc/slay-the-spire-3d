#pragma once

#include "CoreMinimal.h"
#include "MapTypes.h"
#include "UObject/Object.h"
#include "MapManager.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMapStateChanged, const FMapRunState&);

UCLASS(BlueprintType)
class SS3D_API UMapManager : public UObject
{
    GENERATED_BODY()

public:
    void GenerateMap(int32 Seed);

    const FMapRunState& GetMapState() const { return State; }
    TArray<FMapNodeData> GetAvailableNextNodes() const;
    bool IsNodeSelectable(int32 NodeId) const;
    bool SelectNode(int32 NodeId);
    bool CompleteCurrentNode();
    FMapNodeData GetCurrentNode() const;

    FOnMapStateChanged OnMapStateChanged;

private:
    FMapRunState State;

    FMapNodeData* FindNode(int32 NodeId);
    const FMapNodeData* FindNode(int32 NodeId) const;
    void BuildAct(int32 ActIndex, FRandomStream& Random);
    void ConnectRows(int32 ActIndex, const TArray<TArray<int32>>& Rows);
    static EMapNodeType RollNodeType(int32 ActIndex, int32 RowIndex, int32 ColumnIndex, FRandomStream& Random);
    void PublishState();
};
