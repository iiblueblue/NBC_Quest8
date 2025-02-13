// Fill out your copyright notice in the Description page of Project Settings.


#include "DebuffItem.h"

ADebuffItem::ADebuffItem()
{
	DebuffDuration = 0;
	ItemType = "DefaultDebuff";
}

void ADebuffItem::ActivateItem(AActor* Activator)
{
	Super::ActivateItem(Activator);
	if (Activator && Activator->ActorHasTag("Player"))
	{
		// 디버프 시작
		StartDebuff(Activator);
		
	}
}

void ADebuffItem::StartDebuff(AActor* Activator)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("SlowDebuff Start!!!!")));
}

void ADebuffItem::OnDebuffTimeUp()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("SlowDebuff End!!!!")));
}
