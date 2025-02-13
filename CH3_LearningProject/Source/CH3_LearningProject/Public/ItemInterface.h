// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ItemInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UItemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CH3_LEARNINGPROJECT_API IItemInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION()
	virtual void OnItemOverlap(
		UPrimitiveComponent* OverlappedComp, // 오버랩이 발생한 자기자신(Sphere Component)
		AActor* OtherActor, // 부딪힌 상대방의 액터(Player)
		UPrimitiveComponent* OtherComp, // 부딪힌 상대의 컴포넌트(Capshule Component)
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) = 0;
	UFUNCTION()
	virtual void OnItemEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex) = 0;
	virtual void ActivateItem(AActor* Activator) = 0;
	virtual FName GetItemType() const = 0;

	// 지뢰, 힐링, 코인
	// 힐링, 코인 - 즉시 발동형 - 오버랩
	// 지뢰 - 범위 내 오버랩 - 발동이 5초뒤 폭발 - 오버랩 - 데미지
};
