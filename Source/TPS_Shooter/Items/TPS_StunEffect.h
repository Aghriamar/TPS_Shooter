//// Fill out your copyright notice in the Description page of Project Settings.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "TPS_StunEffect.generated.h"
//
//class UParticleSystemComponent;
//
///**
// * 
// */
//UCLASS()
//class TPS_SHOOTER_API UTPS_StunEffect : public UTPS_StateEffect
//{
//	GENERATED_BODY()
//	
//public:
//	UTPS_StunEffect();
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StunEffect")
//	float StunDuration = 3.0f; // Длительность стана
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StunEffect")
//		float Time = 0.0f;
//
//	UPROPERTY(BlueprintReadOnly)
//		bool bIsActive = false;
//
//	UPROPERTY(BlueprintReadOnly)
//		AActor* OwnerActor = nullptr;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StunEffect")
//		UAnimMontage* StunAnimation = nullptr;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StunEffect")
//		UParticleSystem* StunParticleFX = nullptr;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StunEffect")
//		FName AttachBoneName = TEXT("head");
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StunEffect")
//		FVector AttachOffset = FVector(0.0f, 0.0f, 20.0f);
//
//protected:
//	virtual void ExecuteEffect(AActor* TargetActor) override;
//	virtual void EndEffect() override;
//	virtual void TickEffect(float DeltaTime) override;
//
//private:
//	float CurrentStunTime;
//	UParticleSystemComponent* ActiveStunParticle = nullptr;
//	TEnumAsByte<EMovementMode> OriginalMovementMode;
//};
