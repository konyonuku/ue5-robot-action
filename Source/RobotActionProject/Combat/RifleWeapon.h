// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RifleWeapon.generated.h"

class UStaticMeshComponent;
class UNiagaraSystem;


UCLASS()
class ROBOTACTIONPROJECT_API ARifleWeapon : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARifleWeapon();

	void StartFire();
	void StopFire();
	bool CanFire() const;

	void SetAimDirection(const FVector& WorldDirection);

	FVector GetMuzzleLocation() const;


protected:
	//// Called when the game starts or when spawned
	//virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;


	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Fire", meta = (ClampMin = "0.001"))
	float FireInterval = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Projectile", meta = (ClampMin = "0.001"))
	float ProjectileSpeed = 10000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Projectile", meta = (ClampMin = "0.001"))
	float ProjectileLifetime = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Projectile", meta = (ClampMin = "0.001"))
	float ProjectileMaxDistance = 20000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Projectile", meta = (ClampMin = "0.001"))
	float ProjectileRadius = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Projectile")
	FVector ProjectileVisualScale = FVector(0.2f);

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Damage", meta = (ClampMin = "0.0"))
	float Damage = 10.0f;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName = TEXT("Muzzle");

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
	TObjectPtr<UNiagaraSystem> MuzzleFlashSystem;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
	TObjectPtr<UNiagaraSystem> BulletImpactSystem;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:
	bool TryFireShot();

	bool bFireInputHeld = false;
	double NextFireTime = 0.0;

	FVector AimDirection = FVector::ForwardVector;
	// Non-Lockon -> Crosshair(Camera)
	// Lockon -> Muzzle to Target

	void PlayMuzzleFlash();

};
