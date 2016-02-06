// Copyright Johan Wieslander

#pragma once

#include "GameFramework/Character.h"
#include "DefaultPlayer.generated.h"

UCLASS()
class TABALURASA_API ADefaultPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADefaultPlayer();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	
};
