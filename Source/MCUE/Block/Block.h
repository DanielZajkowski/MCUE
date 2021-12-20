// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include "GameFramework/Actor.h"
#include "Block.generated.h"

UCLASS()
class MCUE_API ABlock : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABlock();


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* SM_Block;

	uint8 MinimumMaterial;

	UPROPERTY(EditDefaultsOnly)
	float Resistance;

	UPROPERTY(BlueprintReadWrite)
	float BreakingStage;

	//called every time we want to break the black down further
	void Break();

	void ResetBlock();

	//called once the block has hit the final breaking stage
	void OnBroken(bool HasRequiredPickaxe);
};
