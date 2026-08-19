#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SS3DPlayerController.generated.h"

UCLASS()
class SS3D_API ASS3DPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    UFUNCTION(Exec)
    void SS3D(const FString& Args);
};
