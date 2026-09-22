#include "SonoxoFighterCharacter.h"

#include "Components/BillboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Texture2D.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ASonoxoFighterCharacter::ASonoxoFighterCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->InitCapsuleSize(42.0f, 88.0f);

    UCharacterMovementComponent* Move = GetCharacterMovement();
    Move->GravityScale = 2.15f;
    Move->JumpZVelocity = 790.0f;
    Move->AirControl = 0.82f;
    Move->MaxWalkSpeed = 720.0f;
    Move->BrakingDecelerationWalking = 2200.0f;
    Move->bConstrainToPlane = true;
    Move->SetPlaneConstraintNormal(FVector(0.0f, 1.0f, 0.0f));
    Move->bSnapToPlaneAtStart = true;

    FighterBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("FighterBillboard"));
    FighterBillboard->SetupAttachment(GetRootComponent());
    FighterBillboard->SetRelativeLocation(FVector(0.0f, 0.0f, 12.0f));
    FighterBillboard->SetRelativeScale3D(FVector(0.42f));

    static ConstructorHelpers::FObjectFinder<UTexture2D> DefaultTexture(TEXT("/Game/Images/Characters/pikachu.pikachu"));
    if (DefaultTexture.Succeeded())
    {
        FighterBillboard->SetSprite(DefaultTexture.Object);
    }
}

void ASonoxoFighterCharacter::BeginPlay()
{
    Super::BeginPlay();
    SpawnLocation = GetActorLocation();
}

void ASonoxoFighterCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    AttackCooldown = FMath::Max(0.0f, AttackCooldown - DeltaSeconds);

    if (GetActorLocation().Z < KillZ)
    {
        LoseStockAndRespawn();
    }

    if (bCpuControlled)
    {
        UpdateCpu(DeltaSeconds);
    }
}

void ASonoxoFighterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ASonoxoFighterCharacter::MoveRight);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction(TEXT("LightAttack"), IE_Pressed, this, &ASonoxoFighterCharacter::LightAttack);
    PlayerInputComponent->BindAction(TEXT("HeavyAttack"), IE_Pressed, this, &ASonoxoFighterCharacter::HeavyAttack);
}

void ASonoxoFighterCharacter::SetFighterTexture(UTexture2D* Texture)
{
    if (Texture)
    {
        FighterBillboard->SetSprite(Texture);
    }
}

void ASonoxoFighterCharacter::SetCpuControlled(bool bEnabled, ASonoxoFighterCharacter* Target)
{
    bCpuControlled = bEnabled;
    CpuTarget = Target;
}

void ASonoxoFighterCharacter::MoveRight(float Value)
{
    if (bCpuControlled || FMath::IsNearlyZero(Value))
    {
        return;
    }

    FacingSign = FMath::Sign(Value);
    AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Value * MoveSpeedScale);
}

void ASonoxoFighterCharacter::LightAttack()
{
    PerformAttack(LightAttackDamage, 600.0f, 0.26f);
}

void ASonoxoFighterCharacter::HeavyAttack()
{
    PerformAttack(HeavyAttackDamage, 920.0f, 0.58f);
}

void ASonoxoFighterCharacter::PerformAttack(float Damage, float BaseKnockback, float Cooldown)
{
    if (AttackCooldown > 0.0f || Stocks <= 0)
    {
        return;
    }

    AttackCooldown = Cooldown;

    const FVector Start = GetActorLocation() + FVector(FacingSign * 45.0f, 0.0f, 20.0f);
    const FVector End = Start + FVector(FacingSign * AttackRange, 0.0f, 0.0f);

    FCollisionShape Sphere = FCollisionShape::MakeSphere(AttackRadius);
    TArray<FHitResult> Hits;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(SonoxoFighterAttack), false, this);

    if (GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_Pawn, Sphere, Params))
    {
        for (const FHitResult& Hit : Hits)
        {
            ASonoxoFighterCharacter* Other = Cast<ASonoxoFighterCharacter>(Hit.GetActor());
            if (Other && Other != this)
            {
                Other->ReceiveFighterHit(Damage, BaseKnockback, GetActorLocation());
                break;
            }
        }
    }
}

void ASonoxoFighterCharacter::ReceiveFighterHit(float DamageAmount, float BaseKnockback, const FVector& SourceLocation)
{
    DamagePercent += DamageAmount;

    const float HorizontalSign = GetActorLocation().X >= SourceLocation.X ? 1.0f : -1.0f;
    const float Scaling = 1.0f + (DamagePercent / 115.0f);
    const FVector Knockback(HorizontalSign * BaseKnockback * Scaling, 0.0f, 360.0f * Scaling);

    LaunchCharacter(Knockback, true, true);
}

void ASonoxoFighterCharacter::LoseStockAndRespawn()
{
    Stocks = FMath::Max(0, Stocks - 1);
    DamagePercent = 0.0f;

    if (Stocks <= 0)
    {
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
        GetCharacterMovement()->DisableMovement();
        return;
    }

    SetActorLocation(SpawnLocation + FVector(0.0f, 0.0f, 180.0f), false, nullptr, ETeleportType::TeleportPhysics);
    GetCharacterMovement()->Velocity = FVector::ZeroVector;
}

void ASonoxoFighterCharacter::UpdateCpu(float DeltaSeconds)
{
    ASonoxoFighterCharacter* Target = CpuTarget.Get();
    if (!Target || Stocks <= 0)
    {
        return;
    }

    const float DeltaX = Target->GetActorLocation().X - GetActorLocation().X;
    FacingSign = DeltaX >= 0.0f ? 1.0f : -1.0f;

    if (FMath::Abs(DeltaX) > 120.0f)
    {
        AddMovementInput(FVector(1.0f, 0.0f, 0.0f), FacingSign * 0.72f);
    }
    else if (AttackCooldown <= 0.0f)
    {
        if (FMath::FRand() > 0.68f)
        {
            HeavyAttack();
        }
        else
        {
            LightAttack();
        }
    }

    if (Target->GetActorLocation().Z > GetActorLocation().Z + 110.0f && GetCharacterMovement()->IsMovingOnGround())
    {
        Jump();
    }
}
