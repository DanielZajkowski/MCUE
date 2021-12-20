// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MCUEGameMode.generated.h"

UCLASS(minimalapi)
class AMCUEGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMCUEGameMode();

	virtual void BeginPlay();

	enum EHUDState : uint8
	{
		HS_Ingame,
		HS_Inventory,
		HS_Craft_Menu
	};

	//check the hud state, and then calls applyhud to apply whatever hud we've using
	void ApplyHUDChanges();

	//gets the hud state
	uint8 GetHUDState();

	//setter function for hudstate, applies the new state and then calls applyhudchanges
	UFUNCTION(BlueprintCallable, Category = "HUD Functions")
	void ChangeHUDState(uint8 NewState);

	//applies a hud to the screen, returns true if successful, false otherwise
	bool ApplyHUD(TSubclassOf<class UUserWidget> WidgetToApply, bool ShowMouseCursor, bool EnableClickEvents);

protected:

	//the current hudstate
	uint8 HUDState;

	//the hud to be show Ingame
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Blueprint Widgets", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> IngameHUDClass;

	//the hud to be show inventory
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Blueprint Widgets", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> InventoryHUDClass;

	//the hud to be show crafting menu
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Blueprint Widgets", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> CraftMenuHUDClass;

	//the current hud being displayed on the screen
	class UUserWidget* CurrentWidget;
};



