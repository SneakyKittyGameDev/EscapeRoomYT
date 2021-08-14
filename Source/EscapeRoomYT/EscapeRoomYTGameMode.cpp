// Copyright Epic Games, Inc. All Rights Reserved.

#include "EscapeRoomYTGameMode.h"
#include "EscapeRoomYTCharacter.h"
#include "UObject/ConstructorHelpers.h"

AEscapeRoomYTGameMode::AEscapeRoomYTGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
