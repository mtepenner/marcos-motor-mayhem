#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MMMGameMode.generated.h"

UCLASS()
class MARCOSMOTORMAYHEM_API AMMMGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMMMGameMode();

protected:
    virtual void BeginPlay() override;
};
