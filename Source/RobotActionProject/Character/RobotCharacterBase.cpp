// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RobotCharacterBase.h"

#include "RobotActionProject.h" //LogRobotActionProject
#include "Character/RobotMovementComponent.h"
#include "Character/LockOnComponent.h"
#include "Components/SceneComponent.h"

// Sets default values
//ARobotCharacterBase::ARobotCharacterBase()
//{
// 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
//	PrimaryActorTick.bCanEverTick = true;
//
//}

ARobotCharacterBase::ARobotCharacterBase(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<URobotMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	UE_LOG(LogRobotActionProject, Display, TEXT(">MovementComponent: %s"), *GetCharacterMovement()->GetClass()->GetName());

	PrimaryActorTick.bCanEverTick = true;
}

void ARobotCharacterBase::SetHP(float NewHP)
{
	if (IsDead()) return;
	float clampedValue = FMath::Clamp(NewHP, 0.0f, MaxHP);
	if (clampedValue == HP) return;
	HP = clampedValue;
	if (HP == 0) HandleDeath();
	OnHPChanged.Broadcast(HP, MaxHP);
}

void ARobotCharacterBase::SetEN(float NewEN)
{
	if (IsDead()) return;
	float clampedValue = FMath::Clamp(NewEN, 0.0f, MaxEN);
	if (clampedValue == EN) return;

	const bool bBecameDepleted = EN > 0.0f && clampedValue <= 0.0f;
	EN = clampedValue;

	if (bBecameDepleted) {
		ENRecoveryDelayRemaining = DepletedENRecoveryDelay;
		StopAssaultBoost();
	}
	OnENChanged.Broadcast(EN, MaxEN);
}

void ARobotCharacterBase::SetACS(float NewACS)
{
	if (IsDead()) return;
	float clampedValue = FMath::Clamp(NewACS, 0.0f, MaxACS);
	if (clampedValue == ACS) return;
	ACS = clampedValue;
	OnACSChanged.Broadcast(ACS, MaxACS);
}

void ARobotCharacterBase::HandleDeath()
{
	if (IsDead()) return;

	bIsDead = true;
	StopAssaultBoost();
	bIsHovering = false;
	ENRecoveryDelayRemaining = 0.0f;

	URobotMovementComponent* MoveComp = Cast<URobotMovementComponent>(GetCharacterMovement());
	if (IsValid(MoveComp)) {
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();

		MoveComp->StopQuickBoost();
	}

	ULockOnComponent* LockOnComp = FindComponentByClass<ULockOnComponent>();
	if (IsValid(LockOnComp)) {
		LockOnComp->ClearLockOn();
	}

	OnDied.Broadcast();
}

bool ARobotCharacterBase::CanSpendEN(float Cost) const
{
	if (IsDead() || Cost <= 0.0f || GetEN() <= 0.0f) return false;

	return true;
}

bool ARobotCharacterBase::TrySpendEN(float Cost)
{
	if (!CanSpendEN(Cost)) return false;
	
	ENRecoveryDelayRemaining = NormalENRecoveryDelay;
	SetEN(GetEN() - Cost);

	return true;
}

bool ARobotCharacterBase::IsENDepleted() const
{
	return GetEN() <= 0.0f;
}

void ARobotCharacterBase::DoAssaultBoost(const FVector& WorldDirection)
{
	URobotMovementComponent* MoveComp = Cast<URobotMovementComponent>(GetCharacterMovement());
	if (IsDead() || IsENDepleted() || bIsAssaultBoosting || !IsValid(MoveComp)) return;
	if (MoveComp->IsQuickBoost() || (MoveComp->MovementMode != MOVE_Walking && MoveComp->MovementMode != MOVE_Falling)) return;
	if (WorldDirection.ContainsNaN() || WorldDirection.IsNearlyZero() || !IsValid(MoveComp->UpdatedComponent)) return;


	bIsAssaultBoosting = true;
	MoveComp->StartAssaultBoostMovement(WorldDirection.GetSafeNormal());
	UE_LOG(LogRobotActionProject, Display, TEXT("<AssaultBoostAction!!!>"));

}

void ARobotCharacterBase::DoQuickBoost(const FVector& WorldDirection)
{
	URobotMovementComponent* MoveComp = Cast<URobotMovementComponent>(GetMovementComponent());
	if (!IsValid(MoveComp)) return;

	if (IsDead() || MoveComp->IsQuickBoost()) return;

	const bool bFromAssault = IsAssaultBoosting() && MoveComp->MovementMode == MOVE_Flying;

	if (!bFromAssault && MoveComp->MovementMode != MOVE_Walking && MoveComp->MovementMode != MOVE_Falling) return;

	if (WorldDirection.ContainsNaN()) return;

	const FVector Direction = WorldDirection.GetSafeNormal2D();
	if (Direction.IsNearlyZero()) return;

	if (!TrySpendEN(30.0f)) {
		UE_LOG(LogRobotActionProject, Display, TEXT("Quick boost rejected: cannot spend EN."));
		return;
	}

	StopAssaultBoost();

	UE_LOG(LogRobotActionProject, Display, TEXT("<QuickBoostAction!!!>: %s"), *WorldDirection.ToString());

	MoveComp->StartQuickBoost(Direction);

}

void ARobotCharacterBase::StopAssaultBoost()
{
	if (!bIsAssaultBoosting) return;
	
	bIsAssaultBoosting = false;
	URobotMovementComponent* MoveComp = Cast<URobotMovementComponent>(GetCharacterMovement());
	if (IsValid(MoveComp)) {
		MoveComp->StopAssaultBoostMovement();
	}

	if (!IsDead() && !IsENDepleted()) {
		ENRecoveryDelayRemaining = NormalENRecoveryDelay;
	}
}


// Called when the game starts or when spawned
void ARobotCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	URobotMovementComponent* MoveComp = Cast<URobotMovementComponent>(GetCharacterMovement());
	if (IsValid(MoveComp)) {
		PrimaryActorTick.RemovePrerequisite(MoveComp, MoveComp->PrimaryComponentTick);
		MoveComp->AddTickPrerequisiteActor(this);
	}
	if (MaxHP < 1) MaxHP = 1;
	if (MaxEN < 1) MaxEN = 1;
	if (MaxACS < 1) MaxACS = 1;
	
	HP = MaxHP;
	EN = MaxEN;
	ACS = 0;
	bIsDead = false;
}

// Called every frame
void ARobotCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) return;

	const float DelayTime = FMath::Min(FMath::Max(ENRecoveryDelayRemaining, 0.0f), DeltaTime);
	ENRecoveryDelayRemaining = FMath::Max(0.0f, ENRecoveryDelayRemaining - DeltaTime);
	const float RecoveryTime = DeltaTime - DelayTime;

	// EN usage per tick
	if (bIsAssaultBoosting) {
		TrySpendEN(10.0f * DeltaTime);
		return;
	}
	if (bIsHovering) {
		TrySpendEN(5.0f * DeltaTime);
		return;
	}

	if (RecoveryTime <= 0.0f) return;

	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!IsValid(Movement)) return;

	float RecoveryRate = 0.0f;
	if (Movement->IsMovingOnGround()) {
		RecoveryRate = GroundENRecoveryRate;
	}
	else if (Movement->IsFalling()) {
		RecoveryRate = AirENRecoveryRate;
	}
	else {
		return;
	}

	SetEN(GetEN() + RecoveryRate * RecoveryTime);
}

// Called to bind functionality to input
void ARobotCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

