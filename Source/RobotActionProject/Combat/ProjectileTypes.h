#pragma once

#include "CoreMinimal.h"

class AActor;


struct FProjectileSpawnParams
{
	FVector StartLocation = FVector::ZeroVector;
	FVector Direction = FVector::ForwardVector;
	float Speed = 10000.0f;
	float Lifetime = 2.0f;
	float MaxTravelDistance = 20000.0f;
	float CollisionRadius = 2.0f;
	float Damage = 10.0f;

	TWeakObjectPtr<AActor> Owner;
	TArray<TWeakObjectPtr<AActor>> IgnoredActors;
};


struct FProjectileState
{
	FVector Position = FVector::ZeroVector;
	FVector Velocity = FVector::ZeroVector;
	float RemainingLifetime = 0.0f;
	float RemainingTravelDistance = 0.0f;
	float CollisionRadius = 0.0f;
	float Damage = 0.0f;

	TWeakObjectPtr<AActor> Owner;
	TArray<TWeakObjectPtr<AActor>> IgnoredActors;
};
