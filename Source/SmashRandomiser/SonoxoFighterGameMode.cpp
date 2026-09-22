#include "SonoxoFighterGameMode.h"

#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SonoxoFighterCharacter.h"
#include "SonoxoFighterHUD.h"
#include "UObject/ConstructorHelpers.h"

ASonoxoFighterGameMode::ASonoxoFighterGameMode()
{
    DefaultPawnClass = ASonoxoFighterCharacter::StaticClass();
    HUDClass = ASonoxoFighterHUD::StaticClass();
}

void ASonoxoFighterGameMode::BeginPlay()
{
    Super::BeginPlay();

    BuildArena();
    ConfigureCamera();

    ASonoxoFighterCharacter* Player = Cast<ASonoxoFighterCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (Player)
    {
        Player->SetActorLocation(FVector(-280.0f, 0.0f, 160.0f));
    }

    SpawnOpponent();
}

void ASonoxoFighterGameMode::BuildArena()
{
    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!CubeMesh)
    {
        return;
    }

    auto SpawnPlatform = [&](const FVector& Location, const FVector& Scale)
    {
        AStaticMeshActor* Platform = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
        if (Platform)
        {
            Platform->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
            Platform->SetActorScale3D(Scale);
            Platform->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
        }
    };

    SpawnPlatform(FVector(0.0f, 0.0f, -85.0f), FVector(9.0f, 2.2f, 0.45f));
    SpawnPlatform(FVector(-330.0f, 0.0f, 150.0f), FVector(2.1f, 1.4f, 0.18f));
    SpawnPlatform(FVector(330.0f, 0.0f, 150.0f), FVector(2.1f, 1.4f, 0.18f));
    SpawnPlatform(FVector(0.0f, 0.0f, 285.0f), FVector(2.2f, 1.4f, 0.18f));
}

void ASonoxoFighterGameMode::ConfigureCamera()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        return;
    }

    const FVector CameraLocation(0.0f, -1650.0f, 330.0f);
    const FVector LookAt(0.0f, 0.0f, 135.0f);
    const FRotator CameraRotation = (LookAt - CameraLocation).Rotation();

    ACameraActor* Camera = GetWorld()->SpawnActor<ACameraActor>(CameraLocation, CameraRotation);
    if (Camera)
    {
        Camera->GetCameraComponent()->SetFieldOfView(52.0f);
        PC->SetViewTarget(Camera);
    }
}

void ASonoxoFighterGameMode::SpawnOpponent()
{
    ASonoxoFighterCharacter* Player = Cast<ASonoxoFighterCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ASonoxoFighterCharacter* Cpu = GetWorld()->SpawnActor<ASonoxoFighterCharacter>(
        ASonoxoFighterCharacter::StaticClass(),
        FVector(280.0f, 0.0f, 160.0f),
        FRotator::ZeroRotator,
        Params);

    if (!Cpu)
    {
        return;
    }

    UTexture2D* CpuTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Images/Characters/mewtwo.mewtwo"));
    if (CpuTexture)
    {
        Cpu->SetFighterTexture(CpuTexture);
    }

    Cpu->SetCpuControlled(true, Player);
}
