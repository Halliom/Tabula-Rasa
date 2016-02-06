// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "TabaluRasaGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TABALURASA_API ATabaluRasaGameMode : public AGameMode
{
	GENERATED_BODY()
public:

	ATabaluRasaGameMode(const FObjectInitializer& ObjectInitiailizer);

	virtual void BeginPlay() override;
	
};
