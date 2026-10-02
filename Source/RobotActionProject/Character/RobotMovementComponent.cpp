// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/RobotMovementComponent.h"
#include "Character/RobotCharacterBase.h"
#include "Components/SceneComponent.h"


URobotMovementComponent::URobotMovementComponent()
{
	// Configure character movement
	bOrientRotationToMovement = true;
	RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	JumpZVelocity = 500.f;
	AirControl = 0.35f;
	MaxWalkSpeed = 500.f;
	MinAnalogWalkSpeed = 20.f;
	BrakingDecelerationWalking = 2000.f;
	BrakingDecelerationFalling = 1500.0f;
}

void URobotMovementComponent::SetBoostEnabled(bool bEnabled)
{
	bBoostEnabled = bEnabled;
}

bool URobotMovementComponent::IsBoostEnabled() const
{
	return bBoostEnabled;
}

void URobotMovementComponent::StartQuickBoost(const FVector& WorldDirection)
{
	bIsQuickBoosting = true;
	QuickBoostDirection = WorldDirection;
	QuickBoostTimeRemaining = QuickBoostDuration;
}

void URobotMovementComponent::StopQuickBoost()
{
	if (IsQuickBoost() == false) return;

	bIsQuickBoosting = false;
	QuickBoostTimeRemaining = 0.0f;

	// Falling(Horizontal) velocity after dash(QuickBoost)
	FVector FallingVelocity(Velocity.X, Velocity.Y, 0.0f);
	FallingVelocity = FallingVelocity.GetClampedToMaxSize(GetMaxSpeed());

	Velocity.X = FallingVelocity.X;
	Velocity.Y = FallingVelocity.Y;

}

bool URobotMovementComponent::IsQuickBoost() const
{
	return bIsQuickBoosting;
}

bool URobotMovementComponent::IsAssaultBoostMovementActive() const
{
	const ARobotCharacterBase* Robot = Cast<ARobotCharacterBase>(CharacterOwner);
	return Robot != nullptr && !Robot->IsDead() && Robot->IsAssaultBoosting() && MovementMode == MOVE_Flying;
}

bool URobotMovementComponent::FindAssaultBoostFloor(FFindFloorResult& Floor) const
{
	if (!HasValidData()) return false;
	FindFloor(UpdatedComponent->GetComponentLocation(), Floor, false);
	return Floor.IsWalkableFloor() && Floor.GetDistanceToFloor() <= MaxStepHeight;
}

void URobotMovementComponent::SetAssaultBoostDirection(const FVector& WorldDirection)
{
	if (WorldDirection.ContainsNaN() || WorldDirection.IsNearlyZero()) return;
	AssaultBoostDirection = WorldDirection.GetSafeNormal();
}

void URobotMovementComponent::StartAssaultBoostMovement(const FVector& WorldDirection)
{
	if (!HasValidData() || WorldDirection.ContainsNaN() || WorldDirection.IsNearlyZero()) return;
	SetAssaultBoostDirection(WorldDirection);
	ConsumeInputVector();
	Acceleration = FVector::ZeroVector;
	SetMovementMode(MOVE_Flying);
}

void URobotMovementComponent::StopAssaultBoostMovement()
{
	AssaultBoostDirection = FVector::ZeroVector;
	if (!HasValidData()) return;

	const ARobotCharacterBase* Robot = Cast<ARobotCharacterBase>(CharacterOwner);
	if (Robot != nullptr && Robot->IsDead()) {
		StopMovementImmediately();
		DisableMovement();
		return;
	}

	if (MovementMode != MOVE_Flying) return;
	FFindFloorResult Floor;
	FindFloor(UpdatedComponent->GetComponentLocation(), Floor, false);
	const bool bOnFloor = Floor.IsWalkableFloor() && Floor.GetDistanceToFloor() <= MAX_FLOOR_DIST;
	const double ExitVerticalSpeed = Velocity.Z;
	SetMovementMode(bOnFloor ? MOVE_Walking : MOVE_Falling);
	FVector HorizontalVelocity(Velocity.X, Velocity.Y, 0.0);
	HorizontalVelocity = HorizontalVelocity.GetClampedToMaxSize(GetMaxSpeed());
	Velocity = FVector(HorizontalVelocity.X, HorizontalVelocity.Y, bOnFloor ? 0.0 : ExitVerticalSpeed);

}

float URobotMovementComponent::GetMaxSpeed() const
{
	if (IsAssaultBoostMovementActive()) return FMath::Max(0.0f, AssaultBoostSpeed);
	if (!IsBoostEnabled()) {
		return Super::GetMaxSpeed();
	}

	switch (MovementMode) {
		case MOVE_Walking:
			return FMath::Max(0.0f, GroundBoostSpeed);
		case MOVE_Falling:
			return FMath::Max(0.0f, AirBoostSpeed);
		default:
			return Super::GetMaxSpeed();
	}
}

float URobotMovementComponent::GetMaxAcceleration() const
{
	if (!IsBoostEnabled()) {
		return Super::GetMaxAcceleration();
	}

	switch (MovementMode) {
	case MOVE_Walking:
		return FMath::Max(0.0f, GroundBoostAcceleration);
	case MOVE_Falling:
		return FMath::Max(0.0f, AirBoostAcceleration);
	default:
		return Super::GetMaxAcceleration();
	}
}

float URobotMovementComponent::GetMaxBrakingDeceleration() const
{
	if (!IsBoostEnabled()) {
		return Super::GetMaxBrakingDeceleration();
	}

	switch (MovementMode) {
	case MOVE_Walking:
		return FMath::Max(0.0f, GroundBoostBraking);
	case MOVE_Falling:
		return FMath::Max(0.0f, AirBoostBraking);
	default:
		return Super::GetMaxBrakingDeceleration();
	}
}

void URobotMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	if (IsAssaultBoostMovementActive()) {
		FVector Direction = AssaultBoostDirection;
		FFindFloorResult Floor;

		if (Direction.Z <= 0.0 && FindAssaultBoostFloor(Floor)) {
			const FVector HorizontalDirection(Direction.X, Direction.Y, 0.0);
			Direction = FVector::VectorPlaneProject(HorizontalDirection, Floor.HitResult.ImpactNormal).GetSafeNormal();
		}
		Velocity = Direction * FMath::Max(0.0f, AssaultBoostSpeed);
		return;
	}
	if (bIsQuickBoosting && (IsMovingOnGround() || IsFalling())) {
		Velocity.X = QuickBoostDirection.X * QuickBoostSpeed;
		Velocity.Y = QuickBoostDirection.Y * QuickBoostSpeed;
		
		return;
	}

	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);

}

void URobotMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	Super::HandleImpact(Hit, TimeSlice, MoveDelta);
	if (!IsAssaultBoostMovementActive() || !Hit.IsValidBlockingHit() || IsWalkable(Hit)) return;

	if (FVector::DotProduct(MoveDelta.GetSafeNormal(), Hit.ImpactNormal) < -0.1) {
		if (ARobotCharacterBase* Robot = Cast<ARobotCharacterBase>(CharacterOwner)) {
			Robot->StopAssaultBoost();
		}
	}
}

void URobotMovementComponent::PhysFlying(float DeltaTime, int32 Iterations)
{
	if (!IsAssaultBoostMovementActive()) {
		Super::PhysFlying(DeltaTime, Iterations);
		return;
	}

	float RemainingTime = DeltaTime;
	while (RemainingTime >= MIN_TICK_TIME && Iterations < MaxSimulationIterations && HasValidData() && IsAssaultBoostMovementActive()) {
		++Iterations;
		const float TimeTick = GetSimulationTimeStep(RemainingTime, Iterations);
		RemainingTime -= TimeTick;
		bJustTeleported = false;
		FFindFloorResult Floor;
		const bool bFollowFloor = AssaultBoostDirection.Z <= 0.0 && FindAssaultBoostFloor(Floor);
		CalcVelocity(TimeTick, 0.0f, false, 0.0f);
		const FVector Delta = Velocity * TimeTick;
		FHitResult Hit(1.0f);
		SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

		if (Hit.IsValidBlockingHit()) {
			HandleImpact(Hit, TimeTick, Delta);
			if (!IsAssaultBoostMovementActive()) {
				StartNewPhysics(RemainingTime + TimeTick * (1.0f - Hit.Time), Iterations);
				return;
			}

			const FVector SlideDelta = FVector::VectorPlaneProject(Delta * (1.0f - Hit.Time), Hit.Normal);
			FHitResult SlideHit(1.0f);
			SafeMoveUpdatedComponent(SlideDelta, UpdatedComponent->GetComponentQuat(), true, SlideHit);
			if (SlideHit.IsValidBlockingHit()) HandleImpact(SlideHit, TimeTick, SlideDelta);
			if (!IsAssaultBoostMovementActive()) {
				StartNewPhysics(RemainingTime, Iterations);
				return;
			}
		}

		if (bFollowFloor && FindAssaultBoostFloor(Floor)) {
			const float TargetDistance = (MIN_FLOOR_DIST + MAX_FLOOR_DIST) * 0.5f;
			const FVector FloorDelta(0.0, 0.0, TargetDistance - Floor.GetDistanceToFloor());
			FHitResult FloorHit(1.0f);
			SafeMoveUpdatedComponent(FloorDelta, UpdatedComponent->GetComponentQuat(), true, FloorHit);
			if (FloorHit.IsValidBlockingHit()) HandleImpact(FloorHit, TimeTick, FloorDelta);
			if (!IsAssaultBoostMovementActive()) {
				StartNewPhysics(RemainingTime, Iterations);
				return;
			}
		}
	}

}

void URobotMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (IsQuickBoost()) {
		QuickBoostTimeRemaining -= DeltaTime;
		if (QuickBoostTimeRemaining <= 0.0f) StopQuickBoost();
	}

}
