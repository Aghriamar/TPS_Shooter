// Copyright Epic Games, Inc. All Rights Reserved.

#include "TPS_ShooterCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "../Game/TPS_ShooterGameInstance.h"
#include "../Items/ProjectileDefault.h"

ATPS_ShooterCharacter::ATPS_ShooterCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	InventoryComponent = CreateDefaultSubobject<UTPSInventoryComponent>(TEXT("InventoryComponent"));
	CharHealthComponent = CreateDefaultSubobject<UTPSCharacterHealthComponent>(TEXT("HealthComponent"));

	if (CharHealthComponent)
	{
		CharHealthComponent->OnDead.AddDynamic(this, &ATPS_ShooterCharacter::CharDead);
	}
	if (InventoryComponent)
	{
		InventoryComponent->OnSwitchWeapon.AddDynamic(this, &ATPS_ShooterCharacter::InitWeapon);
	}

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ATPS_ShooterCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	if (CurrentCursor)
	{
		APlayerController* myPC = Cast<APlayerController>(GetController());
		if (myPC)
		{
			FHitResult TraceHitResult;
			myPC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();

			CurrentCursor->SetWorldLocation(TraceHitResult.Location);
			CurrentCursor->SetWorldRotation(CursorR);
		}
	}

	MovementTick(DeltaSeconds);
}

void ATPS_ShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (CursorMaterial)
	{
		CurrentCursor = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CursorMaterial, CursorSize, FVector(0));
	}
}

void ATPS_ShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ATPS_ShooterCharacter::InputAxisX);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ATPS_ShooterCharacter::InputAxisY);

	PlayerInputComponent->BindAction(TEXT("ChangeToSprint"), EInputEvent::IE_Pressed, this, &ATPS_ShooterCharacter::InputSprintPressed);
	PlayerInputComponent->BindAction(TEXT("ChangeToWalk"), EInputEvent::IE_Pressed, this, &ATPS_ShooterCharacter::InputWalkPressed);
	PlayerInputComponent->BindAction(TEXT("AimEvent"), EInputEvent::IE_Pressed, this, &ATPS_ShooterCharacter::InputAimPressed);
	PlayerInputComponent->BindAction(TEXT("ChangeToSprint"), EInputEvent::IE_Released, this, &ATPS_ShooterCharacter::InputSprintReleased);
	PlayerInputComponent->BindAction(TEXT("ChangeToWalk"), EInputEvent::IE_Released, this, &ATPS_ShooterCharacter::InputWalkReleased);
	PlayerInputComponent->BindAction(TEXT("AimEvent"), EInputEvent::IE_Released, this, &ATPS_ShooterCharacter::InputAimReleased);

	PlayerInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Pressed, this, &ATPS_ShooterCharacter::InputAttackPressed);
	PlayerInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Released, this, &ATPS_ShooterCharacter::InputAttackReleased);
	PlayerInputComponent->BindAction(TEXT("ReloadEvent"), EInputEvent::IE_Released, this, &ATPS_ShooterCharacter::TryReloadWeapon);

	PlayerInputComponent->BindAction(TEXT("SwitchNextWeapon"), EInputEvent::IE_Pressed, this, &ATPS_ShooterCharacter::TrySwitchNextWeapon);
	PlayerInputComponent->BindAction(TEXT("SwitchPreviousWeapon"), EInputEvent::IE_Pressed, this, &ATPS_ShooterCharacter::TrySwitchPreviousWeapon);
	
	PlayerInputComponent->BindAction(TEXT("AbilityAction"), EInputEvent::IE_Pressed, this, &ATPS_ShooterCharacter::TryAbilityEnabled);

	PlayerInputComponent->BindAction(TEXT("DropCurrentWeapon"), EInputEvent::IE_Pressed, this, &ATPS_ShooterCharacter::DropCurrentWeapon);

	TArray<FKey> HotKeys;
	HotKeys.Add(EKeys::One);
	HotKeys.Add(EKeys::Two);
	HotKeys.Add(EKeys::Three);
	HotKeys.Add(EKeys::Four);
	HotKeys.Add(EKeys::Five);
	HotKeys.Add(EKeys::Six);
	HotKeys.Add(EKeys::Seven);
	HotKeys.Add(EKeys::Eight);
	HotKeys.Add(EKeys::Nine);
	HotKeys.Add(EKeys::Zero);

	PlayerInputComponent->BindKey(HotKeys[1], IE_Pressed, this, &ATPS_ShooterCharacter::TKeyPressed<1>);
	PlayerInputComponent->BindKey(HotKeys[2], IE_Pressed, this, &ATPS_ShooterCharacter::TKeyPressed<2>);
	PlayerInputComponent->BindKey(HotKeys[3], IE_Pressed, this, &ATPS_ShooterCharacter::TKeyPressed<3>);
	PlayerInputComponent->BindKey(HotKeys[4], IE_Pressed, this, &ATPS_ShooterCharacter::TKeyPressed<4>);
	PlayerInputComponent->BindKey(HotKeys[5], IE_Pressed, this, &ATPS_ShooterCharacter::TKeyPressed<5>);
	PlayerInputComponent->BindKey(HotKeys[6], IE_Pressed, this, &ATPS_ShooterCharacter::TKeyPressed<6>);
	PlayerInputComponent->BindKey(HotKeys[7], IE_Pressed, this, &ATPS_ShooterCharacter::TKeyPressed<7>);
	PlayerInputComponent->BindKey(HotKeys[8], IE_Pressed, this, &ATPS_ShooterCharacter::TKeyPressed<8>);
	PlayerInputComponent->BindKey(HotKeys[9], IE_Pressed, this, &ATPS_ShooterCharacter::TKeyPressed<9>);
	PlayerInputComponent->BindKey(HotKeys[0], IE_Pressed, this, &ATPS_ShooterCharacter::TKeyPressed<0>);
}

void ATPS_ShooterCharacter::InputAxisY(float value)
{
	if (!bIsStunned)
	{
		AxisY = value;
	}
}

void ATPS_ShooterCharacter::InputAxisX(float value)
{
	if (!bIsStunned)
	{
		AxisX = value;
	}
}

void ATPS_ShooterCharacter::InputAttackPressed()
{
	if (!bIsStunned && bIsAlive)
	{
		AttackCharEvent(true);
	}
}

void ATPS_ShooterCharacter::InputAttackReleased()
{
	if (!bIsStunned)
	{
		AttackCharEvent(false);
	}
}

void ATPS_ShooterCharacter::InputWalkPressed()
{
	WalkEnabled = true;
	ChangeMovementState();
}

void ATPS_ShooterCharacter::InputWalkReleased()
{
	WalkEnabled = false;
	ChangeMovementState();
}

void ATPS_ShooterCharacter::InputSprintPressed()
{
	SprintRunEnabled = true;
	ChangeMovementState();
}

void ATPS_ShooterCharacter::InputSprintReleased()
{
	SprintRunEnabled = false;
	ChangeMovementState();
}

void ATPS_ShooterCharacter::InputAimPressed()
{
	AimEnabled = true;
	ChangeMovementState();
}

void ATPS_ShooterCharacter::InputAimReleased()
{
	AimEnabled = false;
	ChangeMovementState();
}

void ATPS_ShooterCharacter::MovementTick(float DeltaTime)
{
	if (bIsAlive && !bIsStunned)
	{
		FVector MovementInput = FVector(AxisX, AxisY, 0.0f).GetSafeNormal();

		if (MovementState == EMovementState::SprintRun_State)
		{
			FVector ForwardVector = GetActorForwardVector();
			float ForwardComponent = FVector::DotProduct(MovementInput, ForwardVector);
			if (ForwardComponent > 0.7f)
			{
				AddMovementInput(ForwardVector, ForwardComponent);
			}
		}
		else
		{
			AddMovementInput(FVector(1.0f, 0.0f, 0.0f), AxisX);
			AddMovementInput(FVector(0.0f, 1.0f, 0.0f), AxisY);
		}

		APlayerController* MyController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (MyController)
		{
			FHitResult TraceHitResult;
			MyController->GetHitResultUnderCursor(ECC_GameTraceChannel1, true, TraceHitResult);
			float FindRotatorResultYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TraceHitResult.Location).Yaw;
			SetActorRotation(FQuat(FRotator(0.0f, FindRotatorResultYaw, 0.0f)));

			if (CurrentWeapon)
			{
				FVector Displacement = FVector(0);
				switch (MovementState)
				{
				case EMovementState::Aim_State:
					Displacement = FVector(0.0f, 0.0f, 160.0f);
					CurrentWeapon->ShouldReduceDispersion = true;
					break;
				case EMovementState::AimWalk_State:
					Displacement = FVector(0.0f, 0.0f, 160.0f);
					CurrentWeapon->ShouldReduceDispersion = true;
					break;
				case EMovementState::Walk_State:
					Displacement = FVector(0.0f, 0.0f, 120.0f);
					CurrentWeapon->ShouldReduceDispersion = false;
					break;
				case EMovementState::Run_State:
					Displacement = FVector(0.0f, 0.0f, 120.0f);
					CurrentWeapon->ShouldReduceDispersion = false;
					break;
				case EMovementState::SprintRun_State:
					CurrentWeapon->ShouldReduceDispersion = false;
					break;
				default:
					break;
				}

				CurrentWeapon->ShootEndLocation = TraceHitResult.Location + Displacement;
			}
		}

		if (MovementState == EMovementState::SprintRun_State)
		{
			CurrentStamina -= StaminaDecreaseRate * DeltaTime;
			if (CurrentStamina <= 0.0f)
			{
				CurrentStamina = 0.0f;
				MovementState = EMovementState::Run_State; // Переход в бег, если выносливость кончилась
			}
		}
		else
		{
			CurrentStamina += StaminaIncreaseRate * DeltaTime;
			if (CurrentStamina > MaxStamina)
			{
				CurrentStamina = MaxStamina;
			}
		}

		ChangeMovementState();
	}
}

EMovementState ATPS_ShooterCharacter::GetMovementState()
{
	return MovementState;
}

TArray<UTPS_StateEffect*> ATPS_ShooterCharacter::GetCurrentEffectsOnChar()
{
	return Effects;
}

int32 ATPS_ShooterCharacter::GetCurrentWeaponIndex()
{
	return CurrentIndexWeapon;
}

bool ATPS_ShooterCharacter::GetIsAlive()
{
	return bIsAlive;
}

void ATPS_ShooterCharacter::AttackCharEvent(bool bIsFiring)
{
	AWeaponDefault* myWeapon = nullptr;
	myWeapon = GetCurrentWeapon();
	if (myWeapon)
	{
		//ToDo Check melee or range
		myWeapon->SetWeaponStateFire(bIsFiring);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("ATPS_ShooterCharacter::AttackCharEvent - CurrentWeapon -NULL"));
}

void ATPS_ShooterCharacter::CharacterUpdate()
{
	float ResSpeed = 600.0f;
	float ResAcceleration = MovementSpeedInfo.NormalAcceleration;

	if (!bIsStunned)
	{
		switch (MovementState)
		{
		case EMovementState::Aim_State:
			ResSpeed = MovementSpeedInfo.AimSpeedNormal;
			break;
		case EMovementState::AimWalk_State:
			ResSpeed = MovementSpeedInfo.AimSpeedWalk;
			break;
		case EMovementState::Walk_State:
			ResSpeed = MovementSpeedInfo.WalkSpeedNormal;
			break;
		case EMovementState::Run_State:
			ResSpeed = MovementSpeedInfo.RunSpeedNormal;
			break;
		case EMovementState::SprintRun_State:
			ResSpeed = MovementSpeedInfo.SprintRunSpeedRun;
			ResAcceleration = MovementSpeedInfo.SprintAcceleration;
			break;
		default:
			break;
		}
	}

	GetCharacterMovement()->MaxWalkSpeed = ResSpeed;
	GetCharacterMovement()->MaxAcceleration = ResAcceleration;
}

void ATPS_ShooterCharacter::ChangeMovementState()
{
	FVector ForwardVector = GetActorForwardVector();
	FVector MovementInput = GetVelocity().GetSafeNormal();
	bool bIsMovingForward = FVector::DotProduct(ForwardVector, MovementInput) > 0.7f;

	if (SprintRunEnabled && bIsMovingForward && CurrentStamina > 0)
	{
		WalkEnabled = false;
		AimEnabled = false;
		MovementState = EMovementState::SprintRun_State;
	}
	else if (WalkEnabled && AimEnabled)
	{
		MovementState = EMovementState::AimWalk_State;
	}
	else if (WalkEnabled)
	{
		MovementState = EMovementState::Walk_State;
	}
	else if (AimEnabled)
	{
		MovementState = EMovementState::Aim_State;
	}
	else
	{
		MovementState = EMovementState::Run_State;
	}

	CharacterUpdate();

	//Weapon state update
	AWeaponDefault* myWeapon = GetCurrentWeapon();
	if (myWeapon)
	{
		myWeapon->UpdateStateWeapon(MovementState);
	}
}

AWeaponDefault* ATPS_ShooterCharacter::GetCurrentWeapon()
{
	return CurrentWeapon;
}

void ATPS_ShooterCharacter::InitWeapon(FName IdWeaponName, FAdditionalWeaponInfo WeaponAdditionalInfo, int32 NewCurrentIndexWeapon)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
	}

	UTPS_ShooterGameInstance* myGI = Cast<UTPS_ShooterGameInstance>(GetGameInstance());
	FWeaponInfo myWeaponInfo;
	if (myGI)
	{
		if (myGI->GetWeaponInfoByName(IdWeaponName, myWeaponInfo))
		{
			if (myWeaponInfo.WeaponClass)
			{
				FVector SpawnLocation = FVector(0);
				FRotator SpawnRotation = FRotator(0);

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Owner = this;
				SpawnParams.Instigator = GetInstigator();

				AWeaponDefault* myWeapon = Cast<AWeaponDefault>(GetWorld()->SpawnActor(myWeaponInfo.WeaponClass, &SpawnLocation, &SpawnRotation, SpawnParams));
				if (myWeapon)
				{
					FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
					myWeapon->AttachToComponent(GetMesh(), Rule, FName("WeaponSocketRightHand"));
					myWeapon->WeaponID = IdWeaponName;
					CurrentWeapon = myWeapon;

					myWeapon->WeaponSetting = myWeaponInfo;
					myWeapon->ReloadTime = myWeaponInfo.ReloadTime;
					myWeapon->UpdateStateWeapon(MovementState);
					myWeapon->AdditionalWeaponInfo = WeaponAdditionalInfo;

					CurrentIndexWeapon = NewCurrentIndexWeapon;

					myWeapon->OnWeaponReloadStart.AddDynamic(this, &ATPS_ShooterCharacter::WeaponReloadStart);
					myWeapon->OnWeaponReloadEnd.AddDynamic(this, &ATPS_ShooterCharacter::WeaponReloadEnd);
					myWeapon->OnWeaponFireStart.AddDynamic(this, & ATPS_ShooterCharacter::WeaponFireStart);
					
					// after switch try reload weapon if needed
					if (CurrentWeapon->GetWeaponRound() <= 0 && CurrentWeapon->CheckCanWeaponReload())
						CurrentWeapon->InitReload();
					if (InventoryComponent)
						InventoryComponent->OnWeaponAmmoAviable.Broadcast(myWeapon->WeaponSetting.WeaponType);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ATPS_ShooterCharacter::InitWeapon - Weapon not found in table -NULL"));
		}
	}
}

void ATPS_ShooterCharacter::TryReloadWeapon()
{
	if (!bIsStunned && bIsAlive && CurrentWeapon && !CurrentWeapon->WeaponReloading)
	{
		if (CurrentWeapon->GetWeaponRound() < CurrentWeapon->WeaponSetting.MaxRound && CurrentWeapon->CheckCanWeaponReload())
			CurrentWeapon->InitReload();
	}
}

void ATPS_ShooterCharacter::WeaponReloadStart(UAnimMontage* Anim)
{
	if (!bIsStunned)
	{
		WeaponReloadStart_BP(Anim);
	}
}

void ATPS_ShooterCharacter::WeaponReloadEnd(bool bIsSuccess, int32 AmmoTake)
{
	if (InventoryComponent && CurrentWeapon)
	{
		InventoryComponent->AmmoSlotChangeValue(CurrentWeapon->WeaponSetting.WeaponType, AmmoTake);
		InventoryComponent->SetAdditionalInfoWeapon(CurrentIndexWeapon, CurrentWeapon->AdditionalWeaponInfo);
	}
	WeaponReloadEnd_BP(bIsSuccess);
}

bool ATPS_ShooterCharacter::TrySwitchWeaponToIndexByKeyInput(int32 ToIndex)
{
	bool bIsSuccess = false;
	if (CurrentWeapon && !CurrentWeapon->WeaponReloading && InventoryComponent->WeaponSlots.IsValidIndex(ToIndex))
	{
		if (CurrentIndexWeapon != ToIndex && InventoryComponent)
		{
			int32 OldIndex = CurrentIndexWeapon;
			FAdditionalWeaponInfo OldInfo;

			if (CurrentWeapon)
			{
				OldInfo = CurrentWeapon->AdditionalWeaponInfo;
				if (CurrentWeapon->WeaponReloading)
					CurrentWeapon->CancelReload();
			}

			bIsSuccess = InventoryComponent->SwitchWeaponByIndex(ToIndex, OldIndex, OldInfo);
		}
	}
	return bIsSuccess;
}

void ATPS_ShooterCharacter::DropCurrentWeapon()
{
	if (InventoryComponent)
	{
		FDropItem ItemInfo;
		InventoryComponent->DropWeapobByIndex(CurrentIndexWeapon, ItemInfo);
	}
}

void ATPS_ShooterCharacter::WeaponReloadStart_BP_Implementation(UAnimMontage* Anim)
{
	// in BP
}

void ATPS_ShooterCharacter::WeaponReloadEnd_BP_Implementation(bool bIsSuccess)
{
	// in BP
}

void ATPS_ShooterCharacter::WeaponFireStart(UAnimMontage* Anim)
{
	if (!bIsStunned && InventoryComponent && CurrentWeapon)
		InventoryComponent->SetAdditionalInfoWeapon(CurrentIndexWeapon, CurrentWeapon->AdditionalWeaponInfo);
	WeaponFireStart_BP(Anim);
}

void ATPS_ShooterCharacter::WeaponFireStart_BP_Implementation(UAnimMontage* Anim)
{
	// in BP
}

UDecalComponent* ATPS_ShooterCharacter::GetCursorToWorld()
{
	return CurrentCursor;
}

//ToDO in one func TrySwitchPreviosWeapon && TrySwicthNextWeapon
//need Timer to Switch with Anim, this method stupid i must know switch success for second logic inventory
//now we not have not success switch/ if 1 weapon switch to self
void ATPS_ShooterCharacter::TrySwitchNextWeapon()
{
	if (!bIsStunned && CurrentWeapon && !CurrentWeapon->WeaponReloading && InventoryComponent->WeaponSlots.Num() > 1)
	{
		//We have more then one weapon go switch
		int8 OldIndex = CurrentIndexWeapon;
		FAdditionalWeaponInfo OldInfo;
		if (CurrentWeapon)
		{
			OldInfo = CurrentWeapon->AdditionalWeaponInfo;
			if (CurrentWeapon->WeaponReloading)
				CurrentWeapon->CancelReload();
		}

		if (InventoryComponent)
		{
			if (InventoryComponent->SwitchWeaponToIndexByNextPreviosIndex(CurrentIndexWeapon + 1, OldIndex, OldInfo, true))
			{
			}
		}
	}
}

void ATPS_ShooterCharacter::TrySwitchPreviousWeapon()
{
	if (!bIsStunned && CurrentWeapon && !CurrentWeapon->WeaponReloading && InventoryComponent->WeaponSlots.Num() > 1)
	{
		//We have more then one weapon go switch
		int8 OldIndex = CurrentIndexWeapon;
		FAdditionalWeaponInfo OldInfo;
		if (CurrentWeapon)
		{
			OldInfo = CurrentWeapon->AdditionalWeaponInfo;
			if (CurrentWeapon->WeaponReloading)
				CurrentWeapon->CancelReload();
		}

		if (InventoryComponent)
		{
			//InventoryComponent->SetAdditionalInfoWeapon(OldIndex, GetCurrentWeapon()->AdditionalWeaponInfo);
			if (InventoryComponent->SwitchWeaponToIndexByNextPreviosIndex(CurrentIndexWeapon - 1, OldIndex, OldInfo, false))
			{
			}
		}
	}
}

void ATPS_ShooterCharacter::TryAbilityEnabled()
{
	 if (!bIsStunned && AbilityEffect)//TODO Cool down
	 {
	 	UTPS_StateEffect* NewEffect = NewObject<UTPS_StateEffect>(this, AbilityEffect);
	 	if (NewEffect)
	 	{
	 		NewEffect->InitObject(this, NAME_None);
	 	}
	 }
}

EPhysicalSurface ATPS_ShooterCharacter::GetSurfaceType()
{
	EPhysicalSurface Result = EPhysicalSurface::SurfaceType_Default;
	if (CharHealthComponent)
	{
		if (CharHealthComponent->GetCurrentShield() <= 0)
		{
			if (GetMesh())
			{
				UMaterialInterface* myMaterial = GetMesh()->GetMaterial(0);
				if (myMaterial)
				{
					Result = myMaterial->GetPhysicalMaterial()->SurfaceType;
				}
			}
		}
	}
	return Result;
}

TArray<UTPS_StateEffect*> ATPS_ShooterCharacter::GetAllCurrentEffects()
{
	return Effects;
}

void ATPS_ShooterCharacter::RemoveEffect(UTPS_StateEffect* RemoveEffect)
{
	Effects.Remove(RemoveEffect);
}

void ATPS_ShooterCharacter::AddEffect(UTPS_StateEffect* newEffect)
{
	Effects.Add(newEffect);
}

void ATPS_ShooterCharacter::CharDead_BP_Implementation()
{
	//BP
}

void ATPS_ShooterCharacter::CharDead()
{
	float TimeAnim = 0.0f;
	int32 rnd = FMath::RandHelper(DeadsAnim.Num());
	if (DeadsAnim.IsValidIndex(rnd) && DeadsAnim[rnd] && GetMesh() && GetMesh()->GetAnimInstance())
	{
		TimeAnim = DeadsAnim[rnd]->GetPlayLength();
		GetMesh()->GetAnimInstance()->Montage_Play(DeadsAnim[rnd]);
	}

	bIsAlive = false;

	if (GetController())
	{
		GetController()->UnPossess();
	}

	UnPossessed();

	//Timer rag doll
	GetWorldTimerManager().SetTimer(TimerHandle_RagDollTimer, this, &ATPS_ShooterCharacter::EnableRagdoll, TimeAnim, false);

	GetCursorToWorld()->SetVisibility(false);

	AttackCharEvent(false);

	CharDead_BP();
}

void ATPS_ShooterCharacter::EnableRagdoll()
{
	if (GetMesh())
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		GetMesh()->SetSimulatePhysics(true);
	}
}

float ATPS_ShooterCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (bIsAlive)
	{
		CharHealthComponent->ChangeHealthValue(-DamageAmount);
	}

	if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		AProjectileDefault* myProjectile = Cast<AProjectileDefault>(DamageCauser);
		if (myProjectile)
		{
			UTypes::AddEffectBySurfaceType(this, NAME_None, myProjectile->ProjectileSetting.Effect, GetSurfaceType()); // to do NAME_None - bone for radial damage
		}
	}

	return ActualDamage;
}