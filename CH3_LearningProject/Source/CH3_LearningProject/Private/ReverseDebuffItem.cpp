// Fill out your copyright notice in the Description page of Project Settings.


#include "ReverseDebuffItem.h"
#include "SpartaCharacter.h"

AReverseDebuffItem::AReverseDebuffItem()
{
	DebuffDuration = 3.0f;
	ItemType = "ReverseDebuff";
}

void AReverseDebuffItem::ActivateItem(AActor* Activator)
{
	Super::ActivateItem(Activator);
	if (Activator && Activator->ActorHasTag("Player"))
	{
		DestroyItem();
	}
}

void AReverseDebuffItem::StartDebuff(AActor* Activator)
{
	Super::StartDebuff(Activator);
	ASpartaCharacter* Player = Cast<ASpartaCharacter>(Activator);
	if (Player)
	{
		Player->StartReverseDebuff(DebuffDuration);
	}
}

void AReverseDebuffItem::OnDebuffTimeUp()
{
}
