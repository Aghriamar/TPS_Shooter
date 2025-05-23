// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TPS_ShooterGameMode.generated.h"

UCLASS(minimalapi)
class ATPS_ShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATPS_ShooterGameMode();

	void PlayerCharacterDead();
};



