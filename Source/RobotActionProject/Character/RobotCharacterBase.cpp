// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RobotCharacterBase.h"

// Sets default values
ARobotCharacterBase::ARobotCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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
	EN = clampedValue;
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
	if(UCharacterMovementComponent* MoveComp = GetCharacterMovement()) {
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}
	OnDied.Broadcast();
}

// Called when the game starts or when spawned
void ARobotCharacterBase::BeginPlay()
{
	Super::BeginPlay();
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

}

// Called to bind functionality to input
void ARobotCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

