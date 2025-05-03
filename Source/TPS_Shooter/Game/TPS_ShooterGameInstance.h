// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "../FuncLibrary/Types.h"
#include "Engine/DataTable.h"
#include "../Items/WeaponDefault.h"

#include "TPS_ShooterGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class TPS_SHOOTER_API UTPS_ShooterGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	//table
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSetting")
		UDataTable* WeaponInfoTable = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSetting")
		UDataTable* DropItemInfoTable = nullptr;
	UFUNCTION(BlueprintCallable)
		bool GetWeaponInfoByName(FName NameWeapon, FWeaponInfo& OutInfo);
	UFUNCTION(BlueprintCallable)
		bool GetDropItemInfoByName(FName NameItem, FDropItem& OutInfo);
};
