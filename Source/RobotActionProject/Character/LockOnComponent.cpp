// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/LockOnComponent.h"

#include "Character/RobotCharacterBase.h"
#include "GameFramework/PlayerController.h"
#include "SceneView.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RobotActionProject.h" //LogRobotActionProject


// Sets default values for this component's properties
ULockOnComponent::ULockOnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void ULockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void ULockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	if (IsLockedOn()) {
		ARobotCharacterBase* Target = CurrentTarget.Get();
		if (!CheckLineSightToTarget(Target)) {
			OccludedElapsedTime += DeltaTime;

			if (OccludedElapsedTime >= OcclusionGracePeriod) {
				ClearLockOn();
				return;
			}
		}
		else {
			OccludedElapsedTime = 0.0f;
		}

		ARobotCharacterBase* OwnerRobot = Cast<ARobotCharacterBase>(GetOwner());
		if (!IsValid(OwnerRobot) || OwnerRobot->IsDead()) return;

		const double DistanceCm = FVector::Dist(OwnerRobot->GetActorLocation(), Target->GetActorLocation());
		if (DistanceCm > MaxMaintainDistance) ClearLockOn();

	}

}

void ULockOnComponent::ToggleLockOn()
{
	if (IsLockedOn())	ClearLockOn();
	else				TryLockOn();
}

bool ULockOnComponent::TryLockOn()
{
	ARobotCharacterBase* OwnerRobot = Cast<ARobotCharacterBase>(GetOwner());
	if (!IsValid(OwnerRobot) || OwnerRobot->IsDead()) return false;

	if (IsLockedOn()) return true;

	CurrentTarget.Reset();

	TArray<ARobotCharacterBase*> Candidates;
	GatherLockOnCandidates(Candidates);

	const FVector OwnerLocation = OwnerRobot->GetActorLocation();
	ARobotCharacterBase* BestTarget = nullptr;
	double BestWorldDistanceSquared = TNumericLimits<double>::Max();

	for (ARobotCharacterBase* Target : Candidates)
	{
		const double WorldDistanceSquared = FVector::DistSquared(OwnerLocation, Target->GetActorLocation());
		if (WorldDistanceSquared < BestWorldDistanceSquared)
		{
			BestTarget = Target;
			BestWorldDistanceSquared = WorldDistanceSquared;
		}

	}

	if (!IsValid(BestTarget)) return false;

	CurrentTarget = BestTarget;
	return true;

}

void ULockOnComponent::GatherLockOnCandidates(TArray<ARobotCharacterBase*>& OutCandidates) const
{
	OutCandidates.Reset();

	ARobotCharacterBase* OwnerRobot = Cast<ARobotCharacterBase>(GetOwner());
	if (!IsValid(OwnerRobot) || OwnerRobot->IsDead()) return;

	APlayerController* PlayerController = Cast<APlayerController>(OwnerRobot->GetController());
	if (!IsValid(PlayerController) || MaxAcquireDistance <= 0.0f) return;

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!IsValid(LocalPlayer) || !IsValid(LocalPlayer->ViewportClient)) return;

	FViewport* Viewport = LocalPlayer->ViewportClient->Viewport;
	if (Viewport == nullptr) return;

	FSceneViewProjectionData ProjectionData;
	if (!LocalPlayer->GetProjectionData(Viewport, ProjectionData)) return;

	const FIntRect ViewRect = ProjectionData.GetConstrainedViewRect();
	if (ViewRect.Width() <= 0 || ViewRect.Height() <= 0) return;

	const FMatrix ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
	const FVector OwnerLocation = OwnerRobot->GetActorLocation();
	const double MaxDistanceSquared = FMath::Square(static_cast<double>(MaxAcquireDistance));

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnerRobot);

	TArray<AActor*> Candidates;
	// UKismetSystemLibrary::SphereOverlapActors(context object for search world, sphere pos, sphere radius, object type to collect, class to collect, actors to ignore, output) 
	UKismetSystemLibrary::SphereOverlapActors(
		this, OwnerLocation, MaxAcquireDistance, ObjectTypes,
		ARobotCharacterBase::StaticClass(), ActorsToIgnore, Candidates
	);

	for (AActor* Candidate : Candidates) {
		ARobotCharacterBase* Target = Cast<ARobotCharacterBase>(Candidate);
		if (!IsValid(Target) || Target == OwnerRobot || Target->IsDead()
			|| !Target->ActorHasTag(FName(TEXT("LockOnTarget")))) continue; // tagging condition for dummy. òÁª»£¡

		const FVector TargetLocation = Target->GetActorLocation();
		const double WorldDistanceSquared = FVector::DistSquared(OwnerLocation, TargetLocation);
		if (WorldDistanceSquared > MaxDistanceSquared) continue;

		FVector2D ScreenLocation;
		if (!FSceneView::ProjectWorldToScreen(TargetLocation, ViewRect, ViewProjectionMatrix, ScreenLocation)) continue;

		const FVector2D NormalizedLocation((ScreenLocation.X - ViewRect.Min.X) / ViewRect.Width(), (ScreenLocation.Y - ViewRect.Min.Y) / ViewRect.Height());
		if (NormalizedLocation.ContainsNaN() || NormalizedLocation.X < 0.0 || NormalizedLocation.X > 1.0 || NormalizedLocation.Y < 0.0 || NormalizedLocation.Y > 1.0) continue;

		// more condition.
		if (!CheckLineSightToTarget(Target)) continue;
		/////

		OutCandidates.Add(Target);
	}
}

bool ULockOnComponent::CheckLineSightToTarget(const ARobotCharacterBase* Target) const
{
	const ARobotCharacterBase* OwnerRobot = Cast<ARobotCharacterBase>(GetOwner());
	UWorld* World = GetWorld();
	if (!IsValid(OwnerRobot) || !IsValid(Target) || !IsValid(World)) return false;

	const APlayerController* PlayerController = Cast<APlayerController>(OwnerRobot->GetController());
	if (!IsValid(PlayerController)) return false;

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RobotLockOnLineSight), false);
	QueryParams.AddIgnoredActor(OwnerRobot);

	FHitResult HitResult;
	// UWorld::LineTraceSingleByChanne
	const bool bBlockingHit = World->LineTraceSingleByChannel(HitResult, CameraLocation, Target->GetActorLocation(), ECC_Visibility, QueryParams);


	return (!bBlockingHit || HitResult.GetActor() == Target);

}

void ULockOnComponent::ClearLockOn()
{
	CurrentTarget = nullptr;
	OccludedElapsedTime = 0.0f;
}

bool ULockOnComponent::IsLockedOn() const
{
	return (GetCurrentTarget() != nullptr);

}

ARobotCharacterBase* ULockOnComponent::GetCurrentTarget() const
{
	ARobotCharacterBase* Target = CurrentTarget.Get();

	if (!IsValid(Target) || Target->IsDead()) return nullptr;

	return Target;

}

bool ULockOnComponent::TrySwitchTarget(ESwitchDirection Direction)
{
	ARobotCharacterBase* Target = GetCurrentTarget();
	if (Target == nullptr) return false;

	ARobotCharacterBase* OwnerRobot = Cast<ARobotCharacterBase>(GetOwner());
	if (!IsValid(OwnerRobot) || OwnerRobot->IsDead()) return false;

	APlayerController* PlayerController = Cast<APlayerController>(OwnerRobot->GetController());
	if (!IsValid(PlayerController)) return false;

	if (Direction != ESwitchDirection::Left && Direction != ESwitchDirection::Right) return false;

	FVector2D TargetScreenLocation;
	if (!PlayerController->ProjectWorldLocationToScreen(Target->GetActorLocation(), TargetScreenLocation, true) || TargetScreenLocation.ContainsNaN()) return false;
    
	TArray<ARobotCharacterBase*> Candidates;
	GatherLockOnCandidates(Candidates);

	ARobotCharacterBase* NextTarget = nullptr;
	double BestDistance = TNumericLimits<double>::Max();

	ARobotCharacterBase* WrapTarget = nullptr;
	double WrapScreenX = (Direction == ESwitchDirection::Right ? TNumericLimits<double>::Max() : -TNumericLimits<double>::Max());


	for (ARobotCharacterBase* Candidate : Candidates)
	{
		if (!IsValid(Candidate) || Candidate == Target) continue;

		FVector2D CandidateScreenLocation;
		if (!PlayerController->ProjectWorldLocationToScreen(Candidate->GetActorLocation(), CandidateScreenLocation, true) || CandidateScreenLocation.ContainsNaN()) continue;

		const bool bCloserToWrapEdge = (Direction == ESwitchDirection::Right ? CandidateScreenLocation.X < WrapScreenX : CandidateScreenLocation.X > WrapScreenX);
		if (bCloserToWrapEdge)
		{
			WrapTarget = Candidate;
			WrapScreenX = CandidateScreenLocation.X;
		}

		const double Offset = CandidateScreenLocation.X - TargetScreenLocation.X;
		if (Direction == ESwitchDirection::Right && Offset <= 0.0) continue;
		if (Direction == ESwitchDirection::Left && Offset >= 0.0) continue;

		const double Distance = FMath::Abs(Offset);
		if (Distance < BestDistance)
		{
			NextTarget = Candidate;
			BestDistance = Distance;
		}
	}

	if (!IsValid(NextTarget))
	{
		NextTarget = WrapTarget;
	}

	if (!IsValid(NextTarget)) return false;

	CurrentTarget = NextTarget;
	OccludedElapsedTime = 0.0f;
	return true;

}


