// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Combat/ProjectileTypes.h"
#include "ProjectileSubsystem.generated.h"

struct FHitResult;
class AProjectileRenderer;
class UNiagaraSystem;


/**
 * 
 */
UCLASS()
class ROBOTACTIONPROJECT_API UProjectileSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	// invalid spawn data or the pool is full -> return false
	bool TrySpawnProjectile(const FProjectileSpawnParams& Params);


	// link to ProjectileRenderer
	bool RegisterRenderer(AProjectileRenderer* InRenderer);
	void UnregisterRenderer(AProjectileRenderer* InRenderer);


private:
	bool UpdateProjectile(int32 PoolIndex, float DeltaTime, FHitResult& OutHit);
	void ReleaseProjectile(int32 ActiveListIndex);

	TArray<FProjectileState> ProjectilePool;
	TArray<int32> ActiveIndices;
	TArray<int32> FreeIndices;

	int32 PoolCapacity = 256;
	bool bDrawDebug = true;

	TWeakObjectPtr<AProjectileRenderer> Renderer;


	void PlayImpactEffect(UNiagaraSystem* ImpactSystem, const FHitResult& Hit);

};

/*
	apply damage: UpdateProjectile(): 이동 검사, 충돌 -> Tick(): ReleaseProjectile() 호출, 유효 충돌이면 데미지 적용
*/
