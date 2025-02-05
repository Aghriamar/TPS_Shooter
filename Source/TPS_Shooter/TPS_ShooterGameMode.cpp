// Copyright Epic Games, Inc. All Rights Reserved.

#include "TPS_ShooterGameMode.h"
#include "TPS_ShooterPlayerController.h"
#include "TPS_ShooterCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATPS_ShooterGameMode::ATPS_ShooterGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ATPS_ShooterPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprint/Character/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}