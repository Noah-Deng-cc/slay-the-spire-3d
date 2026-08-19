#include "SS3DPlayerController.h"

#include "SS3DGameMode.h"

void ASS3DPlayerController::SS3D(const FString& Args)
{
    if (ASS3DGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ASS3DGameMode>() : nullptr)
    {
        GameMode->ExecuteCommand(Args);
    }
}
