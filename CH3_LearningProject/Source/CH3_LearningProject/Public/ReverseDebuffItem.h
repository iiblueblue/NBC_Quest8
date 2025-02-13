// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DebuffItem.h"
#include "ReverseDebuffItem.generated.h"

/**
 * 
 */
UCLASS()
class CH3_LEARNINGPROJECT_API AReverseDebuffItem : public ADebuffItem
{
	GENERATED_BODY()
public:
	AReverseDebuffItem();

	FTimerHandle ReverseDebuffTimerHandle;

	virtual void ActivateItem(AActor* Activator) override;
	void StartDebuff(AActor* Activator) override;
	void OnDebuffTimeUp() override;
	
};
