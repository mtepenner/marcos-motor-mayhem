#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MMMRaceArena.generated.h"

class UStaticMeshComponent;

UCLASS()
class MARCOSMOTORMAYHEM_API AMMMRaceArena : public AActor
{
    GENERATED_BODY()

public:
    AMMMRaceArena();

private:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* FloorMesh;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* CenterIslandMesh;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* StartGateMesh;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* CheckpointAMesh;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* CheckpointBMesh;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* CheckpointCMesh;
};
