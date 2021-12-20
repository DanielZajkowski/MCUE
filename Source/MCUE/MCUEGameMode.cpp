// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "MCUEGameMode.h"
#include "MCUEHUD.h"
#include "MCUECharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AMCUEGameMode::AMCUEGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AMCUEHUD::StaticClass();

	HUDState = EHUDState::HS_Ingame;
}

void AMCUEGameMode::BeginPlay()
{
	Super::BeginPlay();

	ApplyHUDChanges();
}

void AMCUEGameMode::ApplyHUDChanges()
{
	//remove the previous hud, since we're applying a new one
	if (CurrentWidget != nullptr)
	{
		CurrentWidget->RemoveFromParent();
	}

	/*check hudstate, and apply the hud corresponding to whatever hud should be open*/
	switch (HUDState)
	{
		case EHUDState::HS_Ingame:
		{
			ApplyHUD(IngameHUDClass, false, false);
		}
		case EHUDState::HS_Inventory:
		{
			ApplyHUD(InventoryHUDClass, true, true);
		}
		case EHUDState::HS_Craft_Menu:
		{
			ApplyHUD(CraftMenuHUDClass, true, true);
		}
		default:
		{
			ApplyHUD(IngameHUDClass, false, false);
		}
	}
}

uint8 AMCUEGameMode::GetHUDState()
{
	return HUDState;
}

void AMCUEGameMode::ChangeHUDState(uint8 NewState)
{
	HUDState = NewState;
	ApplyHUDChanges();
}

bool AMCUEGameMode::ApplyHUD(TSubclassOf<class UUserWidget> WidgetToApply, bool ShowMouseCursor, bool EnableClickEvents)
{
	/*get a reference to the player, and the player controller*/
	AMCUECharacter* MyCharacter = Cast<AMCUECharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	APlayerController* MyController = GetWorld()->GetFirstPlayerController();

	/*Nullcheck the widget before applying it*/
	if (WidgetToApply != nullptr)
	{
		/*set mouse events and visibility according to the parameters taken by the function*/
		MyController->bShowMouseCursor = ShowMouseCursor;
		MyController->bEnableClickEvents = EnableClickEvents;

		/*create the widget*/
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), WidgetToApply);

		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
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
