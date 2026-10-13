// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileRenderer.generated.h"

class UInstancedStaticMeshComponent;


UCLASS()
class ROBOTACTIONPROJECT_API AProjectileRenderer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectileRenderer();

	bool InitializeInstances(int32 Capacity);
	void ShowProjectile(int32 PoolIndex, const FVector& Position, const FVector& Direction, const FVector& Scale);
	void HideProjectile(int32 PoolIndex);


protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


private:	
	UPROPERTY(VisibleAnywhere, Category = "Projectile|Visual")
	TObjectPtr<UInstancedStaticMeshComponent> BulletInstances;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Visual")
	FVector MeshForwardAxis = FVector::UpVector;

	int32 InstanceCapacity = 0;
	FTransform MakeVisualTransform(const FVector& Position, const FVector& Direction, const FVector& Scale) const;

};
