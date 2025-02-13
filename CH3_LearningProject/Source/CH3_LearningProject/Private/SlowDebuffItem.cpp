// Fill out your copyright notice in the Description page of Project Settings.


#include "SlowDebuffItem.h"
#include "SpartaCharacter.h"

ASlowDebuffItem::ASlowDebuffItem()
{
	DebuffDuration = 5.0f;
	ItemType = "SlowDebuff";
}

void ASlowDebuffItem::ActivateItem(AActor* Activator)
{
	Super::ActivateItem(Activator);
	if (Activator && Activator->ActorHasTag("Player"))
	{
		DestroyItem();
	}
}

void ASlowDebuffItem::StartDebuff(AActor* Activator)
{
	Super::StartDebuff(Activator);
	ASpartaCharacter* Player = Cast<ASpartaCharacter>(Activator);
	if (Player)
	{
		Player->StartSlowDebuff(DebuffDuration);
	}
}

void ASlowDebuffItem::OnDebuffTimeUp()
{
	Super::OnDebuffTimeUp();
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("SlowDebuff End!!!!")));
}
