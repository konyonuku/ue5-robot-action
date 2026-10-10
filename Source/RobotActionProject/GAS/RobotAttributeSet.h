// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "RobotAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class ROBOTACTIONPROJECT_API URobotAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayAttributeData HP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayAttributeData EN;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayAttributeData ACS;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayAttributeData MaxHP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayAttributeData MaxEN;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayAttributeData MaxACS;


	ATTRIBUTE_ACCESSORS_BASIC(URobotAttributeSet, HP)
	ATTRIBUTE_ACCESSORS_BASIC(URobotAttributeSet, MaxHP)

	ATTRIBUTE_ACCESSORS_BASIC(URobotAttributeSet, EN)
	ATTRIBUTE_ACCESSORS_BASIC(URobotAttributeSet, MaxEN)

	ATTRIBUTE_ACCESSORS_BASIC(URobotAttributeSet, ACS)
	ATTRIBUTE_ACCESSORS_BASIC(URobotAttributeSet, MaxACS)


protected:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

private:
	void ClampAttributeValue(const FGameplayAttribute& Attribute, float& NewValue) const;

};
