// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RobotCharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRobotValueChanged, float, CurrentValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRobotDied);

UCLASS(Abstract, Blueprintable)
class ROBOTACTIONPROJECT_API ARobotCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARobotCharacterBase();

    UFUNCTION(BlueprintPure, Category = "Robot|Stats")
    float GetHP() const { return HP; }

    UFUNCTION(BlueprintPure, Category = "Robot|Stats")
    float GetMaxHP() const { return MaxHP; }

    UFUNCTION(BlueprintPure, Category = "Robot|Stats")
    float GetEN() const { return EN; }

    UFUNCTION(BlueprintPure, Category = "Robot|Stats")
    float GetMaxEN() const { return MaxEN; }

    UFUNCTION(BlueprintPure, Category = "Robot|Stats")
    float GetACS() const { return ACS; }

    UFUNCTION(BlueprintPure, Category = "Robot|Stats")
    float GetMaxACS() const { return MaxACS; }

    UFUNCTION(BlueprintPure, Category = "Robot|State")
    bool IsDead() const { return bIsDead; }

    UFUNCTION(BlueprintCallable, Category = "Robot|Stats")
    void SetHP(float NewHP);

    UFUNCTION(BlueprintCallable, Category = "Robot|Stats")
    void SetEN(float NewEN);

    UFUNCTION(BlueprintCallable, Category = "Robot|Stats")
    void SetACS(float NewACS);

    UPROPERTY(BlueprintAssignable, Category = "Robot|Events")
    FRobotValueChanged OnHPChanged;

    UPROPERTY(BlueprintAssignable, Category = "Robot|Events")
    FRobotValueChanged OnENChanged;

    UPROPERTY(BlueprintAssignable, Category = "Robot|Events")
    FRobotValueChanged OnACSChanged;

    UPROPERTY(BlueprintAssignable, Category = "Robot|Events")
    FRobotDied OnDied;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


private:
    void HandleDeath();

    UPROPERTY(EditDefaultsOnly, Category = "Robot|Stats", meta = (ClampMin = "1.0"))
    float MaxHP = 100.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Robot|Stats", meta = (ClampMin = "1.0"))
    float MaxEN = 100.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Robot|Stats", meta = (ClampMin = "1.0"))
    float MaxACS = 100.0f;

    UPROPERTY(VisibleInstanceOnly, Category = "Robot|Stats")
    float HP = 0.0f;

    UPROPERTY(VisibleInstanceOnly, Category = "Robot|Stats")
    float EN = 0.0f;

    UPROPERTY(VisibleInstanceOnly, Category = "Robot|Stats")
    float ACS = 0.0f;

    UPROPERTY(VisibleInstanceOnly, Category = "Robot|State")
    bool bIsDead = false;
};
