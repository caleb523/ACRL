// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "FirstProjectGameMode.h"
#include "FirstProjectPawn.h"

AFirstProjectGameMode::AFirstProjectGameMode()
{
	// set default pawn class to our flying pawn
	DefaultPawnClass = AFirstProjectPawn::StaticClass();
}
