#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Actor.h"
#include "RobotDebugWidget.generated.h"

class ARobotCharacterBase;
class UTextBlock;

/** Event-driven debug display. The Blueprint child supplies the layout. */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class ROBOTACTIONPROJECT_API URobotDebugWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Pass nullptr to clear the display. Call again after removing and reusing the widget. */
	UFUNCTION(BlueprintCallable, Category = "Robot|Debug")
	void SetObservedRobot(ARobotCharacterBase* NewRobot);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HPText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ENText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ACSText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StateText;

	UPROPERTY(Transient)
	TWeakObjectPtr<ARobotCharacterBase> ObservedRobot;

	void UnbindRobot(ARobotCharacterBase* Robot);
	void RefreshDisplay();
	void RefreshState();
	void ShowNoTarget();
	static void SetStatText(UTextBlock* TextBlock, const TCHAR* Label, float CurrentValue, float MaxValue);

	UFUNCTION()
	void HandleHPChanged(float CurrentValue, float MaxValue);

	UFUNCTION()
	void HandleENChanged(float CurrentValue, float MaxValue);

	UFUNCTION()
	void HandleACSChanged(float CurrentValue, float MaxValue);

	UFUNCTION()
	void HandleDied();

	UFUNCTION()
	void HandleRobotEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason);
};
