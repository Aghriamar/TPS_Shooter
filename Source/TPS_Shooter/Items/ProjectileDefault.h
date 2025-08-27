// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "../FuncLibrary/Types.h"
#include "ProjectileDefault.generated.h"

UCLASS()
class TPS_SHOOTER_API AProjectileDefault : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectileDefault();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class UStaticMeshComponent* BulletMesh = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class USphereComponent* BulletCollisionSphere = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class UProjectileMovementComponent* BulletProjectileMovement = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class UParticleSystemComponent* BulletFX = nullptr;
	UPROPERTY(BlueprintReadOnly)
	FProjectileInfo ProjectileSetting;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable)
	void InitProjectile(FProjectileInfo InitParam);
	UFUNCTION()
	virtual void BulletCollisionSphereHit(class UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION()
	void BulletCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void BulletCollisionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
		virtual void ImpactProjectile();

	UFUNCTION(NetMulticast, Reliable)
	void InitVisualMeshProjectile_Multicast(UStaticMesh* newMesh, FTransform MeshRelative);
	void InitVisualMeshProjectile_Multicast_Implementation(UStaticMesh* newMesh, FTransform MeshRelative);
	UFUNCTION(NetMulticast, Reliable)
	void InitVisualTrailProjectile_Multicast(UParticleSystem* NewTemplate, FTransform TemplateRelative);
	void InitVisualTrailProjectile_Multicast_Implementation(UParticleSystem* NewTemplate, FTransform TemplateRelative);
	UFUNCTION(NetMulticast, Reliable)
	void SpawnHitDecal_Multicast(UMaterialInterface* DecalMaterial, UPrimitiveComponent* OtherComp, FHitResult HitResult);
	void SpawnHitDecal_Multicast_Implementation(UMaterialInterface* DecalMaterial, UPrimitiveComponent* OtherComp, FHitResult HitResult);
	UFUNCTION(NetMulticast, Reliable)
	void SpawnHitFX_Multicast(UParticleSystem* FxTemplate, FHitResult HitResult);
	void SpawnHitFX_Multicast_Implementation(UParticleSystem* FxTemplate, FHitResult HitResult);
	UFUNCTION(NetMulticast, Reliable)
	void SpawnHitSound_Multicast(USoundBase* HitSound, FHitResult HitResult);
	void SpawnHitSound_Multicast_Implementation(USoundBase* HitSound, FHitResult HitResult);
};
