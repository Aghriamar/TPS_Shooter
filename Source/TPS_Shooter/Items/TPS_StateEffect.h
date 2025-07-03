// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Particles/ParticleSystemComponent.h"
#include "TPS_StateEffect.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class TPS_SHOOTER_API UTPS_StateEffect : public UObject
{
	GENERATED_BODY()
public:
	
	virtual bool InitObject(AActor* Actor, FName NameBoneHit);
	virtual void DestroyObject();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	TArray<TEnumAsByte<EPhysicalSurface>> PossibleInteractSurface;

	UPROPERTY()
	bool bIsStakable = false;

	AActor* myActor = nullptr;
};

UCLASS()
class TPS_SHOOTER_API UTPS_StateEffect_ExecuteOnce : public UTPS_StateEffect
{
	GENERATED_BODY()

public:
	bool InitObject(AActor* Actor, FName NameBoneHit) override;
	void DestroyObject() override;

	virtual void ExecuteOnce();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting Execute Once")
		float Power = 20.0f;
};

UCLASS()
class TPS_SHOOTER_API UTPS_StateEffect_ExecuteTimer : public UTPS_StateEffect
{
	GENERATED_BODY()

public:
 	bool InitObject(AActor* Actor, FName NameBoneHit) override;
	void DestroyObject() override;

	virtual void Execute();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting ExecuteTimer")
		float Power = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting ExecuteTimer")
		float Timer = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting ExecuteTimer")
		float RateTime = 1.0f;

	FTimerHandle TimerHandle_ExecuteTimer;
	FTimerHandle TimerHandle_EffectTimer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting ExecuteTimer")
		UParticleSystem* ParticleEffect = nullptr;

	UParticleSystemComponent* ParticleEmitter = nullptr;
};

UCLASS()
class TPS_SHOOTER_API UTPS_StateEffect_Stun : public UTPS_StateEffect
{
	GENERATED_BODY()

public:
    bool InitObject(AActor* Actor, FName NameBoneHit) override;
	void DestroyObject() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stun")
		float StunDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stun")
		UAnimMontage* StunAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stun")
		TArray<UParticleSystem*> PossibleParticleEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stun")
		UParticleSystem* CurrentParticleEffect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stun")
		FName EffectBoneName = FName("head");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stun")
		FTransform EffectOffset = FTransform(FRotator(0), FVector(0, 0, 0), FVector(1));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stun")
		UParticleSystemComponent* ParticleComponent = nullptr;

	FTimerHandle EffectTimerHandle;

	UFUNCTION(BlueprintCallable, Category = "Stun")
		FTransform GetEffectOffset() const;

	UFUNCTION(BlueprintCallable, Category = "Stun")
		FName GetEffectBoneName() const;

	//UFUNCTION(BlueprintImplementableEvent, Category = "Stun")
		UFUNCTION(BlueprintImplementableEvent, Category = "Stun")
	void OnStunEffectApplied(ATPS_ShooterCharacter* TargetCharacter);
	//void OnStunEffectApplied_Implementation(ATPS_ShooterCharacter* TargetCharacter);

protected:
	UFUNCTION()
		void EndEffect();
};