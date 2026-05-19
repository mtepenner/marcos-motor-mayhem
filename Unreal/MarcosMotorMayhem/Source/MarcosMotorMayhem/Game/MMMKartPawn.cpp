#include "Game/MMMKartPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "UObject/ConstructorHelpers.h"

AMMMKartPawn::AMMMKartPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    ChassisMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChassisMesh"));
    ChassisMesh->SetupAttachment(SceneRoot);
    ChassisMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 45.0f));
    ChassisMesh->SetRelativeScale3D(FVector(1.8f, 1.0f, 0.6f));
    ChassisMesh->SetCollisionProfileName(TEXT("Pawn"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        ChassisMesh->SetStaticMesh(CubeMesh.Object);
    }

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(SceneRoot);
    CameraBoom->TargetArmLength = 700.0f;
    CameraBoom->SetRelativeRotation(FRotator(-30.0f, 0.0f, 0.0f));
    CameraBoom->bDoCollisionTest = false;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
    MovementComponent->MaxSpeed = 2200.0f;
    MovementComponent->Acceleration = 4500.0f;
    MovementComponent->Deceleration = 3800.0f;
    MovementComponent->TurningBoost = 8.0f;

    AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AMMMKartPawn::BeginPlay()
{
    Super::BeginPlay();
    ResetKart();
}

void AMMMKartPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!FMath::IsNearlyZero(SteeringInput))
    {
        AddActorLocalRotation(FRotator(0.0f, SteeringInput * TurnRateDegrees * DeltaSeconds, 0.0f));
    }

    if (!FMath::IsNearlyZero(ThrottleInput))
    {
        AddMovementInput(GetActorForwardVector(), ThrottleInput);
    }
}

void AMMMKartPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    check(PlayerInputComponent);
    PlayerInputComponent->BindAxis(TEXT("Throttle"), this, &AMMMKartPawn::SetThrottle);
    PlayerInputComponent->BindAxis(TEXT("Steer"), this, &AMMMKartPawn::SetSteer);
    PlayerInputComponent->BindAction(TEXT("ResetKart"), IE_Pressed, this, &AMMMKartPawn::ResetKart);
}

void AMMMKartPawn::SetThrottle(float Value)
{
    ThrottleInput = Value;
}

void AMMMKartPawn::SetSteer(float Value)
{
    SteeringInput = Value;
}

void AMMMKartPawn::ResetKart()
{
    SetActorLocation(SpawnLocation);
    SetActorRotation(FRotator::ZeroRotator);

    if (MovementComponent)
    {
        MovementComponent->StopMovementImmediately();
    }
}
