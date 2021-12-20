// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MCUECharacter.generated.h"

class UInputComponent;
class ABlock;
class AWieldable;

UCLASS(config=Game)
class AMCUECharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWieldable> WieldableMesh;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

public:
	AMCUECharacter();

	virtual void BeginPlay();

	virtual void Tick(float DeltaTime) override;

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_WieldedItem;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AMCUEProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	//get current inventory slot for hud
	UFUNCTION(BlueprintPure, Category = "HUD")
	int32 GetCurrentInventorySlot();

	//adds an item to our inventory
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool AddItemToInventory(AWieldable* Item);

	//gets the thumball for a given item
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UTexture2D* GetThumbnailAtInventorySlot(uint8 Slot);

	//the type of tool and tool material of the currently wielded item
	uint8 ToolType;
	uint8 MaterialType;

	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

protected:
	
	/** Fires a projectile. */
	void OnFire();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	void TurnAtRate(float Rate);

	void LookUpAtRate(float Rate);
	
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

private:

	//number of inventory slots
	const int32 NUM_OF_INVENTORY_SLOTS = 10;

	//current inv slots
	int32 CurrentInventorySlots;

	//update the wielded item
	void UpdateWieldedItem();

	//gets the current wielded item
	AWieldable* GetCurrentlyWieldedItem();

	//throws the current wielded item
	void Throw();

	//increment and decrement inventory slots
	void MoveUpInventorySlots();
	void MoveDownInventorySlots();

	//true if player is breaking, false otherwise
	bool bIsBreaking;

	//called when hitting with a tool
	void OnHit();
	void EndHit();

	//plays the minig animation
	void PlayHitAnim();

	//called when we want to break a block
	void BreakBlock();

	//check if there is a block in front of the player
	void CheckForBlocks();

	//stores the block currently being looked at by the player
	ABlock* CurrentBlock;

	//the character reach
	float Reach;

	//timer handle
	FTimerHandle BlockBreakingHandle;
	FTimerHandle HitAnimHandle;

	UPROPERTY(EditAnywhere)
	TArray<AWieldable*> Inventory;

};

