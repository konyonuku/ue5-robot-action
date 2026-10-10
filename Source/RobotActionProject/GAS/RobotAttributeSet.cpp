// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/RobotAttributeSet.h"


void URobotAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) 
{
    Super::PreAttributeChange(Attribute, NewValue);
    ClampAttributeValue(Attribute, NewValue);

}

void URobotAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
    Super::PreAttributeBaseChange(Attribute, NewValue);
    ClampAttributeValue(Attribute, NewValue);

}

void URobotAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);

    if (Attribute == GetMaxHPAttribute() && GetHP() > NewValue)
    {
        SetHP(NewValue);
    }
    else if (Attribute == GetMaxENAttribute() && GetEN() > NewValue)
    {
        SetEN(NewValue);
    }
    else if (Attribute == GetMaxACSAttribute() && GetACS() > NewValue)
    {
        SetACS(NewValue);
    }

}

void URobotAttributeSet::ClampAttributeValue(const FGameplayAttribute& Attribute, float& NewValue) const
{
    if (Attribute == GetMaxHPAttribute()
        || Attribute == GetMaxENAttribute()
        || Attribute == GetMaxACSAttribute())
    {
        NewValue = FMath::Max(NewValue, 1.0f);
    }
    else if (Attribute == GetHPAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHP());
    }
    else if (Attribute == GetENAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxEN());
    }
    else if (Attribute == GetACSAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxACS());
    }

}
