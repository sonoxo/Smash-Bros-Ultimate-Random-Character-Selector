#include "SonoxoFighterHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "SonoxoFighterCharacter.h"

void ASonoxoFighterHUD::DrawHUD()
{
    Super::DrawHUD();

    if (!Canvas || !GEngine)
    {
        return;
    }

    UFont* Font = GEngine->GetLargeFont();
    DrawText(TEXT("SONOXO // PLATFORM FIGHTER"), FLinearColor::White, 38.0f, 28.0f, Font, 1.1f, false);
    DrawText(TEXT("A/D move  |  SPACE jump  |  J light  |  K heavy"), FLinearColor::White, 40.0f, 72.0f, GEngine->GetSmallFont(), 1.0f, false);

    float Y = Canvas->ClipY - 125.0f;
    int32 Index = 0;

    for (TActorIterator<ASonoxoFighterCharacter> It(GetWorld()); It; ++It)
    {
        ASonoxoFighterCharacter* Fighter = *It;
        if (!Fighter)
        {
            continue;
        }

        const FString Label = FString::Printf(
            TEXT("%s  %.0f%%   STOCKS %d"),
            Fighter->IsCpuControlled() ? TEXT("CPU") : TEXT("P1"),
            Fighter->GetDamagePercent(),
            Fighter->GetStocks());

        const float X = Index == 0 ? 60.0f : Canvas->ClipX - 360.0f;
        DrawText(Label, FLinearColor::White, X, Y, Font, 1.2f, false);
        ++Index;
    }
}
