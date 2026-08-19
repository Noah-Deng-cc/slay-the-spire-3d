#pragma once

#include "CoreMinimal.h"
#include "MapTypes.generated.h"

UENUM(BlueprintType)
enum class EMapNodeType : uint8
{
    Start,
    Combat,
    Reward,
    Shop,
    Event,
    Elite,
    Rest,
    Boss
};

USTRUCT(BlueprintType)
struct FMapNodeData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 NodeId = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly)
    int32 ActIndex = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 RowIndex = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 ColumnIndex = 0;

    UPROPERTY(BlueprintReadOnly)
    EMapNodeType NodeType = EMapNodeType::Combat;

    UPROPERTY(BlueprintReadOnly)
    TArray<int32> NextNodeIds;

    UPROPERTY(BlueprintReadOnly)
    bool bVisited = false;

    UPROPERTY(BlueprintReadOnly)
    bool bCompleted = false;
};

USTRUCT(BlueprintType)
struct FMapRunState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 Seed = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentAct = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentNodeId = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly)
    bool bRunComplete = false;

    UPROPERTY(BlueprintReadOnly)
    TArray<FMapNodeData> Nodes;
};
