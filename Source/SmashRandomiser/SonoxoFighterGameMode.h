#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SonoxoFighterGameMode.generated.h"

UCLASS()
class SMASHRANDOMISER_API ASonoxoFighterGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ASonoxoFighterGameMode();

protected:
    virtual void BeginPlay() override;

private:
    void BuildArena();
    void ConfigureCamera();
    void SpawnOpponent();
};
