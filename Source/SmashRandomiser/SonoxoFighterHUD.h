#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SonoxoFighterHUD.generated.h"

UCLASS()
class SMASHRANDOMISER_API ASonoxoFighterHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;
};
