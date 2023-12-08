// Copyright Epic Games, Inc. All Rights Reserved.

#include "MovellaXsensDot.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "Windows/WindowsHWrapper.h"

#include <CoreMinimal.h>

#define LOCTEXT_NAMESPACE "FMovellaXsensDotModule"

void FMovellaXsensDotModule::StartupModule()
{
	
	xstypeDLLHandle = FPlatformProcess::GetDllHandle(TEXT("xstypes64.dll"));
	xsDotDLLHandle = FPlatformProcess::GetDllHandle(TEXT("movelladot_pc_sdk64.dll"));
	if (xsDotDLLHandle == nullptr)
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

		FMessageDialog::Open(EAppMsgCategory::Error,EAppMsgType::Type::Ok, FText::FromString(FString::Printf(TEXT("DLL load failed : %s (error : %d) \n %s"), "movelladot_pc_sdk64.dll",errorCode, *errorAdvice)));
	}
}

void FMovellaXsensDotModule::ShutdownModule()
{
	if(xsDotDLLHandle != nullptr)
		FPlatformProcess::FreeDllHandle(xsDotDLLHandle);

	if (xstypeDLLHandle != nullptr)
		FPlatformProcess::FreeDllHandle(xstypeDLLHandle);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMovellaXsensDotModule, MovellaXsensDot)
