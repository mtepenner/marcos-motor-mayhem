#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MMMKartPawn.generated.h"

class UCameraComponent;
class UFloatingPawnMovement;
class USpringArmComponent;
class UStaticMeshComponent;

UCLASS()
class MARCOSMOTORMAYHEM_API AMMMKartPawn : public APawn
{
    GENERATED_BODY()

public:
    AMMMKartPawn();

    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
    virtual void BeginPlay() override;

private:
    void SetThrottle(float Value);
    void SetSteer(float Value);
    void ResetKart();

    UPROPERTY(VisibleAnywhere, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* ChassisMesh;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UCameraComponent* FollowCamera;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UFloatingPawnMovement* MovementComponent;

    UPROPERTY(EditAnywhere, Category = "Driving")
    float TurnRateDegrees = 110.0f;

    UPROPERTY(EditAnywhere, Category = "Driving")
    FVector SpawnLocation = FVector(0.0f, 0.0f, 120.0f);

    float ThrottleInput = 0.0f;
    float SteeringInput = 0.0f;
};
