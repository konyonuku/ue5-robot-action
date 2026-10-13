// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/ProjectileSubsystem.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "DrawDebugHelpers.h"
#include "Combat/ProjectileRenderer.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"


void UProjectileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PoolCapacity = FMath::Max(1, PoolCapacity);
	ProjectilePool.Reset();
	ProjectilePool.SetNum(PoolCapacity);
	ActiveIndices.Reset(PoolCapacity);
	FreeIndices.Reset(PoolCapacity);

	for (int32 PoolIndex = PoolCapacity - 1; PoolIndex >= 0; --PoolIndex) {
		FreeIndices.Add(PoolIndex);
	}

}

void UProjectileSubsystem::Deinitialize()
{
	AProjectileRenderer* CurrentRenderer = Renderer.Get();
	if (IsValid(CurrentRenderer)) {
		for (const int32 PoolIndex : ActiveIndices) {
			CurrentRenderer->HideProjectile(PoolIndex);
		}
	}

	Renderer.Reset();
	ActiveIndices.Empty();
	FreeIndices.Empty();
	ProjectilePool.Empty();

	Super::Deinitialize();
}

void UProjectileSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// A swapped-in entry was already visited when iterating backwards.
	for (int32 ActiveListIndex = ActiveIndices.Num() - 1; ActiveListIndex >= 0; --ActiveListIndex)
	{
		const int32 PoolIndex = ActiveIndices[ActiveListIndex];
		FHitResult Hit;
		if (!UpdateProjectile(PoolIndex, DeltaTime, Hit))
		{
			UNiagaraSystem* ImpactSystem = ProjectilePool[PoolIndex].ImpactSystem.Get();
			const bool bImpact = Hit.bBlockingHit || Hit.bStartPenetrating;

			ReleaseProjectile(ActiveListIndex);

			if (bImpact) PlayImpactEffect(ImpactSystem, Hit);

		}
		else
		{
			AProjectileRenderer* CurrentRenderer = Renderer.Get();
			if (IsValid(CurrentRenderer))
			{
				const FProjectileState& State = ProjectilePool[PoolIndex];
				CurrentRenderer->ShowProjectile(PoolIndex, State.Position, State.Velocity.GetSafeNormal(), State.VisualScale);
			}
		}

	}
}

TStatId UProjectileSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UProjectileSubsystem, STATGROUP_Tickables); // tickable subsystem template
}

bool UProjectileSubsystem::TrySpawnProjectile(const FProjectileSpawnParams& Params)
{
	if (FreeIndices.IsEmpty() || !Params.Owner.IsValid()) return false;

	if (Params.StartLocation.ContainsNaN() || Params.Direction.ContainsNaN()) return false;

	if (!FMath::IsFinite(Params.Speed) || Params.Speed <= 0.0f
		|| !FMath::IsFinite(Params.Lifetime) || Params.Lifetime <= 0.0f
		|| !FMath::IsFinite(Params.MaxTravelDistance) || Params.MaxTravelDistance <= 0.0f
		|| !FMath::IsFinite(Params.CollisionRadius) || Params.CollisionRadius <= 0.0f
		|| !FMath::IsFinite(Params.Damage) || Params.Damage < 0.0f) {
		return false;
	}


	const FVector Direction = Params.Direction.GetSafeNormal();
	if (Direction.ContainsNaN() || Direction.IsNearlyZero()) return false;

	const int32 PoolIndex = FreeIndices.Last();
	if (!ProjectilePool.IsValidIndex(PoolIndex)) return false;

	FProjectileState& State = ProjectilePool[PoolIndex];
	State.Position = Params.StartLocation;
	State.Velocity = Direction * Params.Speed;
	State.RemainingLifetime = Params.Lifetime;
	State.RemainingTravelDistance = Params.MaxTravelDistance;
	State.CollisionRadius = Params.CollisionRadius;
	State.Damage = Params.Damage;
	State.VisualScale = Params.VisualScale;
	State.Owner = Params.Owner;
	State.IgnoredActors = Params.IgnoredActors;
	State.ImpactSystem = Params.ImpactSystem;

	FreeIndices.Pop(EAllowShrinking::No);
	ActiveIndices.Add(PoolIndex);

	AProjectileRenderer* CurrentRenderer = Renderer.Get();
	if (IsValid(CurrentRenderer)) {
		CurrentRenderer->ShowProjectile(PoolIndex, State.Position, Direction, State.VisualScale);
	}
	return true;

}

bool UProjectileSubsystem::RegisterRenderer(AProjectileRenderer* InRenderer)
{
	if (!IsValid(InRenderer) || InRenderer->GetWorld() != GetWorld()) return false;

	AProjectileRenderer* CurrentRenderer = Renderer.Get();
	if (IsValid(CurrentRenderer)) return CurrentRenderer == InRenderer;

	if (ProjectilePool.IsEmpty()) return false;

	if (!InRenderer->InitializeInstances(ProjectilePool.Num())) return false;

	Renderer = InRenderer;
	for (const int32 PoolIndex : ActiveIndices)
	{
		const FProjectileState& State = ProjectilePool[PoolIndex];
		InRenderer->ShowProjectile(PoolIndex, State.Position, State.Velocity.GetSafeNormal(), State.VisualScale);
	}

	return true;

}

void UProjectileSubsystem::UnregisterRenderer(AProjectileRenderer* InRenderer)
{
	if (!Renderer.IsValid()) {
		Renderer.Reset();
		return;
	}

	AProjectileRenderer* CurrentRenderer = Renderer.Get();
	if (CurrentRenderer != InRenderer) return;

	if (IsValid(CurrentRenderer)) {
		for (const int32 PoolIndex : ActiveIndices) {
			CurrentRenderer->HideProjectile(PoolIndex);
		}
	}
	Renderer.Reset();

}

void UProjectileSubsystem::ReleaseProjectile(int32 ActiveListIndex)
{
	if (!ActiveIndices.IsValidIndex(ActiveListIndex)) return;

	const int32 PoolIndex = ActiveIndices[ActiveListIndex];
	if (!ProjectilePool.IsValidIndex(PoolIndex)) return;

	AProjectileRenderer* CurrentRenderer = Renderer.Get();
	if (IsValid(CurrentRenderer)) {
		CurrentRenderer->HideProjectile(PoolIndex);
	}
	ActiveIndices.RemoveAtSwap(ActiveListIndex, 1, EAllowShrinking::No);
	FreeIndices.Add(PoolIndex);

}

void UProjectileSubsystem::PlayImpactEffect(UNiagaraSystem* ImpactSystem, const FHitResult& Hit)
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || !IsValid(ImpactSystem)) return;

	if (!Hit.bBlockingHit && !Hit.bStartPenetrating) return;

	if (Hit.ImpactPoint.ContainsNaN() || Hit.ImpactNormal.ContainsNaN()) return;

	const FVector Normal = Hit.ImpactNormal.GetSafeNormal();
	if (Normal.IsNearlyZero()) return;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, ImpactSystem, Hit.ImpactPoint, Normal.Rotation(), FVector::OneVector, true, true, ENCPoolMethod::None);

}

bool UProjectileSubsystem::UpdateProjectile(int32 PoolIndex, float DeltaTime, FHitResult& OutHit)
{
	OutHit = FHitResult();

	if (!ProjectilePool.IsValidIndex(PoolIndex)) return false;

	FProjectileState& State = ProjectilePool[PoolIndex];
	if (State.RemainingLifetime <= 0.0f || State.RemainingTravelDistance <= 0.0f) return false;

	if (DeltaTime <= 0.0f) return true;

	UWorld* World = GetWorld();
	const double Speed = State.Velocity.Size();
	if (!IsValid(World) || Speed <= UE_SMALL_NUMBER) return false;

	const double Lifetime = State.RemainingLifetime;
	const double RangeTime = State.RemainingTravelDistance / Speed;
	const double MoveTime = FMath::Min3(static_cast<double>(DeltaTime), Lifetime, RangeTime);
	const double MoveDistance = FMath::Min(MoveTime * Speed, static_cast<double>(State.RemainingTravelDistance));
	const FVector Start = State.Position;
	const FVector End = Start + (State.Velocity / Speed) * MoveDistance;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ProjectileMovement), false);
	QueryParams.bFindInitialOverlaps = true;

	AActor* Owner = State.Owner.Get();
	if (IsValid(Owner)) QueryParams.AddIgnoredActor(Owner);

	for (const TWeakObjectPtr<AActor>& IgnoredActor : State.IgnoredActors)
	{
		AActor* Actor = IgnoredActor.Get();
		if (IsValid(Actor)) {
			QueryParams.AddIgnoredActor(Actor);
		}
	}

	const bool bHit = World->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(State.CollisionRadius), QueryParams);

	const bool bCollided = bHit || OutHit.bStartPenetrating;
	const double TravelFraction = bCollided ? FMath::Clamp(static_cast<double>(OutHit.Time), 0.0, 1.0) : 1.0; // projectile start->final position ratio
	State.Position = bCollided ? OutHit.Location : End;
	State.RemainingLifetime = FMath::Max(0.0f, State.RemainingLifetime - static_cast<float>(MoveTime * TravelFraction));
	State.RemainingTravelDistance = FMath::Max(0.0f, State.RemainingTravelDistance - static_cast<float>(MoveDistance * TravelFraction));

	if (bDrawDebug) {
		DrawDebugLine(World, Start, State.Position, bCollided ? FColor::Red : FColor::Green, false, 0.1f, 0, 1.0f);
	}

	if (bCollided) return false;
	if (MoveTime >= Lifetime) State.RemainingLifetime = 0.0f;
	if (MoveTime >= RangeTime) State.RemainingTravelDistance = 0.0f;

	return (State.RemainingLifetime > 0.0f && State.RemainingTravelDistance > 0.0f);
}
