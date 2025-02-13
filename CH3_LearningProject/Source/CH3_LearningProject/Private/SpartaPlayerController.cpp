// Fill out your copyright notice in the Description page of Project Settings.


#include "SpartaPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "SpartaGameState.h"
#include "SpartaGameInstance.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

ASpartaPlayerController::ASpartaPlayerController()
	: InputMappingContext(nullptr),
	MoveAction(nullptr),
	JumpAction(nullptr),
	LookAction(nullptr),
	SprintAction(nullptr),
	HUDWidgetClass(nullptr),
	HUDWidgetInstance(nullptr),
	MainMenuWidgetClass(nullptr),
	MainMenuWidgetInstance(nullptr)
{

}

UUserWidget* ASpartaPlayerController::GetHUDWidget() const
{
	return HUDWidgetInstance;
}

void ASpartaPlayerController::ShowGameHUD()
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();

			bShowMouseCursor = false;
			SetInputMode(FInputModeGameOnly());
		}
		
		ASpartaGameState* SpartaGameState = GetWorld() ? GetWorld()->GetGameState<ASpartaGameState>() : nullptr;
		if (SpartaGameState)
		{
			SpartaGameState->UpdateHUD();
		}
	}
}

void ASpartaPlayerController::ShowMainMenu(bool bIsRestart)
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	if (MainMenuWidgetClass)
	{
		MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->AddToViewport();

			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());
		}

		if (UTextBlock* StartButtonText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("StartButtonText"))))
		{
			if (bIsRestart) // 게임 오버 상태
			{
				StartButtonText->SetText(FText::FromString(TEXT("Restart")));
				
			}
			else
			{
				StartButtonText->SetText(FText::FromString(TEXT("Start")));
			}
		}

		if (UTextBlock* ExitButtonText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("BackButtonText"))))
		{
			if (bIsRestart) // 게임 오버 상태
			{
				ExitButtonText->SetText(FText::FromString(TEXT("Back To Main")));

			}
			else
			{
				ExitButtonText->SetText(FText::FromString(TEXT("Exit")));
			}
		}

		// 애니메이션 실행
		if (bIsRestart)
		{
			UFunction* PlayAnimFunc = MainMenuWidgetInstance->FindFunction(FName("PlayGameOverAnim"));
			if (PlayAnimFunc)
			{
				MainMenuWidgetInstance->ProcessEvent(PlayAnimFunc, nullptr);
			}

			if (UTextBlock* TotalScoreText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName("TotalScoreText")))
			{
				if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(UGameplayStatics::GetGameInstance(this)))
				{
					TotalScoreText->SetText(FText::FromString(
						FString::Printf(TEXT("Total Score: %d"), SpartaGameInstance->TotalScore)
					));
				}
			}
		}
	}
}

void ASpartaPlayerController::AddDebuffWidget(FName DebuffType, float DebuffDuration)
{
	if (DebuffType == "ReverseDebuff")
	{
		ReverseDebuffWidgetInstance = CreateWidget<UUserWidget>(this, DebuffWidgetClass);
		if (ReverseDebuffWidgetInstance)
		{
			UpdateDebuffTime("ReverseDebuff", DebuffDuration);
			if (UImage* DebuffWidgetImage = Cast<UImage>(ReverseDebuffWidgetInstance->GetWidgetFromName(TEXT("DebuffImage"))))
			{
				DebuffWidgetImage->SetColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f));
			}

			if (UHorizontalBox* HorizontalBox = Cast<UHorizontalBox>(HUDWidgetInstance->GetWidgetFromName(TEXT("DebuffBox"))))
			{
				HorizontalBox->AddChild(ReverseDebuffWidgetInstance);
			}
		}
			
	}
	else if (DebuffType == "SlowDebuff")
	{
		SlowDebuffWidgetInstance = CreateWidget<UUserWidget>(this, DebuffWidgetClass);
		if (SlowDebuffWidgetInstance)
		{
			UpdateDebuffTime("SlowDebuff", DebuffDuration);
			if (UImage* DebuffWidgetImage = Cast<UImage>(SlowDebuffWidgetInstance->GetWidgetFromName(TEXT("DebuffImage"))))
			{
				DebuffWidgetImage->SetColorAndOpacity(FLinearColor(0.0f, 0.0f, 1.0f, 1.0f));
			}

			if (UHorizontalBox* HorizontalBox = Cast<UHorizontalBox>(HUDWidgetInstance->GetWidgetFromName(TEXT("DebuffBox"))))
			{
				HorizontalBox->AddChild(SlowDebuffWidgetInstance);
			}
		}
	}
}

void ASpartaPlayerController::RemoveDebuffWidget(FName DebuffType)
{
	if (DebuffType == "SlowDebuff")
	{
		if (UHorizontalBox* HorizontalBox = Cast<UHorizontalBox>(HUDWidgetInstance->GetWidgetFromName(TEXT("DebuffBox"))))
		{
			HorizontalBox->RemoveChild(SlowDebuffWidgetInstance);
		}
	}
	else if (DebuffType == "ReverseDebuff")
	{
		if (UHorizontalBox* HorizontalBox = Cast<UHorizontalBox>(HUDWidgetInstance->GetWidgetFromName(TEXT("DebuffBox"))))
		{
			HorizontalBox->RemoveChild(ReverseDebuffWidgetInstance);
		}
	}
}

void ASpartaPlayerController::UpdateDebuffTime(FName DebuffName, float RemainingTime)
{
	if (DebuffName == "SlowDebuff")
	{
		if (SlowDebuffWidgetInstance)
		{
			if (UTextBlock* DebuffWidgetTimer = Cast<UTextBlock>(SlowDebuffWidgetInstance->GetWidgetFromName(TEXT("DebuffTimer"))))
			{
				DebuffWidgetTimer->SetText(FText::FromString(
					FString::Printf(TEXT("%d"), FMath::RoundToInt(RemainingTime))));
			}
		}
	}
	else if (DebuffName == "ReverseDebuff")
	{
		if (ReverseDebuffWidgetInstance)
		{
			if (UTextBlock* DebuffWidgetTimer = Cast<UTextBlock>(ReverseDebuffWidgetInstance->GetWidgetFromName(TEXT("DebuffTimer"))))
			{
				DebuffWidgetTimer->SetText(FText::FromString(
					FString::Printf(TEXT("%d"), FMath::RoundToInt(RemainingTime))));
			}
		}
	}
}


void ASpartaPlayerController::StartGame()
{
	if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		SpartaGameInstance->CurrentLevelIndex = 0;
		SpartaGameInstance->TotalScore = 0;
	}

	UGameplayStatics::OpenLevel(GetWorld(), FName("BasicLevel"));
	SetPause(false);
}

void ASpartaPlayerController::ExitGame()
{
	if (UTextBlock* ExitButtonText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("BackButtonText"))))
	{
		if (ExitButtonText->GetText().EqualTo(FText::FromString(TEXT("Exit")))) // 게임 오버 상태
		{
			UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
		}
		else
		{
			UGameplayStatics::OpenLevel(GetWorld(), FName("MainMenu"));
		}
	}
	
}

void ASpartaPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer()) // 현재 플레이어의 로컬 플레이어 객체를 가져옴(플레이어 입력이나 화면뷰를 관리하는 객체)
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubSystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()) // 로컬 플레이어로부터 UEnhancedInputLocalPlayerSubsystem(입력 시스템을 관리함)을 가져옴
		{
			if (InputMappingContext) // 잘 할당되어 있는지 안전코드
			{
				SubSystem->AddMappingContext(InputMappingContext, 0); // IMC를 0번 우선순위로 활성화 시켜라
			}
		}
	}

	FString CurrentMapName = GetWorld()->GetMapName();
	if (CurrentMapName.Contains("MenuLevel"))
	{
		ShowMainMenu(false);
	}
}
