// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/RifleWeapon.h"
#include "Character/RobotCharacterBase.h"
#include "Combat/ProjectileSubsystem.h"
#include "Combat/ProjectileTypes.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"


// Sets default values
ARifleWeapon::ARifleWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void ARifleWeapon::StartFire()
{
	bFireInputHeld = true;
	TryFireShot();
}

void ARifleWeapon::StopFire()
{
	bFireInputHeld = false;
}

bool ARifleWeapon::CanFire() const
{
	const ARobotCharacterBase* Robot = Cast<ARobotCharacterBase>(GetOwner());
	if (!IsValid(Robot) || Robot->IsDead()) return false;

	if (!IsValid(WeaponMesh) || MuzzleSocketName.IsNone()) return false;
	if (!WeaponMesh->DoesSocketExist(MuzzleSocketName)) return false;

	const UWorld* World = GetWorld();
	if (!IsValid(World)) return false;

	return World->GetTimeSeconds() >= NextFireTime;

}

void ARifleWeapon::SetAimDirection(const FVector& WorldDirection)
{
	if (WorldDirection.ContainsNaN() || WorldDirection.IsNearlyZero()) return;
	AimDirection = WorldDirection.GetSafeNormal();

}

FVector ARifleWeapon::GetMuzzleLocation() const
{
	return WeaponMesh->GetSocketLocation(MuzzleSocketName);

}

void ARifleWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopFire();

	Super::EndPlay(EndPlayReason);

}

// Called every frame
void ARifleWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bFireInputHeld) return;

	TryFireShot();

}

bool ARifleWeapon::TryFireShot()
{
	if (!CanFire()) return false;

	UWorld* World = GetWorld();
	UProjectileSubsystem* Projectiles = World->GetSubsystem<UProjectileSubsystem>();
	if (!IsValid(Projectiles)) return false;

	FProjectileSpawnParams Params;
	Params.StartLocation = GetMuzzleLocation();
	Params.Direction = AimDirection;
	Params.Speed = ProjectileSpeed;
	Params.Lifetime = ProjectileLifetime;
	Params.MaxTravelDistance = ProjectileMaxDistance;
	Params.CollisionRadius = ProjectileRadius;
	Params.Damage = Damage;
	Params.VisualScale = ProjectileVisualScale;
	Params.Owner = GetOwner();
	Params.IgnoredActors.Add(this);
	Params.ImpactSystem = BulletImpactSystem.Get();

	if (!Projectiles->TrySpawnProjectile(Params)) return false;

	PlayMuzzleFlash();

	NextFireTime = World->GetTimeSeconds() + FireInterval;
	return true;

}

void ARifleWeapon::PlayMuzzleFlash()
{
	if (!IsValid(MuzzleFlashSystem) || !IsValid(WeaponMesh)) return;

	if (MuzzleSocketName.IsNone() || !WeaponMesh->DoesSocketExist(MuzzleSocketName)) return;

	UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFlashSystem.Get(), WeaponMesh.Get(), MuzzleSocketName,
		FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true, true, ENCPoolMethod::None);

}

