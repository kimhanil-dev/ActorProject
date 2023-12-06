// Copyright Epic Games, Inc. All Rights Reserved.

#include "MovellaXsensDot.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"

#include <CoreMinimal.h>

#define LOCTEXT_NAMESPACE "FMovellaXsensDotModule"

void FMovellaXsensDotModule::StartupModule()
{
	FString dllName = TEXT("movelladot_pc_sdk64.dll");

	DLLHandle = FPlatformProcess::GetDllHandle(*dllName);
	if (DLLHandle == nullptr)
	{
		FString errorAdvice;
		DWORD errorCode = GetLastError();
		switch (errorCode)
		{
		case ERROR_FILE_NOT_FOUND:
		{
			errorAdvice = TEXT("Make sure the DLL is in these directories. \n\n");

			TArray<FString> dllDirectories;
			FPlatformProcess::GetDllDirectories(dllDirectories);
			for (const FString& directory : dllDirectories)
			{
				errorAdvice += directory + '\n';
			}
		}
			break;
		default:
			break;
		}

		FMessageDialog::Open(EAppMsgCategory::Error,EAppMsgType::Type::Ok, FText::FromString(FString::Printf(TEXT("DLL load failed : %s (error : %d) \n %s"), *dllName,errorCode, *errorAdvice)));

		assert(true);
	}
}

void FMovellaXsensDotModule::ShutdownModule()
{
	if(DLLHandle != nullptr)
		FPlatformProcess::FreeDllHandle(DLLHandle);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMovellaXsensDotModule, MovellaXsensDot)
