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

	// Create a decal in the world to show the cursor's location
	/*CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/Materials/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}
	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());*/

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
	InitWeapon(InitWeaponName);
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

	PlayerInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Pressed, this, &ATPS_ShooterCharacter::InputAttackPressed);
	PlayerInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Released, this, &ATPS_ShooterCharacter::InputAttackReleased);
	PlayerInputComponent->BindAction(TEXT("ReloadEvent"), EInputEvent::IE_Released, this, &ATPS_ShooterCharacter::TryReloadWeapon);
}

void ATPS_ShooterCharacter::InputAxisX(float value)
{
	AxisX = value;
}

void ATPS_ShooterCharacter::InputAxisY(float value)
{
	AxisY = value;
}

void ATPS_ShooterCharacter::InputAttackPressed()
{
	AttackCharEvent(true);
}

void ATPS_ShooterCharacter::InputAttackReleased()
{
	AttackCharEvent(false);
}

void ATPS_ShooterCharacter::MovementTick(float DeltaTime)
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

void ATPS_ShooterCharacter::InitWeapon(FName IdWeaponName)
{
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
				SpawnParams.Owner = GetOwner();
				SpawnParams.Instigator = GetInstigator();

				AWeaponDefault* myWeapon = Cast<AWeaponDefault>(GetWorld()->SpawnActor(myWeaponInfo.WeaponClass, &SpawnLocation, &SpawnRotation, SpawnParams));
				if (myWeapon)
				{
					FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
					myWeapon->AttachToComponent(GetMesh(), Rule, FName("WeaponSocketRightHand"));
					CurrentWeapon = myWeapon;

					myWeapon->WeaponSetting = myWeaponInfo;
					myWeapon->WeaponInfo.Round = myWeaponInfo.MaxRound;
					//Remove !!! Debug
					myWeapon->ReloadTime = myWeaponInfo.ReloadTime;
					myWeapon->UpdateStateWeapon(MovementState);
					myWeapon->OnWeaponReloadStart.AddDynamic(this, &ATPS_ShooterCharacter::WeaponReloadStart);
					myWeapon->OnWeaponReloadEnd.AddDynamic(this, &ATPS_ShooterCharacter::WeaponReloadEnd);
					myWeapon->OnWeaponFireStart.AddDynamic(this, & ATPS_ShooterCharacter::WeaponFireStart);
					myWeapon->UpdateWeaponComponent(); // Обновляем привязку ShootLocation
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
	if (CurrentWeapon && !CurrentWeapon->WeaponReloading)
	{
		if (CurrentWeapon->GetWeaponRound() <= CurrentWeapon->WeaponSetting.MaxRound)
			CurrentWeapon->InitReload();
	}
}

void ATPS_ShooterCharacter::WeaponReloadStart(UAnimMontage* Anim)
{
	WeaponReloadStart_BP(Anim);
}

void ATPS_ShooterCharacter::WeaponReloadEnd()
{
	WeaponReloadEnd_BP();
}

void ATPS_ShooterCharacter::WeaponReloadStart_BP_Implementation(UAnimMontage* Anim)
{
	// in BP
}

void ATPS_ShooterCharacter::WeaponReloadEnd_BP_Implementation()
{
	// in BP
}

void ATPS_ShooterCharacter::WeaponFireStart(UAnimMontage* Anim)
{
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
