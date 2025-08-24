// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../FuncLibrary/Types.h"
#include "../Items/WeaponDefault.h"
#include "../Character/TPSInventoryComponent.h"
#include "../Character/TPSCharacterHealthComponent.h"
#include "../Interface/TPS_IGameActor.h"
#include "../Items/TPS_StateEffect.h"
//#include "Components/WidgetComponent.h"

#include "TPS_ShooterCharacter.generated.h"

UCLASS(Blueprintable)
class ATPS_ShooterCharacter : public ACharacter, public ITPS_IGameActor
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;

	//Inputs
	void InputAxisY(float Value);
	void InputAxisX(float Value);

	void InputAttackPressed();
	void InputAttackReleased();

	void InputWalkPressed();
	void InputWalkReleased();

	void InputSprintPressed();
	void InputSprintReleased();

	void InputAimPressed();
	void InputAimReleased();

	//Inventory Inputs
	void TrySwitchNextWeapon();
	void TrySwitchPreviousWeapon();
	//Ability Inputs
	void TryAbilityEnabled();

	template<int32 Id>
	void TKeyPressed()
	{
		TrySwitchWeaponToIndexByKeyInput(Id);
	}
	//Inputs End

	//Input Flags
	float AxisX = 0.0f;
	float AxisY = 0.0f;

	bool SprintRunEnabled = false;
	bool WalkEnabled = false;
	bool AimEnabled = false;

	bool bIsAlive = true;

	UPROPERTY(Replicated)
	EMovementState MovementState = EMovementState::Run_State;

	AWeaponDefault* CurrentWeapon = nullptr;

	UDecalComponent* CurrentCursor = nullptr;

	TArray<UTPS_StateEffect*> Effects;

	int32 CurrentIndexWeapon = 0;

	UFUNCTION()
		void CharDead();
	void EnableRagdoll();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

public:
	ATPS_ShooterCharacter();

	FTimerHandle TimerHandle_RagDollTimer;

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
		class UTPSInventoryComponent* InventoryComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	class UTPSCharacterHealthComponent* CharHealthComponent;

	//Cursor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
		UMaterialInterface* CursorMaterial = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
		FVector CursorSize = FVector(20.0f, 40.0f, 40.0f);

	//Default move rule and state character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float CurrentStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float StaminaDecreaseRate = 10.0f; // ”меньшение выносливости в секунду при спринте

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float StaminaIncreaseRate = 5.0f;  // ¬осстановление выносливости в секунду

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		FCharacterSpeed MovementSpeedInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		bool bIsStunned = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		TArray<UAnimMontage*> DeadsAnim;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		TSubclassOf<UTPS_StateEffect> AbilityEffect;

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

public:
		
	// Tick Func
	UFUNCTION()
		void MovementTick(float DeltaTime);
	// Tick Func End

	//Func
	void CharacterUpdate();
	void ChangeMovementState();

	void AttackCharEvent(bool bIsFiring);

	UFUNCTION()
		void InitWeapon(FName IdWeaponName, FAdditionalWeaponInfo WeaponAdditionalInfo, int32 NewCurrentIndexWeapon);
	void TryReloadWeapon();
	//
	bool TrySwitchWeaponToIndexByKeyInput(int32 ToIndex);
	void DropCurrentWeapon();

	UFUNCTION()
		void WeaponReloadStart(UAnimMontage* Anim);
	UFUNCTION(BlueprintNativeEvent)
		void WeaponReloadStart_BP(UAnimMontage* Anim);
		void WeaponReloadStart_BP_Implementation(UAnimMontage* Anim);
	UFUNCTION()
		void WeaponReloadEnd(bool bIsSuccess, int32 AmmoSafe);
	UFUNCTION(BlueprintNativeEvent)
		void WeaponReloadEnd_BP(bool bIsSuccess);
		void WeaponReloadEnd_BP_Implementation(bool bIsSuccess);
	UFUNCTION()
		void WeaponFireStart(UAnimMontage* Anim);
	UFUNCTION(BlueprintNativeEvent)
		void WeaponFireStart_BP(UAnimMontage* Anim);
		void WeaponFireStart_BP_Implementation(UAnimMontage* Anim);

		UFUNCTION(BlueprintCallable, BlueprintPure)
			AWeaponDefault* GetCurrentWeapon();
		UFUNCTION(BlueprintCallable, BlueprintPure)
			UDecalComponent* GetCursorToWorld();
		UFUNCTION(BlueprintCallable, BlueprintPure)
			EMovementState GetMovementState();
		UFUNCTION(BlueprintCallable, BlueprintPure)
			TArray<UTPS_StateEffect*> GetCurrentEffectsOnChar();
		UFUNCTION(BlueprintCallable, BlueprintPure)
			int32 GetCurrentWeaponIndex();

		UFUNCTION(BlueprintCallable, BlueprintPure)
			bool GetIsAlive();
		//Func End

		//Interface
		EPhysicalSurface GetSurfaceType() override;
		TArray<UTPS_StateEffect*> GetAllCurrentEffects() override;
		void RemoveEffect(UTPS_StateEffect* RemoveEffect)override;
		void AddEffect(UTPS_StateEffect* newEffect)override;
		//End Interface

		UFUNCTION(BlueprintNativeEvent)
			void CharDead_BP();
		void CharDead_BP_Implementation();

		UFUNCTION(Server, Unreliable)
			void SetActorRotationByYaw_OnServer(float Yaw);
			void SetActorRotationByYaw_OnServer_Implementation(float Yaw);
		UFUNCTION(NetMulticast, Unreliable)
			void SetActorRotationByYaw_Multicast(float Yaw);
			void SetActorRotationByYaw_Multicast_Implementation(float Yaw);

		UFUNCTION(Server, Reliable)
			void SetMovementState_OnServer(EMovementState NewState);
			void SetMovementState_OnServer_Implementation(EMovementState NewState);
		UFUNCTION(NetMulticast, Reliable)
			void SetMovementState_Multicast(EMovementState NewState);
			void SetMovementState_Multicast_Implementation(EMovementState NewState);
};