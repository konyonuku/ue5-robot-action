// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/ProjectileRenderer.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Combat/ProjectileSubsystem.h"



// Sets default values
AProjectileRenderer::AProjectileRenderer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	BulletInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BulletInstances"));
	SetRootComponent(BulletInstances);
	BulletInstances->SetMobility(EComponentMobility::Movable);
	BulletInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BulletInstances->SetGenerateOverlapEvents(false);
	BulletInstances->SetCanEverAffectNavigation(false);
	BulletInstances->SetCastShadow(false);

}

bool AProjectileRenderer::InitializeInstances(int32 Capacity)
{
	if (!IsValid(BulletInstances) || !IsValid(BulletInstances->GetStaticMesh()) || Capacity <= 0) return false;

	InstanceCapacity = 0;
	BulletInstances->ClearInstances();

	const FTransform HiddenTransform(FQuat::Identity, FVector::ZeroVector, FVector::ZeroVector);
	TArray<FTransform> Transforms;
	Transforms.Init(HiddenTransform, Capacity);
	BulletInstances->AddInstances(Transforms, false, false, false);

	if (BulletInstances->GetInstanceCount() != Capacity)
	{
		BulletInstances->ClearInstances();
		return false;
	}

	InstanceCapacity = Capacity;
	return true;

}

void AProjectileRenderer::ShowProjectile(int32 PoolIndex, const FVector& Position, const FVector& Direction, const FVector& Scale)
{
	if (!IsValid(BulletInstances) || PoolIndex < 0 || PoolIndex >= InstanceCapacity || PoolIndex >= BulletInstances->GetInstanceCount()) return;

	const FTransform Transform = MakeVisualTransform(Position, Direction, Scale);
	BulletInstances->UpdateInstanceTransform(PoolIndex, Transform, true, true, true);

}


void AProjectileRenderer::HideProjectile(int32 PoolIndex)
{
	if (!IsValid(BulletInstances) || PoolIndex < 0 || PoolIndex >= InstanceCapacity
		|| PoolIndex >= BulletInstances->GetInstanceCount())  return;

	const FTransform HiddenTransform(FQuat::Identity, FVector::ZeroVector, FVector::ZeroVector);
	BulletInstances->UpdateInstanceTransform(PoolIndex, HiddenTransform, false, true, true);

}

void AProjectileRenderer::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!IsValid(World)) return;

	UProjectileSubsystem* Subsystem = World->GetSubsystem<UProjectileSubsystem>();
	if (!IsValid(Subsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("ProjectileRenderer: projectile subsystem is unavailable."));
		return;
	}

	if (!Subsystem->RegisterRenderer(this))
	{
		UE_LOG(LogTemp, Warning, TEXT("ProjectileRenderer: registration failed. Check the mesh, pool initialization, and duplicate renderers."));
	}

}

void AProjectileRenderer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
	if (IsValid(World)) {
		UProjectileSubsystem* Subsystem = World->GetSubsystem<UProjectileSubsystem>();
		if (IsValid(Subsystem))
		{
			Subsystem->UnregisterRenderer(this);
		}
	}

	Super::EndPlay(EndPlayReason);

}

FTransform AProjectileRenderer::MakeVisualTransform(const FVector& Position, const FVector& Direction, const FVector& Scale) const
{
	const FTransform HiddenTransform(FQuat::Identity, FVector::ZeroVector, FVector::ZeroVector);
	if (Position.ContainsNaN() || Direction.ContainsNaN() || Scale.ContainsNaN() || MeshForwardAxis.ContainsNaN() || Scale.X <= 0.0 || Scale.Y <= 0.0 || Scale.Z <= 0.0) return HiddenTransform;
	
	const FVector Forward = MeshForwardAxis.GetSafeNormal();
	const FVector TravelDirection = Direction.GetSafeNormal();
	if (Forward.IsNearlyZero() || TravelDirection.IsNearlyZero()) return HiddenTransform;

	const FQuat Rotation = FQuat::FindBetweenNormals(Forward, TravelDirection);
	return FTransform(Rotation, Position, Scale);

}

