#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SS3DWhiteboxStage.generated.h"

class UCameraComponent;

UCLASS()
class SS3D_API ASS3DWhiteboxStage : public AActor
{
    GENERATED_BODY()

public:
    ASS3DWhiteboxStage();

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> Floor;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> PlayerBody;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> PlayerHead;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> EnemyBody;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> EnemyHead;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UCameraComponent> Camera;

private:
    void ConfigureMesh(UStaticMeshComponent* Component, const TCHAR* AssetPath,
        const FVector& Location, const FVector& Scale, const FLinearColor& Color);
};
