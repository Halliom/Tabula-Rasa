// Fill out your copyright notice in the Description page of Project Settings.

#include "TabaluRasa.h"

#include "ChunkManager.h"
#include "DefaultPlayer.h"

#include "TabaluRasaGameMode.h"

ATabaluRasaGameMode::ATabaluRasaGameMode(const FObjectInitializer& ObjectInitiailizer)
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Character/FirstPerson"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
}

void ATabaluRasaGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Spawns the initial chunk manager to handle all of the chunk
	GetWorld()->SpawnActor<AChunkManager>();
}
