// Copyright Epic Games, Inc. All Rights Reserved.

#include "MotioncaptureGameMode.h"
#include "MotioncaptureCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMotioncaptureGameMode::AMotioncaptureGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
