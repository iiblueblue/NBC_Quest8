// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "DebuffItem.generated.h"

/**
 * 
 */
UCLASS()
class CH3_LEARNINGPROJECT_API ADebuffItem : public ABaseItem
{
	GENERATED_BODY()
public:
	ADebuffItem();
	
	float DebuffDuration;

	virtual void ActivateItem(AActor* Activator) override;

	virtual void StartDebuff(AActor* Activator);
	virtual void OnDebuffTimeUp();
};
