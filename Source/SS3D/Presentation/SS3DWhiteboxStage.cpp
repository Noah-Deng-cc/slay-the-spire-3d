#include "Presentation/SS3DWhiteboxStage.h"

#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    UMaterialInterface* LoadBasicMaterial()
    {
        static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(
            TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
        return MaterialFinder.Succeeded() ? MaterialFinder.Object : nullptr;
    }
}

ASS3DWhiteboxStage::ASS3DWhiteboxStage()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    Floor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Floor"));
    Floor->SetupAttachment(SceneRoot);
    ConfigureMesh(Floor, TEXT("/Engine/BasicShapes/Cube"), FVector(0.0f, 0.0f, -55.0f),
        FVector(8.0f, 6.0f, 0.1f), FLinearColor(0.08f, 0.10f, 0.13f));

    PlayerBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerBody"));
    PlayerBody->SetupAttachment(SceneRoot);
    ConfigureMesh(PlayerBody, TEXT("/Engine/BasicShapes/Cube"), FVector(-180.0f, 0.0f, 80.0f),
        FVector(0.55f, 0.35f, 1.1f), FLinearColor(0.15f, 0.55f, 0.95f));

    PlayerHead = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerHead"));
    PlayerHead->SetupAttachment(SceneRoot);
    ConfigureMesh(PlayerHead, TEXT("/Engine/BasicShapes/Sphere"), FVector(-180.0f, 0.0f, 205.0f),
        FVector(0.45f, 0.45f, 0.45f), FLinearColor(0.25f, 0.70f, 1.0f));

    EnemyBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyBody"));
    EnemyBody->SetupAttachment(SceneRoot);
    ConfigureMesh(EnemyBody, TEXT("/Engine/BasicShapes/Cube"), FVector(180.0f, 0.0f, 80.0f),
        FVector(0.65f, 0.45f, 1.25f), FLinearColor(0.80f, 0.20f, 0.16f));

    EnemyHead = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyHead"));
    EnemyHead->SetupAttachment(SceneRoot);
    ConfigureMesh(EnemyHead, TEXT("/Engine/BasicShapes/Sphere"), FVector(180.0f, 0.0f, 220.0f),
        FVector(0.52f, 0.52f, 0.52f), FLinearColor(1.0f, 0.30f, 0.20f));

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SceneRoot);
    Camera->SetRelativeLocation(FVector(0.0f, -900.0f, 520.0f));
    Camera->SetRelativeRotation(FRotationMatrix::MakeFromX(FVector(0.0f, 1.0f, -0.52f)).Rotator());
    Camera->FieldOfView = 45.0f;
}

void ASS3DWhiteboxStage::ConfigureMesh(UStaticMeshComponent* Component, const TCHAR* AssetPath,
    const FVector& Location, const FVector& Scale, const FLinearColor& Color)
{
    if (!Component) return;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere"));
    UStaticMesh* Mesh = FCString::Stristr(AssetPath, TEXT("Sphere")) ? SphereFinder.Object : CubeFinder.Object;
    Component->SetStaticMesh(Mesh);
    Component->SetRelativeLocation(Location);
    Component->SetRelativeScale3D(Scale);
    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (UMaterialInterface* Material = LoadBasicMaterial())
    {
        Component->SetMaterial(0, Material);
    }
    Component->ComponentTags.Add(FName(*FString::Printf(TEXT("SS3D_Whitebox_%s"), *Color.ToString())));
}
