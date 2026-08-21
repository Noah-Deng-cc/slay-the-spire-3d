#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Core/SS3DTypes.h"
#include "SS3DGameState.generated.h"

UCLASS()
class SS3D_API ASS3DGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    void SetCheckpoint(ESS3DGamePhase InPhase, int32 InSequence, int32 InRunSeed,
        int32 InActIndex, int32 InNodeId, const FString& InLabel);

    UFUNCTION(BlueprintPure)
    ESS3DGamePhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure)
    FSS3DCheckpoint GetLastCheckpoint() const { return LastCheckpoint; }

    UFUNCTION(BlueprintPure)
    const TArray<FSS3DCheckpoint>& GetCheckpointHistory() const { return CheckpointHistory; }

    UPROPERTY(BlueprintReadOnly)
    ESS3DGamePhase CurrentPhase = ESS3DGamePhase::Boot;

    UPROPERTY(BlueprintReadOnly)
    FSS3DCheckpoint LastCheckpoint;

    UPROPERTY(BlueprintReadOnly)
    TArray<FSS3DCheckpoint> CheckpointHistory;
};
