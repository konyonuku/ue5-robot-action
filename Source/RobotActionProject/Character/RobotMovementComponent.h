// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RobotMovementComponent.generated.h"

/**
 *
 */
UCLASS()
class ROBOTACTIONPROJECT_API URobotMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    URobotMovementComponent();

    UFUNCTION(BlueprintCallable, Category = "Robot|Movement")
    void SetBoostEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Robot|Movement")
    bool IsBoostEnabled() const;

    void StartQuickBoost(const FVector& WorldDirection);
    void StopQuickBoost();
    bool IsQuickBoost() const;

    void StartAssaultBoostMovement(const FVector& WorldDirection);
    void SetAssaultBoostDirection(const FVector& WorldDirection);
    void StopAssaultBoostMovement();


protected:
    virtual float GetMaxSpeed() const override;
    virtual float GetMaxAcceleration() const override;
    virtual float GetMaxBrakingDeceleration() const override;

    virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
    virtual void PhysFlying(float DeltaTime, int32 Iterations) override;
    virtual void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.0f, const FVector& MoveDelta = FVector::ZeroVector) override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Robot|Movement")
    bool bBoostEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Robot|Movement", meta = (ClampMin = "0.0"))
    float GroundBoostSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Robot|Movement", meta = (ClampMin = "0.0"))
    float AirBoostSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Robot|Movement", meta = (ClampMin = "0.0"))
    float GroundBoostAcceleration = 2400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Robot|Movement", meta = (ClampMin = "0.0"))
    float AirBoostAcceleration = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Robot|Movement", meta = (ClampMin = "0.0"))
    float GroundBoostBraking = 1600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Robot|Movement", meta = (ClampMin = "0.0"))
    float AirBoostBraking = 300.0f;


private:
    bool IsAssaultBoostMovementActive() const;
    bool FindAssaultBoostFloor(FFindFloorResult& Floor) const;

    bool bIsQuickBoosting = false;
    FVector QuickBoostDirection = FVector::ZeroVector;
    float QuickBoostTimeRemaining = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Robot|Movement", meta = (ClampMin = "0.0"))
    float QuickBoostSpeed = 2000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Robot|Movement", meta = (ClampMin = "0.0"))
    float QuickBoostDuration = 0.3f;


    FVector AssaultBoostDirection = FVector::ZeroVector;

    UPROPERTY(EditDefaultsOnly, Category = "Robot|Movement", meta = (ClampMin = "0.0", Units = "cm/s"))
    float AssaultBoostSpeed = 1400.0f;
};
