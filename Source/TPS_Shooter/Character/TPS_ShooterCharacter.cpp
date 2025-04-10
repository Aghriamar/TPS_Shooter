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

	/*if (CursorToWorld != nullptr)
	{
		if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		{
			if (UWorld* World = GetWorld())
			{
				FHitResult HitResult;
				FCollisionQueryParams Params(NAME_None, FCollisionQueryParams::GetUnknownStatId());
				FVector StartLocation = TopDownCameraComponent->GetComponentLocation();
				FVector EndLocation = TopDownCameraComponent->GetComponentRotation().Vector() * 2000.0f;
				Params.AddIgnoredActor(this);
				World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params);
				FQuat SurfaceRotation = HitResult.ImpactNormal.ToOrientationRotator().Quaternion();
				CursorToWorld->SetWorldLocationAndRotation(HitResult.Location, SurfaceRotation);
			}
		}
		else if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FHitResult TraceHitResult;
			PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();
			CursorToWorld->SetWorldLocation(TraceHitResult.Location);
			CursorToWorld->SetWorldRotation(CursorR);
		}
	}*/

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
	InitWeapon();
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
		FHitResult ResultHit;
		//MyController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery6, false, ResultHit);
		MyController->GetHitResultUnderCursor(ECC_GameTraceChannel1, true, ResultHit);
		float FindRotatorResultYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ResultHit.Location).Yaw;
		SetActorRotation(FQuat(FRotator(0.0f, FindRotatorResultYaw, 0.0f)));
	}

	if (MovementState == EMovementState::SprintRun_State)
	{
		CurrentStamina -= StaminaDecreaseRate * DeltaTime;
		if (CurrentStamina < 0.0f)
		{
			CurrentStamina = 0.0f;
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

void ATPS_ShooterCharacter::InitWeapon()
{
	if (InitWeaponClass)
	{
		FVector SpawnLocation = FVector(0);
		FRotator SpawnRotation = FRotator(0);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = GetInstigator();

		AWeaponDefault* myWeapon = Cast<AWeaponDefault>(GetWorld()->SpawnActor(InitWeaponClass, &SpawnLocation, &SpawnRotation, SpawnParams));
		if (myWeapon)
		{
			FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
			myWeapon->AttachToComponent(GetMesh(), Rule, FName("WeaponSocketRightHand"));
			CurrentWeapon = myWeapon;
			myWeapon->UpdateStateWeapon(MovementState);
			myWeapon->UpdateWeaponComponent(); // Обновляем привязку ShootLocation
		}
	}
}

UDecalComponent* ATPS_ShooterCharacter::GetCursorToWorld()
{
	return CurrentCursor;
}
