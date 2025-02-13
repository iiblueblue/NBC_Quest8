// Fill out your copyright notice in the Description page of Project Settings.


#include "SpartaGameState.h"
#include "SpartaGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "SpawnVolume.h"
#include "CoinItem.h"
#include "SpartaPlayerController.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Blueprint/UserWidget.h"

ASpartaGameState::ASpartaGameState()
{
	Score = 0;
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;
	LevelDuration = 30.0f;
	CurrentLevelIndex = 0;
	MaxLevels = 3;
	WaveDuration = LevelDuration / 3;
	CurrentWaveIndex = 0;
	MaxWaves = 3;
	ItemToSpawn = 40;
}

void ASpartaGameState::BeginPlay()
{
	Super::BeginPlay();

	StartLevel();

	UE_LOG(LogTemp, Warning, TEXT("HUDUpdateTimerHandle"));
	GetWorldTimerManager().SetTimer(
		HUDUpdateTimerHandle,
		this,
		&ASpartaGameState::UpdateHUD,
		0.1f,
		true
	);
}

int32 ASpartaGameState::GetScore() const
{
	return Score;
}

void ASpartaGameState::AddScore(int32 Amount)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			SpartaGameInstance->AddToScore(Amount);
		}
	}
}

void ASpartaGameState::OnGameOver()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			SpartaPlayerController->SetPause(true);
			SpartaPlayerController->ShowMainMenu(true);
		}
	}
}

void ASpartaGameState::StartLevel()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			SpartaPlayerController->ShowGameHUD();
		}
	}
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			CurrentLevelIndex = SpartaGameInstance->CurrentLevelIndex;
		}
	}

	// 변수 초기화
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;

	StartWave();

	// 타이머 시작
	UE_LOG(LogTemp, Warning, TEXT("LevelTimerHandle"));
	GetWorldTimerManager().SetTimer(
		LevelTimerHandle,
		this,
		&ASpartaGameState::OnLevelTimeUp,
		LevelDuration,
		false
	);
}

void ASpartaGameState::OnLevelTimeUp()
{
	//EndLevel();
}

void ASpartaGameState::OnCoinCollected()
{
	CollectedCoinCount++;
	UE_LOG(LogTemp, Warning, TEXT("Coin Collected: %d / %d"),
		CollectedCoinCount,
		SpawnedCoinCount);

	if (SpawnedCoinCount > 0 && CollectedCoinCount >= SpawnedCoinCount && CurrentWaveIndex >= 2)
	{
		EndLevel();
	}
}

void ASpartaGameState::EndLevel()
{
	// 타이머 초기화
	GetWorldTimerManager().ClearTimer(HUDUpdateTimerHandle);
	GetWorldTimerManager().ClearTimer(LevelTimerHandle);

	// 현재 레벨 증가
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			AddScore(Score);
			CurrentLevelIndex++;
			SpartaGameInstance->CurrentLevelIndex=CurrentLevelIndex;
		}
	}

	// 현재 레벨이 MaxLevels 보다 클 경우
	if (CurrentLevelIndex >= MaxLevels)
	{
		OnGameOver(); // 게임 오버
		return;
	}
	
	if (LevelMapNames.IsValidIndex(CurrentLevelIndex))
	{
		UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentLevelIndex]);
	}
	else
	{
		OnGameOver();
	}
}

void ASpartaGameState::UpdateHUD()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			if (UUserWidget* HUDWidget = SpartaPlayerController->GetHUDWidget())
			{
				// Time
				if (UProgressBar* TimeSlider = Cast<UProgressBar>(HUDWidget->GetWidgetFromName(TEXT("TimeBar"))))
				{
					float RemainingTime = GetWorldTimerManager().GetTimerRemaining(LevelTimerHandle);
					TimeSlider->SetPercent(RemainingTime/LevelDuration);
				}

				// Score
				if (UTextBlock* ScoreText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Score"))))
				{
					if (UGameInstance* GameInstance = GetGameInstance())
					{
						USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
						if (SpartaGameInstance)
						{
							ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), SpartaGameInstance->TotalScore)));
						}
					}
				}

				// Level
				if (UTextBlock* LevelIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Level"))))
				{
					LevelIndexText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d"), CurrentLevelIndex + 1)));
				}
			}
		}
	}
}

void ASpartaGameState::StartWave()
{
	// 웨이브 시작 알림
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("%d Wave Start!!!!"), CurrentWaveIndex + 1));

	// Wave에 따라서 스폰하는 아이템 갯수 변경
	switch (CurrentWaveIndex)
	{
	case 0:
		ItemToSpawn = 40;
		break;
	case 1:
		ItemToSpawn = 50;
		break;
	case 2:
		ItemToSpawn = 60;
		break;
	default:
		ItemToSpawn = 40;
		break;
	}

	// 아이템 스폰
	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);

	for (int32 i = 0; i < ItemToSpawn; i++)
	{
		if (FoundVolumes.Num() > 0)
		{
			ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[0]);
			if (SpawnVolume)
			{
				AActor* SpawnedActor = SpawnVolume->SpawnRandomItem();
				if (SpawnedActor && SpawnedActor->IsA(ACoinItem::StaticClass()))
				{
					SpawnedCoinCount++;
				}
			}
		}
	}

	// 웨이브 타이머 시작
	// 타이머 시작
	UE_LOG(LogTemp, Warning, TEXT("WaveTimerHandle"));
	GetWorldTimerManager().SetTimer(
		WaveTimerHandle,
		this,
		&ASpartaGameState::OnWaveTimeUp,
		WaveDuration,
		false
	);
}

void ASpartaGameState::EndWave()
{
	// CurrentWaveIndex 업데이트
	CurrentWaveIndex++;

	// 타이머 초기화
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);

	// CurrentWaveIndex가 MaxWaves가 되었다면
	if (CurrentWaveIndex == MaxWaves)
	{
		// EndLevel로
		EndLevel();
	}
	else // 아직 MaxWaves가 아니라면
	{
		StartWave();
	}
}

void ASpartaGameState::OnWaveTimeUp()
{
	// EndWave 호출
	EndWave();
}
