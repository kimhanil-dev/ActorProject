// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

THIRD_PARTY_INCLUDES_START
#include "movelladot_pc_sdk.h"
THIRD_PARTY_INCLUDES_END

#include "Modules/ModuleManager.h"

class FMovellaXsesnDotCallBack;
class FMovellaXsensDotModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void* DLLHandle = nullptr;
	XsDotConnectionManager* mXsDotConnectionManager = nullptr;
	FMovellaXsesnDotCallBack* mXsDotCallBack = nullptr;
};

class FMovellaXsesnDotCallBack : public XsDotCallback
{
public:
	FMovellaXsesnDotCallBack() {}
	~FMovellaXsesnDotCallBack() {}

	//! \copydoc m_onAdvertisementFound
	virtual void onAdvertisementFound(const XsPortInfo* portInfo) override
	{
		(void)portInfo;
		m_onAdvertisementFound = 0;
	}
};