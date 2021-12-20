// Fill out your copyright notice in the Description page of Project Settings.

#include "Block.h"
#include "Materials/MaterialInstanceDynamic.h"


// Sets default values
ABlock::ABlock()
{
	SM_Block = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bloxk"));

	Resistance = 20.0f;
	BreakingStage = 0.0f;
	MinimumMaterial = 0;
}

// Called when the game starts or when spawned
void ABlock::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABlock::Break()
{
	++BreakingStage;

	float CrackingValue = 1.0f - (BreakingStage / 5.0f);

	UMaterialInstanceDynamic* MatInstance = SM_Block->CreateDynamicMaterialInstance(0);

	if (MatInstance != nullptr)
	{
		MatInstance->SetScalarParameterValue(FName("CrackingValue"), CrackingValue);
	}

	if (BreakingStage == 5.0f)
	{
		OnBroken(true);
	}
}

void ABlock::ResetBlock()
{
	BreakingStage = 0.0f;

	UMaterialInstanceDynamic* MatInstance = SM_Block->CreateDynamicMaterialInstance(0);

	if (MatInstance != nullptr)
	{
		MatInstance->SetScalarParameterValue(FName("CrackingValue"), 1.0f);
	}
}

void ABlock::OnBroken(bool HasRequiredPickaxe)
{
	Destroy();
}


