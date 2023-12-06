// Copyright Epic Games, Inc. All Rights Reserved.

#include "MovellaXsensDot.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"

#include <CoreMinimal.h>

// plugin headers
#include "../MovellaXsensDotPublicHeader.h"

#define LOCTEXT_NAMESPACE "FMovellaXsensDotModule"

void FMovellaXsensDotModule::StartupModule()
{
	// 1. ConnectionManager 생성
	mXsDotConnectionManager = XsDotConnectionManager::construct();

	// 2. CallBack 등록
	mXsDotCallBack = new FMovellaXsesnDotCallBack();
	mXsDotConnectionManager->addXsDotCallbackHandler(mXsDotCallBack);
}

void FMovellaXsensDotModule::ShutdownModule()
{
	if (mXsDotConnectionManager != nullptr)
	{
		mXsDotConnectionManager->destruct();
		mXsDotConnectionManager = nullptr;
	}

	if (mXsDotCallBack != nullptr)
	{
		delete mXsDotCallBack;
		mXsDotCallBack = nullptr;
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMovellaXsensDotModule, MovellaXsensDot)
