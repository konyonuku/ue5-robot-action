// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RobotCharacterBase.h"
#include "PlayerRobot.generated.h"

/**
 * 
 */

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class URobotDebugWidget;
struct FInputActionValue;

UCLASS()
class ROBOTACTIONPROJECT_API APlayerRobot : public ARobotCharacterBase
{
	GENERATED_BODY()
	

public:
    APlayerRobot();
    virtual void Restart() override;
    virtual void UnPossessed() override;
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Robot|Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Robot|Camera")
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(EditDefaultsOnly, Category = "Robot|Input")
    TObjectPtr<UInputAction> MoveAction;
    UPROPERTY(EditDefaultsOnly, Category = "Robot|Input")
    TObjectPtr<UInputAction> LookAction;
    UPROPERTY(EditDefaultsOnly, Category = "Robot|Input")
    TObjectPtr<UInputAction> MouseLookAction;
    UPROPERTY(EditDefaultsOnly, Category = "Robot|Input")
    TObjectPtr<UInputAction> JumpAction;
    UPROPERTY(EditDefaultsOnly, Category = "Robot|Input")
    TObjectPtr<UInputAction> QuickBoostAction;
    UPROPERTY(EditDefaultsOnly, Category = "Robot|Input")
    TObjectPtr<UInputAction> AssaultBoostAction;
    UPROPERTY(EditDefaultsOnly, Category = "Robot|UI")
    TSubclassOf<URobotDebugWidget> DebugWidgetClassToCreate;

private:
    void Move(const FInputActionValue& Value);
    void MoveEnded();
    void Look(const FInputActionValue& Value);
    void JumpStarted();
    void JumpEnded();
    void QuickBoostRequested();
    void AssaultBoostRequested();
    void TryCreateDebugHUD();
    void RemoveDebugHUD();
    FVector GetQuickBoostWorldDirection() const;
    FVector GetAssaultBoostWorldDirection() const;

    UPROPERTY(Transient)
    TObjectPtr<URobotDebugWidget> DebugWidget;
    bool bStatsReady = false;
    FVector2D CurrentMoveInput = FVector2D::ZeroVector;
};
