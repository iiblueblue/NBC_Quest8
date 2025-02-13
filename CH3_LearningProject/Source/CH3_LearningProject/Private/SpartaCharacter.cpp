// Fill out your copyright notice in the Description page of Project Settings.


#include "SpartaCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SpartaPlayerController.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/WidgetComponent.h"
#include "SpartaGameState.h"
#include "Components/TextBlock.h"

ASpartaCharacter::ASpartaCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(GetMesh());
	OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);

	NormalSpeed = 600.0f;
	SprintSpeedMultiplier = 1.7f;
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;
	SlowSpeed = NormalSpeed * 0.3;

	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;

	MaxHealth = 100.0f;
	Health = MaxHealth;
	bIsSlowState = false;
	bIsReverseState = false;
	ReverseDirection = 1.0f;
}

float ASpartaCharacter::GetHealth() const
{
	return Health;
}

void ASpartaCharacter::AddHealth(float Amount)
{
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	UpdateOverheadHP();
}

void ASpartaCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateOverheadHP();
}

void ASpartaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ASpartaPlayerController* PlayerController = Cast<ASpartaPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::Move
				);
			}

			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::StartJump
				);
			}

			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Completed,
					this,
					&ASpartaCharacter::StopJump
				);
			}

			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::Look
				);
			}

			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::StartSprint
				);
			}

			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Completed,
					this,
					&ASpartaCharacter::StopSprint
				);
			}
		}
	}
}

float ASpartaCharacter::TakeDamage(
	float DamageAmount, // 들어온 데미지(방어 적용 전)
	FDamageEvent const& DamageEvent, 
	AController* EventInstigator, 
	AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser); // 실제로 내가 받은 데미지(방어 적용 후)

	Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
	UpdateOverheadHP();

	if (Health <= 0.0f)
	{
		OnDeath();
	}

	return ActualDamage;
}

void ASpartaCharacter::Move(const FInputActionValue& Value)
{
	if (!Controller) return;

	const FVector2D MoveInput = Value.Get<FVector2D>();

	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(GetActorForwardVector(), MoveInput.X* ReverseDirection);
	}
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(GetActorRightVector(), MoveInput.Y* ReverseDirection);
	}
}

void ASpartaCharacter::StartJump(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		Jump();
	}
}

void ASpartaCharacter::StopJump(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		StopJumping();
	}
}

void ASpartaCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookInput = Value.Get<FVector2D>();

	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

void ASpartaCharacter::StartSprint(const FInputActionValue& Value)
{
	if (GetCharacterMovement())
	{
		if (!bIsSlowState)
		{
			GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		}
	}
}

void ASpartaCharacter::StopSprint(const FInputActionValue& Value)
{
	if (GetCharacterMovement())
	{
		if (!bIsSlowState)
		{
			GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
		}
	}
}

void ASpartaCharacter::OnDeath()
{
	// 게임 종료 로직
	ASpartaGameState* SpartaGameState = GetWorld() ? GetWorld()->GetGameState<ASpartaGameState>() : nullptr;
	if (SpartaGameState)
	{
		SpartaGameState->OnGameOver();
	}
}

void ASpartaCharacter::UpdateOverheadHP()
{
	if (!OverheadWidget) return;

	UUserWidget* OverHeadWidgetInstance = OverheadWidget->GetUserWidgetObject();
	if (!OverHeadWidgetInstance) return;

	if (UTextBlock* HPText = Cast<UTextBlock>(OverHeadWidgetInstance->GetWidgetFromName(TEXT("OverHeadHP"))))
	{
		HPText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth)));
	}
}

void ASpartaCharacter::UpdateSlowDebuffTime()
{
	float RemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(SlowDebuffTimerHandle);
	
	if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(GetController()))
	{
		SpartaPlayerController->UpdateDebuffTime("SlowDebuff", RemainingTime);
	}
}

void ASpartaCharacter::UpdateReverseDebuffTime()
{
	float RemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(ReverseDebuffTimerHandle);

	if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(GetController()))
	{
		SpartaPlayerController->UpdateDebuffTime("ReverseDebuff", RemainingTime);
	}
}

void ASpartaCharacter::StartSlowDebuff(float DebuffDuration)
{
	// 이미 Slow 상태라면
	if (bIsSlowState)
	{
		// 타이머 초기화 후 진행
		GetWorld()->GetTimerManager().ClearTimer(SlowDebuffTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(SlowDebuffUpdateTimerHandle);
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(GetController()))
		{
			SpartaPlayerController->UpdateDebuffTime("SlowDebuff", DebuffDuration);
		}
	}
	else
	{
		// UI 표시
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(GetController()))
		{
			SpartaPlayerController->AddDebuffWidget("SlowDebuff", DebuffDuration);
		}
	}

	// Slow 효과
	bIsSlowState = true;
	GetCharacterMovement()->MaxWalkSpeed = SlowSpeed;

	// 타이머 실행
	GetWorld()->GetTimerManager().SetTimer(
		SlowDebuffTimerHandle,
		this,
		&ASpartaCharacter::EndSlowDebuff,
		DebuffDuration,
		false
	);

	GetWorld()->GetTimerManager().SetTimer(
		SlowDebuffUpdateTimerHandle,
		this,
		&ASpartaCharacter::UpdateSlowDebuffTime,
		1.0f,
		true
	);
}

void ASpartaCharacter::StartReverseDebuff(float DebuffDuration)
{
	if (bIsReverseState)
	{
		GetWorld()->GetTimerManager().ClearTimer(ReverseDebuffTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(ReverseDebuffUpdateTimerHandle);
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(GetController()))
		{
			SpartaPlayerController->UpdateDebuffTime("ReverseDebuff", DebuffDuration);
		}
	}
	else
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(GetController()))
		{
			SpartaPlayerController->AddDebuffWidget("ReverseDebuff", DebuffDuration);
		}
	}

	bIsReverseState = true;
	ReverseDirection = -1.0f;

	GetWorld()->GetTimerManager().SetTimer(
		ReverseDebuffTimerHandle,
		this,
		&ASpartaCharacter::EndReverseDebuff,
		DebuffDuration,
		false
	);

	GetWorld()->GetTimerManager().SetTimer(
		ReverseDebuffUpdateTimerHandle,
		this,
		&ASpartaCharacter::UpdateReverseDebuffTime,
		1.0f,
		true
	);
}

void ASpartaCharacter::EndSlowDebuff()
{
	bIsSlowState = false;
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(GetController()))
	{
		SpartaPlayerController->RemoveDebuffWidget("SlowDebuff");
	}
	GetWorld()->GetTimerManager().ClearTimer(SlowDebuffTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(SlowDebuffUpdateTimerHandle);
}

void ASpartaCharacter::EndReverseDebuff()
{
	bIsReverseState = false;
	ReverseDirection = 1.0f;
	if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(GetController()))
	{
		SpartaPlayerController->RemoveDebuffWidget("ReverseDebuff");
	}
	GetWorld()->GetTimerManager().ClearTimer(ReverseDebuffTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(ReverseDebuffUpdateTimerHandle);
}
