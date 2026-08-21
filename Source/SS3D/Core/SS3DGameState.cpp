#include "Core/SS3DGameState.h"

void ASS3DGameState::SetCheckpoint(ESS3DGamePhase InPhase, int32 InSequence, int32 InRunSeed,
    int32 InActIndex, int32 InNodeId, const FString& InLabel)
{
    CurrentPhase = InPhase;
    LastCheckpoint.Phase = InPhase;
    LastCheckpoint.Sequence = InSequence;
    LastCheckpoint.RunSeed = InRunSeed;
    LastCheckpoint.ActIndex = InActIndex;
    LastCheckpoint.NodeId = InNodeId;
    LastCheckpoint.Label = InLabel;
    CheckpointHistory.Add(LastCheckpoint);
}
