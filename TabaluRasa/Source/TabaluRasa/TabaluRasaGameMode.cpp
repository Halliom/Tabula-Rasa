// Fill out your copyright notice in the Description page of Project Settings.

#include "TabaluRasa.h"

#include "ChunkManager.h"

#include "TabaluRasaGameMode.h"

void ATabaluRasaGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Spawns the initial chunk manager to handle all of the chunk
	GetWorld()->SpawnActor<AChunkManager>();
}
