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
class ULockOnComponent;


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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Robot|LockOn")
    TObjectPtr<ULockOnComponent> LockOnComponent;


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

    UPROPERTY(EditDefaultsOnly, Category = "Robot|Input")
    TObjectPtr<UInputAction> LockOnAction;

    UPROPERTY(EditDefaultsOnly, Category = "Robot|Input")
    TObjectPtr<UInputAction> SwitchTargetLeftAction;

    UPROPERTY(EditDefaultsOnly, Category = "Robot|Input")
    TObjectPtr<UInputAction> SwitchTargetRightAction;

    UPROPERTY(EditDefaultsOnly, Category = "Robot|Input")
    TObjectPtr<UInputAction> LeftHandWeaponFireAction;

    UPROPERTY(EditDefaultsOnly, Category = "Robot|Input")
    TObjectPtr<UInputAction> RightHandWeaponFireAction;

    UPROPERTY(EditDefaultsOnly, Category = "Robot|Input")
    TObjectPtr<UInputAction> LeftShoulderWeaponFireAction;

    UPROPERTY(EditDefaultsOnly, Category = "Robot|Input")
    TObjectPtr<UInputAction> RightShoulderWeaponFireAction;
    
    
    UPROPERTY(EditDefaultsOnly, Category = "Robot|UI")
    TSubclassOf<URobotDebugWidget> DebugWidgetClassToCreate;

private:
    bool bStatsReady = false;
    
    void Move(const FInputActionValue& Value);
    void MoveEnded();
    void Look(const FInputActionValue& Value);
    void JumpStarted();
    void JumpEnded();
    

    void QuickBoostRequested();
    void AssaultBoostRequested();
    FVector GetQuickBoostWorldDirection() const;
    FVector GetAssaultBoostWorldDirection() const;
    FVector2D CurrentMoveInput = FVector2D::ZeroVector;


    void TryCreateDebugHUD();
    void RemoveDebugHUD();

    UPROPERTY(Transient)
    TObjectPtr<URobotDebugWidget> DebugWidget;


    void LockOnRequested();
    void SwitchTargetLeftRequested();
    void SwitchTargetRightRequested();

    void UpdateLockOnCamera(float DeltaTime);
    const float LockOnCameraTracingSpeed = 6.0f; //Camera Interpolation Speed


    void LeftHandWeaponFireRequested();
    void RightHandWeaponFireRequested();
    //void LeftShoulderWeaponFireRequested();
    //void RightShoulderWeaponFireRequested();


};
