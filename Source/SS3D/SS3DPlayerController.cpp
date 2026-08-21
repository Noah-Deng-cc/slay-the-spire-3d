#include "SS3DPlayerController.h"

#include "SS3DGameMode.h"
#include "InputCoreTypes.h"

void ASS3DPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (!InputComponent) return;

    InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ASS3DPlayerController::ToggleCursorMode);
    InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &ASS3DPlayerController::HandleConfirm);
    InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ASS3DPlayerController::HandleEndTurn);
    InputComponent->BindKey(EKeys::M, IE_Pressed, this, &ASS3DPlayerController::HandleMapShortcut);
    InputComponent->BindKey(EKeys::H, IE_Pressed, this, &ASS3DPlayerController::HandleHandShortcut);
}

void ASS3DPlayerController::SS3D(const FString& Args)
{
    if (ASS3DGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ASS3DGameMode>() : nullptr)
    {
        GameMode->ExecuteCommand(Args);
    }
}

void ASS3DPlayerController::ToggleCursorMode()
{
    bShowMouseCursor = !bShowMouseCursor;
    if (bShowMouseCursor)
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetHideCursorDuringCapture(false);
        SetInputMode(InputMode);
    }
    else
    {
        FInputModeGameOnly InputMode;
        SetInputMode(InputMode);
    }
}

void ASS3DPlayerController::HandleConfirm()
{
    if (ASS3DGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ASS3DGameMode>() : nullptr)
    {
        if (!GameMode->IsRunStarted())
        {
            GameMode->ExecuteCommand(TEXT("character odette"));
            GameMode->ExecuteCommand(TEXT("new 1337"));
        }
        else if (GameMode->IsCombatActive())
        {
            GameMode->ExecuteCommand(TEXT("end"));
        }
        else if (GameMode->GetPendingRewards().Num() > 0)
        {
            GameMode->ExecuteCommand(TEXT("reward 0"));
        }
        else
        {
            GameMode->ExecuteCommand(TEXT("map"));
        }
    }
}

void ASS3DPlayerController::HandleEndTurn()
{
    if (ASS3DGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ASS3DGameMode>() : nullptr)
    {
        if (GameMode->IsCombatActive()) GameMode->ExecuteCommand(TEXT("end"));
    }
}

void ASS3DPlayerController::HandleMapShortcut()
{
    if (ASS3DGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ASS3DGameMode>() : nullptr)
    {
        GameMode->ExecuteCommand(TEXT("map"));
    }
}

void ASS3DPlayerController::HandleHandShortcut()
{
    if (ASS3DGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ASS3DGameMode>() : nullptr)
    {
        if (GameMode->IsCombatActive()) GameMode->ExecuteCommand(TEXT("hand"));
    }
}
