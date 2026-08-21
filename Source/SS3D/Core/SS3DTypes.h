#pragma once

#include "CoreMinimal.h"
#include "SS3DTypes.generated.h"

UENUM(BlueprintType)
enum class ESS3DGamePhase : uint8
{
    Boot,
    CharacterSelect,
    Map,
    Combat,
    Reward,
    Shop,
    Event,
    Rest,
    Victory,
    Defeat
};

USTRUCT(BlueprintType)
struct SS3D_API FSS3DCheckpoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    ESS3DGamePhase Phase = ESS3DGamePhase::Boot;

    UPROPERTY(BlueprintReadOnly)
    int32 Sequence = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 RunSeed = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 ActIndex = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 NodeId = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly)
    FString Label;
};
