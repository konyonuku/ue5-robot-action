// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnComponent.generated.h"

class ARobotCharacterBase;

enum class ESwitchDirection : uint8
{
	Left, Right
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ROBOTACTIONPROJECT_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULockOnComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


public:
	void ToggleLockOn();
	bool TryLockOn();
	void ClearLockOn();
	bool IsLockedOn() const;
	ARobotCharacterBase* GetCurrentTarget() const;

	bool TrySwitchTarget(ESwitchDirection Direction);

private:
	bool CheckLineSightToTarget(const ARobotCharacterBase* Target) const;
	void GatherLockOnCandidates(TArray<ARobotCharacterBase*>& OutCandidates) const;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Robot|State", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<ARobotCharacterBase> CurrentTarget = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Robot|LockOn", meta = (ClampMin = "0.0"))
	float MaxAcquireDistance = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Robot|LockOn", meta = (ClampMin = "0.0"))
	float MaxMaintainDistance = 2500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Robot|LockOn", meta = (ClampMin = "0.0"))
	float OcclusionGracePeriod = 1.0f;

	float OccludedElapsedTime = 0.0f;

};
