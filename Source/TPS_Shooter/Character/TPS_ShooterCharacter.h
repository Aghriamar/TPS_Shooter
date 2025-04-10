// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../FuncLibrary/Types.h"
#include "../Items/WeaponDefault.h"
#include "TPS_ShooterCharacter.generated.h"

UCLASS(Blueprintable)
class ATPS_ShooterCharacter : public ACharacter
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;

public:
	ATPS_ShooterCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns CursorToWorld subobject **/
	//FORCEINLINE class UDecalComponent* GetCursorToWorld() { return CursorToWorld; }

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** A decal that projects to the cursor location. */
	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UDecalComponent* CursorToWorld;*/

public:
	//Cursor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
		UMaterialInterface* CursorMaterial = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
		FVector CursorSize = FVector(20.0f, 40.0f, 40.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float CurrentStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float StaminaDecreaseRate = 10.0f; // ”меньшение выносливости в секунду при спринте

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float StaminaIncreaseRate = 5.0f;  // ¬осстановление выносливости в секунду

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		EMovementState MovementState = EMovementState::Run_State;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		FCharacterSpeed MovementSpeedInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		bool SprintRunEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		bool WalkEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		bool AimEnabled = false;

	//Weapon
	AWeaponDefault* CurrentWeapon = nullptr;

	//For Demo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
		TSubclassOf<AWeaponDefault> InitWeaponClass = nullptr;

	UDecalComponent* CurrentCursor = nullptr;

	UFUNCTION()
		void InputAxisX(float value);
	UFUNCTION()
		void InputAxisY(float value);
	UFUNCTION()
		void InputAttackPressed();
	UFUNCTION()
		void InputAttackReleased();

	float AxisX = 0.0f;
	float AxisY = 0.0f;

	UFUNCTION()
	void MovementTick(float DeltaTime);

	//Func
	UFUNCTION(BlueprintCallable)
		void AttackCharEvent(bool bIsFiring);
	UFUNCTION(BlueprintCallable)
		void CharacterUpdate();
	UFUNCTION(BlueprintCallable)
		void ChangeMovementState();

	UFUNCTION(BlueprintCallable)
		AWeaponDefault* GetCurrentWeapon();
	UFUNCTION(BlueprintCallable)
		void InitWeapon();

	UFUNCTION(BlueprintCallable)
		UDecalComponent* GetCursorToWorld();
};

