// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "../FuncLibrary/Types.h"
#include "../Items/ProjectileDefault.h"
#include "WeaponDefault.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponFireStart, UAnimMontage*, Anim);//ToDo Delegate on event weapon fire - Anim char, state char...
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponReloadStart, UAnimMontage*, Anim);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponReloadEnd, bool, bIsSuccess, int32, AmmoSafe);

UCLASS()
class TPS_SHOOTER_API AWeaponDefault : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponDefault();

	FOnWeaponFireStart OnWeaponFireStart;
	FOnWeaponReloadEnd OnWeaponReloadEnd;
	FOnWeaponReloadStart OnWeaponReloadStart;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class USceneComponent* SceneComponent = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class USkeletalMeshComponent* SkeletalMeshWeapon = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class UStaticMeshComponent* StaticMeshWeapon = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class UArrowComponent* ShootLocation = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Info") // Для отладки и доступа из BP
		FName WeaponID;

	

	UPROPERTY(VisibleAnywhere)
		FWeaponInfo WeaponSetting;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon Info")
		FAdditionalWeaponInfo AdditionalWeaponInfo;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Tick func
	virtual void Tick(float DeltaTime) override;

	void FireTick(float DeltaTime);
	void ReloadTick(float DeltaTime);
	void DispersionTick(float DeltaTime);
	void ClipDropTick(float DeltaTime);
	void ShellDropTick(float DeltaTime);

	void WeaponInit();

	UFUNCTION(Server, Reliable, BlueprintCallable)
		void SetWeaponStateFire_OnServer(bool bIsFire);
		void SetWeaponStateFire_OnServer_Implementation(bool bIsFire);

	bool CheckWeaponCanFire();

	FProjectileInfo GetProjectile();
	UFUNCTION()
	void Fire();

	UFUNCTION(Server, Reliable)
	void UpdateStateWeapon_OnServer(EMovementState NewMovementState);
	void UpdateStateWeapon_OnServer_Implementation(EMovementState NewMovementState);
	void ChangeDispersionByShot();
	float GetCurrentDispersion() const;
	FVector ApplyDispersionToShoot(FVector DirectionShoot) const;

	FVector GetFireEndLocation() const;
	int8 GetNumberProjectileByShot() const;

	//Timers
	float FireTimer = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadLogic")
		float ReloadTimer = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadLogic Debug") //Remove !!! Debug
		float ReloadTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Logic")
		FName IdWeaponName;

	//flags
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireLogic")
		bool WeaponFiring = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadLogic")
		bool WeaponReloading = false;
		bool WeaponAiming = false;

	bool BlockFire = false;
	//Dispersion
	bool ShouldReduceDispersion = false;
	float CurrentDispersion = 0.0f;
	float CurrentDispersionMax = 1.0f;
	float CurrentDispersionMin = 0.1f;
	float CurrentDispersionRecoil = 0.1f;
	float CurrentDispersionReduction = 0.1f;

	//Timer Drop Magazine on reload
	bool DropClipFlag = false;
	float DropClipTimer = -1.0;

	//shell flag
	bool DropShellFlag = false;
	float DropShellTimer = -1.0f;

	UPROPERTY(Replicated)
	FVector ShootEndLocation = FVector(0);

	UFUNCTION(BlueprintCallable)
		int32 GetWeaponRound();
	UFUNCTION()
	void InitReload();
	void FinishReload();
	void CancelReload();

	bool CheckCanWeaponReload();
	int8 GetAviableAmmoForReload();

	UFUNCTION(Server, Reliable)
	void InitDropMesh_OnServer(UStaticMesh* DropMesh, FTransform Offset, FVector DropImpulseDirection, float LifeTimeMesh, float ImpulseRandomDispersion, float PowerImpulse, float CustomMass);
	void InitDropMesh_OnServer_Implementation(UStaticMesh* DropMesh, FTransform Offset, FVector DropImpulseDirection, float LifeTimeMesh, float ImpulseRandomDispersion, float PowerImpulse, float CustomMass);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		bool ShowDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		float SizeVectorToChangeShootDirectionLogic = 100.0f;

	//
	UFUNCTION(Server, UnReliable)
	void UpdateWeaponByCharacterMovementState_OnServer(FVector NewShootEndLocation, bool NewShouldReduceDispersion);
	void UpdateWeaponByCharacterMovementState_OnServer_Implementation(FVector NewShootEndLocation, bool NewShouldReduceDispersion);

	UFUNCTION(NetMulticast, UnReliable)
	void AnimWeaponStart_Multicast(UAnimMontage* Anim);
	void AnimWeaponStart_Multicast_Implementation(UAnimMontage* Anim);
	UFUNCTION(NetMulticast, UnReliable)
	void ShellDropFire_Multicast(UStaticMesh* DropMesh, FTransform Offset, FVector DropImpulseDirection, float LifeTimeMesh, float ImpulseRandomDispersion, float PowerImpulse, float CustomMass, FVector LocalDir);
	void ShellDropFire_Multicast_Implementation(UStaticMesh* DropMesh, FTransform Offset, FVector DropImpulseDirection, float LifeTimeMesh, float ImpulseRandomDispersion, float PowerImpulse, float CustomMass, FVector LocalDir);
	UFUNCTION(NetMulticast, UnReliable)
	void FXWeaponFire_Multicast(UParticleSystem* FxFire, USoundBase* SoundFire);
	void FXWeaponFire_Multicast_Implementation(UParticleSystem* FxFire, USoundBase* SoundFire);

	UFUNCTION(NetMulticast, Unreliable)
	void TraceFX_Multicast(const FVector& Start, const FVector& End);
	void TraceFX_Multicast_Implementation(const FVector& Start, const FVector& End);
	UFUNCTION(NetMulticast, Unreliable)
	void ImpactFX_Multicast(UParticleSystem* FxImpact, USoundBase* SoundImpact, UMaterialInterface* DecalImpact, const FHitResult& Hit);
	void ImpactFX_Multicast_Implementation(UParticleSystem* FxImpact, USoundBase* SoundImpact, UMaterialInterface* DecalImpact, const FHitResult& Hit);

	// Функция для обновления привязки при смене оружия
	UFUNCTION(BlueprintCallable)
		void UpdateWeaponComponent();

private:

	UPROPERTY()
		USceneComponent* ActiveWeaponComponent = nullptr; // Хранит текущий активный компонент
};
