//// Fill out your copyright notice in the Description page of Project Settings.
//
//
//#include "../Items/TPS_StunEffect.h"
//#include "GameFramework/CharacterMovementComponent.h"
//#include "../Character/TPS_ShooterCharacter.h"
//#include "Components/SkeletalMeshComponent.h"
//#include "Kismet/GameplayStatics.h"
//#include "Particles/ParticleSystemComponent.h"
//#include "../Interface/TPS_IGameActor.h"
//
//UTPS_StunEffect::UTPS_StunEffect()
//{
//	bIsStakable = false;
//	bIsActive = true;
//	Time = StunDuration;
//}
//
//void UTPS_StunEffect::ExecuteEffect(AActor* TargetActor)
//{
//	//Super::ExecuteEffect(TargetActor);
//
//	CurrentStunTime = StunDuration;
//
//	ATPS_ShooterCharacter* Character = Cast<ATPS_ShooterCharacter>(TargetActor);
//	if (Character)
//	{
//		OriginalMovementMode = Character->GetCharacterMovement()->MovementMode;
//		Character->GetCharacterMovement()->DisableMovement();
//		Character->GetCharacterMovement()->StopMovementImmediately();
//
//		Character->AttackCharEvent(false);
//		Character->bIsStunned = true;
//
//		if (StunAnimation && Character->GetMesh() && Character->GetMesh()->GetAnimInstance())
//		{
//			Character->GetMesh()->GetAnimInstance()->Montage_Play(StunAnimation);
//		}
//
//		if (StunParticleFX && Character->GetMesh())
//		{
//			ActiveStunParticle = UGameplayStatics::SpawnEmitterAttached(
//				StunParticleFX,
//				Character->GetMesh,
//				AttachBoneName,
//				AttachOffset,
//				FRotator::ZeroRotator,
//				EAttachLocation::SnapTarget,
//				true);
//		}
//	}
//}
//
//void UTPS_StunEffect::EndEffect()
//{
//	//Super::EndEffect();
//
//	ATPS_ShooterCharacter* Character = Cast<ATPS_ShooterCharacter>(OwnerActor);
//	if (Character)
//	{
//		Character->GetCharacterMovement()->SetMovementMode(OriginalMovementMode);
//
//		Character->bIsStunned = false;
//
//		if (StunAnimation && Character->GetMesh() && Character->GetMesh()->GetAnimInstance())
//		{
//			Character->GetMesh()->GetAnimInstance()->Montage_Stop(0.15f, StunAnimation);
//		}
//
//		if (ActiveStunParticle)
//		{
//			ActiveStunParticle->DestroyComponent();
//			ActiveStunParticle = nullptr;
//		}
//	}
//
//	if (OwnerActor && OwnerActor->GetClass()->ImplementsInterface(UTPS_IGameActor::StaticClass()))
//	{
//		ITPS_IGameActor::Execute_RemoveEffect(OwnerActor, this);
//	}
//	ConditionBeginDestroy();
//}
//
//void UTPS_StunEffect::TickEffect(float DeltaTime)
//{
//	//Super::TickEffect(DeltaTime);
//
//	CurrentStunTime -= DeltaTime;
//	if (CurrentStunTime <= 0.0f)
//	{
//		EndEffect();
//	}
//}