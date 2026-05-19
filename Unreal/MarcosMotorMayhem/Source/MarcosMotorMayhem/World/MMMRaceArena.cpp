#include "World/MMMRaceArena.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AMMMRaceArena::AMMMRaceArena()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
    FloorMesh->SetupAttachment(SceneRoot);
    FloorMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -60.0f));
    FloorMesh->SetRelativeScale3D(FVector(30.0f, 30.0f, 1.0f));

    CenterIslandMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CenterIslandMesh"));
    CenterIslandMesh->SetupAttachment(SceneRoot);
    CenterIslandMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -10.0f));
    CenterIslandMesh->SetRelativeScale3D(FVector(7.0f, 7.0f, 1.0f));

    StartGateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StartGateMesh"));
    StartGateMesh->SetupAttachment(SceneRoot);
    StartGateMesh->SetRelativeLocation(FVector(0.0f, -900.0f, 80.0f));
    StartGateMesh->SetRelativeScale3D(FVector(1.0f, 0.3f, 2.2f));

    CheckpointAMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CheckpointAMesh"));
    CheckpointAMesh->SetupAttachment(SceneRoot);
    CheckpointAMesh->SetRelativeLocation(FVector(1200.0f, 0.0f, 80.0f));
    CheckpointAMesh->SetRelativeScale3D(FVector(0.4f, 2.0f, 2.0f));

    CheckpointBMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CheckpointBMesh"));
    CheckpointBMesh->SetupAttachment(SceneRoot);
    CheckpointBMesh->SetRelativeLocation(FVector(0.0f, 1100.0f, 80.0f));
    CheckpointBMesh->SetRelativeScale3D(FVector(2.0f, 0.4f, 2.0f));

    CheckpointCMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CheckpointCMesh"));
    CheckpointCMesh->SetupAttachment(SceneRoot);
    CheckpointCMesh->SetRelativeLocation(FVector(-1200.0f, 0.0f, 80.0f));
    CheckpointCMesh->SetRelativeScale3D(FVector(0.4f, 2.0f, 2.0f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        FloorMesh->SetStaticMesh(CubeMesh.Object);
        CenterIslandMesh->SetStaticMesh(CubeMesh.Object);
        StartGateMesh->SetStaticMesh(CubeMesh.Object);
        CheckpointAMesh->SetStaticMesh(CubeMesh.Object);
        CheckpointBMesh->SetStaticMesh(CubeMesh.Object);
        CheckpointCMesh->SetStaticMesh(CubeMesh.Object);
    }
}
