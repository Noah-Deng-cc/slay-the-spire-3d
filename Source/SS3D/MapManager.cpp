#include "MapManager.h"

namespace
{
    constexpr int32 ActCount = 3;
    constexpr int32 MiddleRows = 6;
}

void UMapManager::GenerateMap(int32 Seed)
{
    State = FMapRunState();
    State.Seed = Seed == 0 ? 1337 : Seed;

    FRandomStream Random(State.Seed);
    for (int32 ActIndex = 0; ActIndex < ActCount; ++ActIndex)
    {
        BuildAct(ActIndex, Random);
    }

    int32 FirstStartId = INDEX_NONE;
    for (const FMapNodeData& Node : State.Nodes)
    {
        if (Node.ActIndex == 0 && Node.NodeType == EMapNodeType::Start)
        {
            FirstStartId = Node.NodeId;
            break;
        }
    }
    State.CurrentNodeId = FirstStartId;
    if (FMapNodeData* Start = FindNode(FirstStartId)) Start->bVisited = true;
    PublishState();
}

void UMapManager::BuildAct(int32 ActIndex, FRandomStream& Random)
{
    TArray<TArray<int32>> Rows;
    TArray<int32> StartRow;
    FMapNodeData Start;
    Start.NodeId = State.Nodes.Num();
    Start.ActIndex = ActIndex;
    Start.RowIndex = 0;
    Start.ColumnIndex = 0;
    Start.NodeType = EMapNodeType::Start;
    State.Nodes.Add(Start);
    StartRow.Add(Start.NodeId);
    Rows.Add(StartRow);

    for (int32 RowIndex = 1; RowIndex <= MiddleRows; ++RowIndex)
    {
        TArray<int32> Row;
        const int32 ColumnCount = Random.RandRange(2, 3);
        for (int32 ColumnIndex = 0; ColumnIndex < ColumnCount; ++ColumnIndex)
        {
            FMapNodeData Node;
            Node.NodeId = State.Nodes.Num();
            Node.ActIndex = ActIndex;
            Node.RowIndex = RowIndex;
            Node.ColumnIndex = ColumnIndex;
            Node.NodeType = RollNodeType(ActIndex, RowIndex, ColumnIndex, Random);
            State.Nodes.Add(Node);
            Row.Add(Node.NodeId);
        }
        Rows.Add(Row);
    }

    TArray<int32> BossRow;
    FMapNodeData Boss;
    Boss.NodeId = State.Nodes.Num();
    Boss.ActIndex = ActIndex;
    Boss.RowIndex = MiddleRows + 1;
    Boss.ColumnIndex = 0;
    Boss.NodeType = EMapNodeType::Boss;
    State.Nodes.Add(Boss);
    BossRow.Add(Boss.NodeId);
    Rows.Add(BossRow);

    ConnectRows(ActIndex, Rows);
}

void UMapManager::ConnectRows(int32 ActIndex, const TArray<TArray<int32>>& Rows)
{
    for (int32 RowIndex = 0; RowIndex + 1 < Rows.Num(); ++RowIndex)
    {
        const TArray<int32>& CurrentRow = Rows[RowIndex];
        const TArray<int32>& NextRow = Rows[RowIndex + 1];
        TSet<int32> Incoming;

        for (int32 SourceIndex = 0; SourceIndex < CurrentRow.Num(); ++SourceIndex)
        {
            const int32 SourceId = CurrentRow[SourceIndex];
            FMapNodeData* Source = FindNode(SourceId);
            if (!Source) continue;

            const int32 PreferredTarget = FMath::Clamp(SourceIndex, 0, NextRow.Num() - 1);
            Source->NextNodeIds.AddUnique(NextRow[PreferredTarget]);
            Incoming.Add(NextRow[PreferredTarget]);
            if (NextRow.Num() > 1 && SourceIndex % 2 == 0)
            {
                Source->NextNodeIds.AddUnique(NextRow[FMath::Min(PreferredTarget + 1, NextRow.Num() - 1)]);
            }
        }

        for (int32 TargetIndex = 0; TargetIndex < NextRow.Num(); ++TargetIndex)
        {
            const int32 TargetId = NextRow[TargetIndex];
            if (Incoming.Contains(TargetId)) continue;
            const int32 SourceId = CurrentRow[TargetIndex % CurrentRow.Num()];
            if (FMapNodeData* Source = FindNode(SourceId)) Source->NextNodeIds.AddUnique(TargetId);
        }
    }

    if (ActIndex < ActCount - 1)
    {
        const int32 BossId = Rows.Last()[0];
        int32 NextStartId = INDEX_NONE;
        for (const FMapNodeData& Node : State.Nodes)
        {
            if (Node.ActIndex == ActIndex + 1 && Node.NodeType == EMapNodeType::Start)
            {
                NextStartId = Node.NodeId;
                break;
            }
        }
        if (NextStartId != INDEX_NONE)
        {
            if (FMapNodeData* Boss = FindNode(BossId)) Boss->NextNodeIds.AddUnique(NextStartId);
        }
    }
}

EMapNodeType UMapManager::RollNodeType(int32 ActIndex, int32 RowIndex, int32 ColumnIndex, FRandomStream& Random)
{
    if (RowIndex == MiddleRows) return ColumnIndex == 0 ? EMapNodeType::Elite : EMapNodeType::Combat;

    const int32 Roll = Random.RandRange(0, 99);
    if (Roll < 45) return EMapNodeType::Combat;
    if (Roll < 60) return EMapNodeType::Reward;
    if (Roll < 72) return EMapNodeType::Event;
    if (Roll < 83) return EMapNodeType::Rest;
    if (Roll < 92) return EMapNodeType::Shop;
    return EMapNodeType::Elite;
}

TArray<FMapNodeData> UMapManager::GetAvailableNextNodes() const
{
    TArray<FMapNodeData> Result;
    const FMapNodeData* Current = FindNode(State.CurrentNodeId);
    if (!Current || (!Current->bCompleted && Current->NodeType != EMapNodeType::Start)) return Result;

    for (int32 NodeId : Current->NextNodeIds)
    {
        const FMapNodeData* Next = FindNode(NodeId);
        if (Next && !Next->bVisited) Result.Add(*Next);
    }
    return Result;
}

bool UMapManager::IsNodeSelectable(int32 NodeId) const
{
    for (const FMapNodeData& Node : GetAvailableNextNodes())
    {
        if (Node.NodeId == NodeId) return true;
    }
    return false;
}

bool UMapManager::SelectNode(int32 NodeId)
{
    if (!IsNodeSelectable(NodeId)) return false;
    FMapNodeData* Node = FindNode(NodeId);
    if (!Node) return false;
    Node->bVisited = true;
    State.CurrentNodeId = NodeId;
    State.CurrentAct = Node->ActIndex;
    PublishState();
    return true;
}

bool UMapManager::CompleteCurrentNode()
{
    FMapNodeData* Current = FindNode(State.CurrentNodeId);
    if (!Current || Current->bCompleted) return false;
    Current->bCompleted = true;

    if (Current->NodeType == EMapNodeType::Boss)
    {
        if (Current->ActIndex == ActCount - 1)
        {
            State.bRunComplete = true;
        }
        else
        {
            for (FMapNodeData& Node : State.Nodes)
            {
                if (Node.ActIndex == Current->ActIndex + 1 && Node.NodeType == EMapNodeType::Start)
                {
                    Node.bVisited = true;
                    State.CurrentNodeId = Node.NodeId;
                    State.CurrentAct = Node.ActIndex;
                    break;
                }
            }
        }
    }
    PublishState();
    return true;
}

FMapNodeData UMapManager::GetCurrentNode() const
{
    const FMapNodeData* Current = FindNode(State.CurrentNodeId);
    return Current ? *Current : FMapNodeData();
}

FMapNodeData* UMapManager::FindNode(int32 NodeId)
{
    return State.Nodes.IsValidIndex(NodeId) ? &State.Nodes[NodeId] : nullptr;
}

const FMapNodeData* UMapManager::FindNode(int32 NodeId) const
{
    return State.Nodes.IsValidIndex(NodeId) ? &State.Nodes[NodeId] : nullptr;
}

void UMapManager::PublishState()
{
    OnMapStateChanged.Broadcast(State);
}
