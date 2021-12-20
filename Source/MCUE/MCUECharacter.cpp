// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "MCUECharacter.h"
#include "MCUEProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "Block/Block.h"
#include "TimerManager.h"
#include "Wieldable/Wieldable.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AMCUECharacter

AMCUECharacter::AMCUECharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.0f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_WieldedItem = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_WieldedItem"));
	FP_WieldedItem->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_WieldedItem->bCastDynamicShadow = false;
	FP_WieldedItem->CastShadow = false;
	// FP_WieldedItem->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_WieldedItem->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_WieldedItem);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_WieldedItem, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	Reach = 250.0f;

}

void AMCUECharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_WieldedItem->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

}

void AMCUECharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckForBlocks();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMCUECharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//wheel mouse
	PlayerInputComponent->BindAction("InventoryUp", IE_Pressed, this, &AMCUECharacter::MoveUpInventorySlots);
	PlayerInputComponent->BindAction("InventoryDown", IE_Pressed, this, &AMCUECharacter::MoveDownInventorySlots);

	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &AMCUECharacter::Throw);

	// Bind fire event
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMCUECharacter::OnHit);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &AMCUECharacter::EndHit);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AMCUECharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMCUECharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMCUECharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMCUECharacter::LookUpAtRate);
}

void AMCUECharacter::UpdateWieldedItem()
{
	Inventory[CurrentInventorySlots] != NULL ? FP_WieldedItem->SetSkeletalMesh(Inventory[CurrentInventorySlots]->WieldableMesh->SkeletalMesh) : FP_WieldedItem->SetSkeletalMesh(NULL);
}

AWieldable * AMCUECharacter::GetCurrentlyWieldedItem()
{
	return Inventory[CurrentInventorySlots] != NULL ? Inventory[CurrentInventorySlots] : nullptr;
}

void AMCUECharacter::Throw()
{
	//get the currently wielded item
	AWieldable* ItemToThrow = GetCurrentlyWieldedItem();

	//raycast to find drop location
	FHitResult LinetraceHit;

	FVector StartTrace = FirstPersonCameraComponent->GetComponentLocation();
	FVector EndTrace = (FirstPersonCameraComponent->GetForwardVector() * Reach) + StartTrace;

	FCollisionQueryParams CQP;
	CQP.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(LinetraceHit, StartTrace, EndTrace, ECollisionChannel::ECC_WorldDynamic, CQP);

	FVector DropLocation = EndTrace;

	if (LinetraceHit.GetActor() != NULL)
	{
		DropLocation = (LinetraceHit.ImpactPoint + 20.0f);
	}

	if (ItemToThrow != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			ItemToThrow->SetActorLocationAndRotation(DropLocation, FRotator::ZeroRotator);
			ItemToThrow->Hide(false);
			Inventory[CurrentInventorySlots] = NULL;
		}
	}

	UpdateWieldedItem();
}

void AMCUECharacter::MoveUpInventorySlots()
{
	CurrentInventorySlots = FMath::Abs((CurrentInventorySlots + 1) % NUM_OF_INVENTORY_SLOTS);
	UpdateWieldedItem();
}

void AMCUECharacter::MoveDownInventorySlots()
{
	if (CurrentInventorySlots == 0)
	{
		CurrentInventorySlots = 9; 
		UpdateWieldedItem();
		return;
	}
	CurrentInventorySlots = FMath::Abs((CurrentInventorySlots + 1) % NUM_OF_INVENTORY_SLOTS);
	UpdateWieldedItem();
}

void AMCUECharacter::OnHit()
{
	PlayHitAnim();

	if (CurrentBlock != nullptr)
	{
		bIsBreaking = true;

		float TimeBetweenBreaks = ((CurrentBlock->Resistance) / 100.f) / 2;

		GetWorld()->GetTimerManager().SetTimer(BlockBreakingHandle, this, &AMCUECharacter::BreakBlock, TimeBetweenBreaks, true);
		GetWorld()->GetTimerManager().SetTimer(HitAnimHandle, this, &AMCUECharacter::PlayHitAnim, 0.4f, true);
	}
}

void AMCUECharacter::EndHit()
{
	GetWorld()->GetTimerManager().ClearTimer(BlockBreakingHandle);
	GetWorld()->GetTimerManager().ClearTimer(HitAnimHandle);

	bIsBreaking = false;

	if (CurrentBlock != nullptr)
	{
		CurrentBlock->ResetBlock();
	}
}

void AMCUECharacter::PlayHitAnim()
{
	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AMCUECharacter::BreakBlock()
{
	if (bIsBreaking && CurrentBlock != nullptr && !CurrentBlock->IsPendingKill())
	{
		CurrentBlock->Break();
	}
}

void AMCUECharacter::CheckForBlocks()
{
	FHitResult LinetraceHit;

	FVector StartTrace = FirstPersonCameraComponent->GetComponentLocation();
	FVector EndTrace = (FirstPersonCameraComponent->GetForwardVector() * Reach) + StartTrace;

	FCollisionQueryParams CQP;
	CQP.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(LinetraceHit, StartTrace, EndTrace, ECollisionChannel::ECC_WorldDynamic, CQP);

	ABlock* PotentialBlock = Cast<ABlock>(LinetraceHit.GetActor());

	if (PotentialBlock != CurrentBlock && CurrentBlock != nullptr)
	{
		CurrentBlock->ResetBlock();
	}

	if (PotentialBlock == NULL)
	{
		CurrentBlock = nullptr;
		return;
	}
	else
	{
		if (CurrentBlock != nullptr && !bIsBreaking)
		{
			CurrentBlock->ResetBlock();
		}
		CurrentBlock = PotentialBlock;
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, *CurrentBlock->GetName());
	}
}

int32 AMCUECharacter::GetCurrentInventorySlot()
{
	return CurrentInventorySlots;
}

bool AMCUECharacter::AddItemToInventory(AWieldable * Item)
{
	if (Item != NULL)
	{
		const int32 AvailableSlot = Inventory.Find(nullptr);

		if (AvailableSlot != INDEX_NONE)
		{
			Inventory[AvailableSlot] = Item;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

UTexture2D * AMCUECharacter::GetThumbnailAtInventorySlot(uint8 Slot)
{
	if (Inventory[Slot] != NULL)
	{
		return Inventory[Slot]->PickupThumbnail;
	}
	else
	{
		return false;
	}
}

void AMCUECharacter::OnFire()
{
	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}


void AMCUECharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMCUECharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMCUECharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMCUECharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
