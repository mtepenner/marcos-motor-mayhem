#include "MMMGameMode.h"

#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "EngineUtils.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "MMMKartPawn.h"
#include "../World/MMMRaceArena.h"

AMMMGameMode::AMMMGameMode()
{
    DefaultPawnClass = AMMMKartPawn::StaticClass();
}

void AMMMGameMode::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    bool bHasArena = false;
    for (TActorIterator<AMMMRaceArena> It(World); It; ++It)
    {
        bHasArena = true;
        break;
    }

    if (!bHasArena)
    {
        World->SpawnActor<AMMMRaceArena>(FVector::ZeroVector, FRotator::ZeroRotator);
    }

    bool bHasDirectionalLight = false;
    for (TActorIterator<ADirectionalLight> It(World); It; ++It)
    {
        bHasDirectionalLight = true;
        break;
    }

    if (!bHasDirectionalLight)
    {
        ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(FVector(0.f, 0.f, 400.f), FRotator(-40.f, -35.f, 0.f));
        if (Sun && Sun->GetLightComponent())
        {
            Sun->GetLightComponent()->Intensity = 10.0f;
        }
    }

    bool bHasSkyLight = false;
    for (TActorIterator<ASkyLight> It(World); It; ++It)
    {
        bHasSkyLight = true;
        break;
    }

    if (!bHasSkyLight)
    {
        ASkyLight* Sky = World->SpawnActor<ASkyLight>(FVector(0.f, 0.f, 200.f), FRotator::ZeroRotator);
        if (Sky && Sky->GetLightComponent())
        {
            Sky->GetLightComponent()->Intensity = 2.0f;
        }
    }
}
