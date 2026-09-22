#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SonoxoFighterCharacter.generated.h"

class UBillboardComponent;
class UTexture2D;

UCLASS()
class SMASHRANDOMISER_API ASonoxoFighterCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASonoxoFighterCharacter();

    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, Category="Sonoxo Fighter")
    void SetFighterTexture(UTexture2D* Texture);

    UFUNCTION(BlueprintCallable, Category="Sonoxo Fighter")
    void SetCpuControlled(bool bEnabled, ASonoxoFighterCharacter* Target);

    UFUNCTION(BlueprintCallable, Category="Sonoxo Fighter")
    void ReceiveFighterHit(float DamageAmount, float BaseKnockback, const FVector& SourceLocation);

    float GetDamagePercent() const { return DamagePercent; }
    int32 GetStocks() const { return Stocks; }
    bool IsCpuControlled() const { return bCpuControlled; }

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category="Visual")
    UBillboardComponent* FighterBillboard;

    UPROPERTY(EditAnywhere, Category="Fighter")
    float MoveSpeedScale = 1.0f;

    UPROPERTY(EditAnywhere, Category="Fighter")
    float DamagePercent = 0.0f;

    UPROPERTY(EditAnywhere, Category="Fighter")
    int32 Stocks = 3;

    UPROPERTY(EditAnywhere, Category="Fighter")
    float KillZ = -700.0f;

    UPROPERTY(EditAnywhere, Category="Fighter")
    float LightAttackDamage = 7.0f;

    UPROPERTY(EditAnywhere, Category="Fighter")
    float HeavyAttackDamage = 14.0f;

    UPROPERTY(EditAnywhere, Category="Fighter")
    float AttackRange = 145.0f;

    UPROPERTY(EditAnywhere, Category="Fighter")
    float AttackRadius = 80.0f;

    FVector SpawnLocation;
    float FacingSign = 1.0f;
    float AttackCooldown = 0.0f;
    bool bCpuControlled = false;
    TWeakObjectPtr<ASonoxoFighterCharacter> CpuTarget;

    void MoveRight(float Value);
    void LightAttack();
    void HeavyAttack();
    void PerformAttack(float Damage, float BaseKnockback, float Cooldown);
    void LoseStockAndRespawn();
    void UpdateCpu(float DeltaSeconds);
};
