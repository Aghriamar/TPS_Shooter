// Fill out your copyright notice in the Description page of Project Settings.

#include "../Items/TPS_StateEffect.h"
#include "../Character/TPSHealthComponent.h"
#include "../Interface/TPS_IGameActor.h"
#include "../Character/TPS_ShooterCharacter.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Animation/AnimMontage.h"
#include "../Interface/TPS_IGameActor.h"

bool UTPS_StateEffect::InitObject(AActor* Actor)
{
	myActor = Actor;

	ITPS_IGameActor* myInterface = Cast<ITPS_IGameActor>(myActor);
	if (myInterface)
	{
		myInterface->AddEffect(this);
	}

	return true;
}

void UTPS_StateEffect::DestroyObject()
{
	ITPS_IGameActor* myInterface = Cast<ITPS_IGameActor>(myActor);
	if (myInterface)
	{
		myInterface->RemoveEffect(this);
	}

	myActor = nullptr;
	if (this && this->IsValidLowLevel())
	{
		this->ConditionalBeginDestroy();
	}
}

bool UTPS_StateEffect_ExecuteOnce::InitObject(AActor* Actor)
{
	Super::InitObject(Actor);
	ExecuteOnce();
	return true;
}

void UTPS_StateEffect_ExecuteOnce::DestroyObject()
{
	Super::DestroyObject();
}

void UTPS_StateEffect_ExecuteOnce::ExecuteOnce()
{
	if (myActor)
	{
		UTPSHealthComponent* myHealthComp = Cast<UTPSHealthComponent>(myActor->GetComponentByClass(UTPSHealthComponent::StaticClass()));
		if (myHealthComp)
		{
			myHealthComp->ChangeHealthValue(Power);
		}
	}

	DestroyObject();
}

bool UTPS_StateEffect_ExecuteTimer::InitObject(AActor* Actor)
{
	Super::InitObject(Actor);

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_EffectTimer, this, &UTPS_StateEffect_ExecuteTimer::DestroyObject, Timer, false);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_ExecuteTimer, this, &UTPS_StateEffect_ExecuteTimer::Execute, RateTime, true);

	if (ParticleEffect)
	{
		//ToDo for object with interface create function return offset, Name bones, 
		//ToDo Random init Effect with aviable array (For)
		FName NameBoneToAttached;
		FVector Loc = FVector(0);

		ParticleEmitter = UGameplayStatics::SpawnEmitterAttached(ParticleEffect, myActor->GetRootComponent(), NameBoneToAttached, Loc, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false);
	}

	return true;
}

void UTPS_StateEffect_ExecuteTimer::DestroyObject()
{
	ParticleEmitter->DestroyComponent();
	ParticleEmitter = nullptr;
	Super::DestroyObject();
}

void UTPS_StateEffect_ExecuteTimer::Execute()
{
	if (myActor)
	{
		//UGameplayStatics::ApplyDamage(myActor,Power,nullptr,nullptr,nullptr);	
		UTPSHealthComponent* myHealthComp = Cast<UTPSHealthComponent>(myActor->GetComponentByClass(UTPSHealthComponent::StaticClass()));
		if (myHealthComp)
		{
			myHealthComp->ChangeHealthValue(Power);
		}
	}
}

bool UTPS_StateEffect_Stun::InitObject(AActor* Actor)
{
	if (!Super::InitObject(Actor))
	{
		return false;
	}

	// Случайный выбор Particle System из массива
	if (PossibleParticleEffects.Num() > 0)
	{
		int32 RandomIndex = FMath::RandHelper(PossibleParticleEffects.Num());
		CurrentParticleEffect = PossibleParticleEffects[RandomIndex];
	}

	// Каст к TPS_ShooterCharacter
	ATPS_ShooterCharacter* Character = Cast<ATPS_ShooterCharacter>(Actor);
	if (Character)
	{
		Character->bIsStunned = true;
		// Отключение ввода
		APlayerController* Controller = Cast<APlayerController> (Character->GetController());
		if (Controller)
		{
			Controller->DisableInput(Controller);
		}

		// Блокировка стрельбы и перезарядки
		if (Character->CurrentWeapon)
		{
			
			Character->CurrentWeapon->BlockFire = true;
			Character->CurrentWeapon->WeaponFiring = false;
			if (Character->CurrentWeapon->WeaponReloading)
			{
				Character->CurrentWeapon->CancelReload();
			}
		}

		// Проигрывание анимации стана (зацикленная)
		if (StunAnimation && Character->GetMesh() && Character->GetMesh()->GetAnimInstance())
		{
			Character->GetMesh()->GetAnimInstance()->Montage_Play(StunAnimation, 1.0f);
		}

		// Установка таймера для завершения эффекта
		GetWorld()->GetTimerManager().SetTimer(
			EffectTimerHandle,
			this,
			&UTPS_StateEffect_Stun::EndEffect,
			StunDuration,
			false
		);

		// Вызов Blueprint-события для визуального эффекта
		OnStunEffectApplied(Character);
	}

	return true;
}

void UTPS_StateEffect_Stun::DestroyObject()
{
	// Уничтожение Particle System (если создано в Blueprint)
	if (ParticleComponent)
	{
		ParticleComponent->DestroyComponent();
		ParticleComponent = nullptr;
	}

	Super::DestroyObject();
}

void UTPS_StateEffect_Stun::EndEffect()
{
	//Каст к TPS_ShooterCharacter
	ATPS_ShooterCharacter* Character = Cast<ATPS_ShooterCharacter>(GetOuter());
	if (Character)
	{
		Character->bIsStunned = false;
		// Восстановление ввода
		APlayerController* Controller = Cast<APlayerController>(Character->GetController());
		if (Controller)
		{
			Controller->EnableInput(Controller);
		}

		//Остановка анимации
		if (Character->GetMesh() && Character->GetMesh()->GetAnimInstance())
		{
			Character->GetMesh()->GetAnimInstance()->StopAllMontages(/*0.15f*/StunDuration);
		}

		// Разблокировка стрельбы
		if (Character->CurrentWeapon)
		{
			Character->CurrentWeapon->BlockFire = false;
			Character->CurrentWeapon->WeaponFiring = false;
		}
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(EffectTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(EffectTimerHandle);
	}

	// Удаление эффекта через интерфейс
	if (ITPS_IGameActor* Interface = Cast<ITPS_IGameActor>(GetOuter()))
	{
		Interface->RemoveEffect(this);
	}

	DestroyObject();
}

FTransform UTPS_StateEffect_Stun::GetEffectOffset() const
{
	return EffectOffset;
}

FName UTPS_StateEffect_Stun::GetEffectBoneName() const
{
	return EffectBoneName;
}