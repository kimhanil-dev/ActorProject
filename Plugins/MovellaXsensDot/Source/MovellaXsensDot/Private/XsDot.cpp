// Fill out your copyright notice in the Description page of Project Settings.


#include "XsDot.h"
#include "XsDefines.h"
#include "XsDotCallbackBridge.h"

#include "movelladot_pc_sdk.h"

// Sets default values for this component's properties
UXsDot::UXsDot()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	XsDotHelper = TUniquePtr<XsDotCallbackBridge>(new XsDotCallbackBridge());
	XsDotHelper->Initialize();
	XsDotHelper->RegisterListener(this);
}

#pragma region UFUNCTIONs

void UXsDot::StartScanning()
{
	XsDotHelper->StartScanning();
}

void UXsDot::StopScanning()
{
	XsDotHelper->StopScanning();
}

void UXsDot::ConnectDevices()
{
	XsDotHelper->ConnectAllDots();
}

#pragma endregion UFUNCTIONs


void UXsDot::GetDetectedDeviceName(TArray<FXsPortInfo>& devices)
{
	for (const XsPortInfo& device : XsDotHelper->GetDetectedDevices())
	{
		devices.Add(FXsPortInfo(device));
	}
}

void UXsDot::GetLiveData(const FString deviceBluetoothAddress, FVector& rotation, FVector& acceleration, bool& valid)
{
	valid = XsDotHelper->GetLiveData(deviceBluetoothAddress, rotation, acceleration);
}

#pragma region UE Events

// Called when the game starts
void UXsDot::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

void UXsDot::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	XsDotHelper->Cleanup();
}

// Called every frame
void UXsDot::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

#pragma endregion UE Events


#pragma region IXsDotCallbackListener

void UXsDot::OnAdvertisementFound(const FXsPortInfo& portInfo)
{
}

void UXsDot::OnError(const XsResultValue result, const FString error)
{
}

#pragma endregion IXsDotCallbackListener