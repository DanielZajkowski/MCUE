// Fill out your copyright notice in the Description page of Project Settings.

#include "Wieldable.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "MCUECharacter.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AWieldable::AWieldable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WieldableMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WieldableMesh"));
	RootComponent = WieldableMesh;

	PickupTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("PickupTrigger"));
	//PickupTrigger->GetGenerateOverlapEvents(true);
	PickupTrigger->SetGenerateOverlapEvents(true);
	PickupTrigger->OnComponentBeginOverlap.AddDynamic(this, &AWieldable::OnRadiusEnter);
	PickupTrigger->SetupAttachment(RootComponent);

	MaterialType = EMaterial::None;
	ToolType = ETool::Unarmed;

	bIsActive = true;

}

// Called when the game starts or when spawned
void AWieldable::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWieldable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FRotator Rotation = WieldableMesh->GetComponentRotation();
	Rotation.Yaw += 1.0f;
	WieldableMesh->SetRelativeRotation(Rotation);

}

void AWieldable::OnRadiusEnter(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (bIsActive)
	{
		AMCUECharacter* Character = Cast<AMCUECharacter>(OtherActor);
		//Character->FP_WieldedItem->SetSkeletalMesh(WieldableMesh->SkeletalMesh);
		Character->AddItemToInventory(this);

		Hide(true);		
	}
}

void AWieldable::Hide(bool bVis)
{
	WieldableMesh->SetVisibility(!bVis);
	bIsActive = !bVis;
}

void AWieldable::OnUsed()
{
	Destroy();
}

