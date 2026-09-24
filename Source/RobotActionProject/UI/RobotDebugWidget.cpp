#include "UI/RobotDebugWidget.h"

#include "Character/RobotCharacterBase.h"
#include "Components/TextBlock.h"

void URobotDebugWidget::SetObservedRobot(ARobotCharacterBase* NewRobot)
{
	UnbindRobot(ObservedRobot.Get());
	ObservedRobot.Reset();

	if (IsValid(NewRobot) && !NewRobot->IsActorBeingDestroyed())
	{
		ObservedRobot = NewRobot;
		NewRobot->OnHPChanged.AddUniqueDynamic(this, &URobotDebugWidget::HandleHPChanged);
		NewRobot->OnENChanged.AddUniqueDynamic(this, &URobotDebugWidget::HandleENChanged);
		NewRobot->OnACSChanged.AddUniqueDynamic(this, &URobotDebugWidget::HandleACSChanged);
		NewRobot->OnDied.AddUniqueDynamic(this, &URobotDebugWidget::HandleDied);
		NewRobot->OnEndPlay.AddUniqueDynamic(this, &URobotDebugWidget::HandleRobotEndPlay);
	}

	// Initial state is a snapshot, not a replay of earlier change notifications.
	RefreshDisplay();
}

void URobotDebugWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshDisplay();
}

void URobotDebugWidget::NativeDestruct()
{
	SetObservedRobot(nullptr);
	Super::NativeDestruct();
}

void URobotDebugWidget::UnbindRobot(ARobotCharacterBase* Robot)
{
	if (!Robot) return;

	Robot->OnHPChanged.RemoveDynamic(this, &URobotDebugWidget::HandleHPChanged);
	Robot->OnENChanged.RemoveDynamic(this, &URobotDebugWidget::HandleENChanged);
	Robot->OnACSChanged.RemoveDynamic(this, &URobotDebugWidget::HandleACSChanged);
	Robot->OnDied.RemoveDynamic(this, &URobotDebugWidget::HandleDied);
	Robot->OnEndPlay.RemoveDynamic(this, &URobotDebugWidget::HandleRobotEndPlay);
}

void URobotDebugWidget::RefreshDisplay()
{
	const ARobotCharacterBase* Robot = ObservedRobot.Get();
	if (!Robot)
	{
		ShowNoTarget();
		return;
	}

	SetStatText(HPText, TEXT("HP"), Robot->GetHP(), Robot->GetMaxHP());
	SetStatText(ENText, TEXT("EN"), Robot->GetEN(), Robot->GetMaxEN());
	SetStatText(ACSText, TEXT("ACS"), Robot->GetACS(), Robot->GetMaxACS());
	RefreshState();
}

void URobotDebugWidget::RefreshState()
{
	if (StateText)
	{
		const ARobotCharacterBase* Robot = ObservedRobot.Get();
		StateText->SetText(FText::FromString(Robot ? (Robot->IsDead() ? TEXT("Dead") : TEXT("Alive")) : TEXT("No Target")));
	}
}

void URobotDebugWidget::ShowNoTarget()
{
	if (HPText)
	{
		HPText->SetText(FText::FromString(TEXT("HP --")));
	}
	if (ENText)
	{
		ENText->SetText(FText::FromString(TEXT("EN --")));
	}
	if (ACSText)
	{
		ACSText->SetText(FText::FromString(TEXT("ACS --")));
	}
	if (StateText)
	{
		StateText->SetText(FText::FromString(TEXT("No Target")));
	}
}

void URobotDebugWidget::SetStatText(UTextBlock* TextBlock, const TCHAR* Label, float CurrentValue, float MaxValue)
{
	if (TextBlock)
	{
		// Preserve fractional values while keeping whole values compact.
		TextBlock->SetText(FText::FromString(FString::Printf(TEXT("%s %s / %s"), Label,
			*FText::AsNumber(CurrentValue).ToString(), *FText::AsNumber(MaxValue).ToString())));
	}
}

void URobotDebugWidget::HandleHPChanged(float CurrentValue, float MaxValue)
{
	SetStatText(HPText, TEXT("HP"), CurrentValue, MaxValue);
}

void URobotDebugWidget::HandleENChanged(float CurrentValue, float MaxValue)
{
	SetStatText(ENText, TEXT("EN"), CurrentValue, MaxValue);
}

void URobotDebugWidget::HandleACSChanged(float CurrentValue, float MaxValue)
{
	SetStatText(ACSText, TEXT("ACS"), CurrentValue, MaxValue);
}

void URobotDebugWidget::HandleDied()
{
	RefreshState();
}

void URobotDebugWidget::HandleRobotEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	// Use the callback argument even if the weak reference is no longer valid.
	UnbindRobot(Cast<ARobotCharacterBase>(Actor));
	ObservedRobot.Reset();
	ShowNoTarget();
}
