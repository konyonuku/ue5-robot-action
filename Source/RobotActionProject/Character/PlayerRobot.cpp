// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PlayerRobot.h"

#include "RobotActionProject.h" //LogRobotActionProject
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h" //CameraBoom
#include "GameFramework/PlayerController.h"
#include "UI/RobotDebugWidget.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "Character/RobotMovementComponent.h"
#include "Character/LockOnComponent.h"

#include "Combat/ProjectileSubsystem.h"


APlayerRobot::APlayerRobot()
{
	// [template code from Character class of 3rd-person template project] 
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	// end template


	LockOnComponent = CreateDefaultSubobject<ULockOnComponent>(TEXT("LockOnComponent"));

}


void APlayerRobot::Restart()
{
	Super::Restart();

	TryCreateDebugHUD();
}

void APlayerRobot::UnPossessed()
{
	RemoveDebugHUD();

	Super::UnPossessed();
}

void APlayerRobot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsValid(GEngine))
	{
		ARobotCharacterBase* Target = IsValid(LockOnComponent) ? LockOnComponent->GetCurrentTarget() : nullptr;

		FString DistanceMessage = TEXT("Target Distance: --");

		if (Target != nullptr) {
			const double DistanceCm = FVector::Dist(GetActorLocation(), Target->GetActorLocation());

			DistanceMessage = FString::Printf(TEXT("Target Distance: %.1f m"), DistanceCm / 100.0);
		}

		GEngine->AddOnScreenDebugMessage(1001, 0.1f, FColor::Yellow, DistanceMessage);
	}


	if (IsDead() || !IsValid(GetController())) return;

	UpdateLockOnCamera(DeltaTime);

	if (IsAssaultBoosting()) {
		URobotMovementComponent* MoveComp = Cast<URobotMovementComponent>(GetCharacterMovement());
		if (IsValid(MoveComp)) {
			MoveComp->SetAssaultBoostDirection(GetAssaultBoostWorldDirection());
		}
	}

}

void APlayerRobot::BeginPlay()
{
	Super::BeginPlay();

	bStatsReady = true; // HUD can read robot's stats
	TryCreateDebugHUD();
}

void APlayerRobot::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	bStatsReady = false;
	RemoveDebugHUD();

	Super::EndPlay(EndPlayReason);
}


void APlayerRobot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	//Cast EnhancedInput(UInputComponent->UEnhancedInputComponent)
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!IsValid(EnhancedInput)) {
		UE_LOG(LogRobotActionProject, Error, TEXT("PlayerRobot needs a valid EnhancedInputComponent."));
		return;
	}

	//Bind Move
	if (IsValid(MoveAction)) {
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerRobot::Move);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &APlayerRobot::MoveEnded);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Canceled, this, &APlayerRobot::MoveEnded);
	}
	else {
		UE_LOG(LogRobotActionProject, Warning, TEXT("MoveAction is not assigned or is invalid."));
	}

	//Bind Look
	if (IsValid(LookAction)) {
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerRobot::Look);
	}
	else {
		UE_LOG(LogRobotActionProject, Warning, TEXT("LookAction is not assigned or is invalid."));
	}

	//Bind MouseLook
	if (IsValid(MouseLookAction)) {
		EnhancedInput->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &APlayerRobot::Look);
	}
	else {
		UE_LOG(LogRobotActionProject, Warning, TEXT("MouseLookAction is not assigned or is invalid."));
	}
	
	//Bind Jump
	if (IsValid(JumpAction)) {
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerRobot::JumpStarted);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayerRobot::JumpEnded);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Canceled, this, &APlayerRobot::JumpEnded);
	}
	else {
		UE_LOG(LogRobotActionProject, Warning, TEXT("JumpAction is not assigned or is invalid."));
	}

	//Bind QuickBoost
	if (IsValid(QuickBoostAction)) {
		EnhancedInput->BindAction(QuickBoostAction, ETriggerEvent::Started, this, &APlayerRobot::QuickBoostRequested);
	}
	else {
		UE_LOG(LogRobotActionProject, Warning, TEXT("QuickBoostAction is not assigned or is invalid."));
	}

	//Bind AssaultBoost
	if (IsValid(AssaultBoostAction)) {
		EnhancedInput->BindAction(AssaultBoostAction, ETriggerEvent::Started, this, &APlayerRobot::AssaultBoostRequested);
	}
	else {
		UE_LOG(LogRobotActionProject, Warning, TEXT("AssaultBoostAction is not assigned or is invalid."));
	}
	
	//Bind LockOn
	if (IsValid(LockOnAction)) {
		EnhancedInput->BindAction(LockOnAction, ETriggerEvent::Started, this, &APlayerRobot::LockOnRequested);
	}
	else {
		UE_LOG(LogRobotActionProject, Warning, TEXT("LockOnAction is not assigned or is invalid."));
	}

	//Bind LockOn Switch To Right
	if (IsValid(SwitchTargetRightAction)) {
		EnhancedInput->BindAction(SwitchTargetRightAction, ETriggerEvent::Started, this, &APlayerRobot::SwitchTargetRightRequested);
	}
	else {
		UE_LOG(LogRobotActionProject, Warning, TEXT("SwitchTargetRightAction is not assigned or is invalid."));
	}

	//Bind LockOn Switch To Left
	if (IsValid(SwitchTargetLeftAction)) {
		EnhancedInput->BindAction(SwitchTargetLeftAction, ETriggerEvent::Started, this, &APlayerRobot::SwitchTargetLeftRequested);
	}
	else {
		UE_LOG(LogRobotActionProject, Warning, TEXT("SwitchTargetLeftAction is not assigned or is invalid."));
	}

	//Bind LH Fire
	if (IsValid(LeftHandWeaponFireAction)) {
		EnhancedInput->BindAction(LeftHandWeaponFireAction, ETriggerEvent::Started, this, &APlayerRobot::LeftHandWeaponFireRequested);
	}
	else {
		UE_LOG(LogRobotActionProject, Warning, TEXT("LeftHandWeaponFireAction is not assigned or is invalid."));
	}

	//Bind RH Fire
	if (IsValid(RightHandWeaponFireAction)) {
		EnhancedInput->BindAction(RightHandWeaponFireAction, ETriggerEvent::Started, this, &APlayerRobot::RightHandWeaponFireRequested);
	}
	else {
		UE_LOG(LogRobotActionProject, Warning, TEXT("RightHandWeaponFireAction is not assigned or is invalid."));
	}
	
}

// Unreal Engine: Left-handed + Z-up -> Forward: +x, Right: +y, Up: +z
void APlayerRobot::Move(const FInputActionValue& Value)
{
	if (IsDead() || !IsValid(GetController())) return;

	FVector2D MovementVector = Value.Get<FVector2D>(); //(Right, Forward)
	CurrentMoveInput = MovementVector;


	const URobotMovementComponent* MoveComp = Cast<URobotMovementComponent>(GetCharacterMovement());

	if (IsAssaultBoosting() || (IsValid(MoveComp) && MoveComp->IsQuickBoost())) return;


	const FRotator Rotation = GetController()->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y); 
	AddMovementInput(RightDirection, MovementVector.X);
}

void APlayerRobot::MoveEnded()
{
	CurrentMoveInput = FVector2D::ZeroVector;
}

void APlayerRobot::Look(const FInputActionValue& Value)
{
	if (IsDead() || !IsValid(GetController())) return;
	if (IsValid(LockOnComponent) && LockOnComponent->IsLockedOn()) return;
	

	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);   //Yaw
	AddControllerPitchInput(LookAxisVector.Y); //Pitch

}

void APlayerRobot::JumpStarted()
{
	if (!IsDead() && !IsAssaultBoosting()) {
		Jump();
	}

}

void APlayerRobot::JumpEnded()
{
	StopJumping();
}

void APlayerRobot::QuickBoostRequested()
{
	if (IsDead()) return;

	DoQuickBoost(GetQuickBoostWorldDirection());
}

void APlayerRobot::AssaultBoostRequested()
{
	if (IsDead()) return;


	if (!IsAssaultBoosting()) {
		DoAssaultBoost(GetAssaultBoostWorldDirection());
	}
	else {
		StopAssaultBoost();
	}
}

void APlayerRobot::TryCreateDebugHUD()
{
	if (IsValid(DebugWidget) || bStatsReady == false) return;


	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (!IsValid(PlayerController) || PlayerController->IsLocalPlayerController() == false) return;
	

	if (!IsValid(DebugWidgetClassToCreate.Get())) {
		UE_LOG(LogRobotActionProject, Warning, TEXT("DebugWidgetClassToCreate is not assigned."));
		return;
	}

	DebugWidget = CreateWidget<URobotDebugWidget>(PlayerController, DebugWidgetClassToCreate);

	if (!IsValid(DebugWidget)) {
		UE_LOG(LogRobotActionProject, Error, TEXT("Failed to create the debug widget."));
		return;
	}

	if (!DebugWidget->AddToPlayerScreen()) {
		UE_LOG(LogRobotActionProject, Warning, TEXT("Failed to add the debug widget to the player screen."));

		RemoveDebugHUD();
		return;
	}

	DebugWidget->SetObservedRobot(this);
}

void APlayerRobot::RemoveDebugHUD()
{
	if (IsValid(DebugWidget)) {
		DebugWidget->SetObservedRobot(nullptr);
		DebugWidget->RemoveFromParent();
		DebugWidget = nullptr;
	}
}

void APlayerRobot::LockOnRequested()
{
	if (!IsValid(LockOnComponent)) return;
	if (IsDead()) return;

	LockOnComponent->ToggleLockOn();

}

void APlayerRobot::SwitchTargetLeftRequested()
{
	if (!IsValid(LockOnComponent)) return;
	if (IsDead()) return;

	LockOnComponent->TrySwitchTarget(ESwitchDirection::Left);
}

void APlayerRobot::SwitchTargetRightRequested()
{
	if (!IsValid(LockOnComponent)) return;
	if (IsDead()) return;

	LockOnComponent->TrySwitchTarget(ESwitchDirection::Right);
}

void APlayerRobot::UpdateLockOnCamera(float DeltaTime)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!IsValid(PlayerController) || !IsValid(LockOnComponent) || !IsValid(FollowCamera)) return;

	ARobotCharacterBase* Target = LockOnComponent->GetCurrentTarget();
	if (Target == nullptr) return;

	const FVector ToTarget = Target->GetActorLocation() - FollowCamera->GetComponentLocation();
	if (ToTarget.IsNearlyZero()) return;


	const FRotator TargetRotation = ToTarget.Rotation();
	const FRotator CurrentRotation = PlayerController->GetControlRotation();

	const FRotator NextRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, LockOnCameraTracingSpeed);

	PlayerController->SetControlRotation(NextRotation);

}

void APlayerRobot::LeftHandWeaponFireRequested()
{
	UE_LOG(LogRobotActionProject, Display, TEXT("LH Fire!"));

	if (IsDead() || !IsValid(FollowCamera)) return;

	UWorld* World = GetWorld();
	if (!IsValid(World)) return;

	UProjectileSubsystem* Projectiles = World->GetSubsystem<UProjectileSubsystem>();
	if (!IsValid(Projectiles)) return;

	FProjectileSpawnParams Params;
	Params.StartLocation = GetActorLocation() + FVector(0.0, 0.0, 50.0);
	Params.Direction = FollowCamera->GetForwardVector();
	Params.Owner = this;

	if (!Projectiles->TrySpawnProjectile(Params))
	{
		UE_LOG(LogRobotActionProject, Warning, TEXT("Projectile spawn failed."));
	}

}

void APlayerRobot::RightHandWeaponFireRequested()
{
	UE_LOG(LogRobotActionProject, Display, TEXT("RH Fire!"));

}

FVector APlayerRobot::GetQuickBoostWorldDirection() const
{
	if (!IsValid(GetController())) {
		return GetActorForwardVector().GetSafeNormal2D();
	}

	const FRotator Rotation = GetController()->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);


	FVector Direction = ForwardDirection * CurrentMoveInput.Y + RightDirection * CurrentMoveInput.X;
	Direction.Z = 0.0f;

	if (Direction.IsNearlyZero()) {
		Direction = ForwardDirection;
	}

	return Direction.GetSafeNormal();
}

FVector APlayerRobot::GetAssaultBoostWorldDirection() const
{
	if (!IsValid(GetController())) {
		return GetActorForwardVector().GetSafeNormal();
	}

	return GetController()->GetControlRotation().Vector().GetSafeNormal();
}
