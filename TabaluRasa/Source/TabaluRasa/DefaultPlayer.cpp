// Copyright Johan Wieslander

#include "TabaluRasa.h"
#include "DefaultPlayer.h"


// Sets default values
ADefaultPlayer::ADefaultPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADefaultPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADefaultPlayer::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// Called to bind functionality to input
void ADefaultPlayer::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

}

