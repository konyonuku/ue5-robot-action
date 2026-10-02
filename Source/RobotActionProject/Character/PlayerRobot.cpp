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
#include "Character/RobotMovementComponent.h"


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

	if (IsDead() || !IsAssaultBoosting() || GetController() == nullptr) {
		return;
	}

	if (URobotMovementComponent* MoveComp = Cast<URobotMovementComponent>(GetCharacterMovement())) {
		// Keep the desired flight direction aligned with the current look rotation.
		MoveComp->SetAssaultBoostDirection(GetAssaultBoostWorldDirection());
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
	if (EnhancedInput == nullptr) {
		UE_LOG(LogRobotActionProject, Error, TEXT("PlayerRobot needs an Enhanced Input(EnhancedInputComponent* == nullptr)"));
		return;
	}

	//Bind Move
	if (MoveAction != nullptr) {
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerRobot::Move);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &APlayerRobot::MoveEnded);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Canceled, this, &APlayerRobot::MoveEnded);
	}
	else {
		UE_LOG(LogRobotActionProject, Warning, TEXT("MoveAction is not assigned(MoveAction == nullptr)"));
	}

	//Bind Look
	if (LookAction != nullptr) {
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerRobot::Look);
	}
	else {
		UE_LOG(LogRobotActionProject, Warning, TEXT("LookAction is not assigned(LookAction == nullptr)"));
	}

	//Bind MouseLook
	if (MouseLookAction != nullptr) {
		EnhancedInput->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &APlayerRobot::Look);
	}
	else {
		UE_LOG(LogRobotActionProject, Warning, TEXT("MouseLookAction is not assigned(MouseLookAction == nullptr)"));
	}
	
	//Bind Jump
	if (JumpAction != nullptr) {
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerRobot::JumpStarted);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayerRobot::JumpEnded);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Canceled, this, &APlayerRobot::JumpEnded);
	}
	else {
		UE_LOG(LogRobotActionProject, Warning, TEXT("JumpAction is not assigned(JumpAction == nullptr)"));
	}

	//Bind QuickBoost
	if (QuickBoostAction != nullptr) {
		EnhancedInput->BindAction(QuickBoostAction, ETriggerEvent::Started, this, &APlayerRobot::QuickBoostRequested);
	}
	else {
		UE_LOG(LogRobotActionProject, Warning, TEXT("QuickBoostAction is not assigned(QuickBoostAction == nullptr)"));
	}

	//Bind AssaultBoost
	if (AssaultBoostAction != nullptr) {
		EnhancedInput->BindAction(AssaultBoostAction, ETriggerEvent::Started, this, &APlayerRobot::AssaultBoostRequested);
	}
	else {
		UE_LOG(LogRobotActionProject, Warning, TEXT("AssaultBoostAction is not assigned(AssaultBoostAction == nullptr)"));
	}
	
}

// Unreal Engine: Left-handed + Z-up -> Forward: +x, Right: +y, Up: +z
void APlayerRobot::Move(const FInputActionValue& Value)
{
	if (IsDead() || GetController() == nullptr) {
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>(); //(Right, Forward)
	CurrentMoveInput = MovementVector;


	const URobotMovementComponent* MoveComp = Cast<URobotMovementComponent>(GetCharacterMovement());

	if (IsAssaultBoosting() || (MoveComp != nullptr && MoveComp->IsQuickBoost())) return;


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
	if (GetController() != nullptr)
	{
		FVector2D LookAxisVector = Value.Get<FVector2D>();

		AddControllerYawInput(LookAxisVector.X);   //Yaw
		AddControllerPitchInput(LookAxisVector.Y); //Pitch
	}
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
	if (IsDead()) {
		return;
	}

	DoQuickBoost(GetQuickBoostWorldDirection());
}

void APlayerRobot::AssaultBoostRequested()
{
	if (IsDead()) {
		return;
	}

	if (!IsAssaultBoosting()) {
		DoAssaultBoost(GetAssaultBoostWorldDirection());
	}
	else {
		StopAssaultBoost();
	}
}

void APlayerRobot::TryCreateDebugHUD()
{
	if (DebugWidget != nullptr || bStatsReady == false) {
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController == nullptr || PlayerController->IsLocalPlayerController() == false) {
		return;
	}

	if (DebugWidgetClassToCreate == nullptr) {
		UE_LOG(LogRobotActionProject, Warning, TEXT("DebugWidgetClassToCreate is not assigned."));
		return;
	}

	DebugWidget = CreateWidget<URobotDebugWidget>(PlayerController, DebugWidgetClassToCreate);

	if (DebugWidget == nullptr) {
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
	if (DebugWidget) {
		DebugWidget->SetObservedRobot(nullptr);
		DebugWidget->RemoveFromParent();
		DebugWidget = nullptr;
	}
}

FVector APlayerRobot::GetQuickBoostWorldDirection() const
{
	if (GetController() == nullptr) {
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
	if (GetController() == nullptr) {
		return GetActorForwardVector().GetSafeNormal();
	}

	return GetController()->GetControlRotation().Vector().GetSafeNormal();
}
