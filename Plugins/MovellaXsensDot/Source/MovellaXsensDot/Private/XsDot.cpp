// Fill out your copyright notice in the Description page of Project Settings.


#include "XsDot.h"
#include "XsDefines.h"
#include "XsDotCallbackBridge.h"

#include "movelladot_pc_sdk.h"

#include "Misc/MessageDialog.h"

// Sets default values for this component's properties
UXsDot::UXsDot()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	XsDotHelper = new XsDotCallbackBridge();
	XsDotHelper->Initialize();
	XsDotHelper->RegisterListener(this);
}

#pragma region UFUNCTIONs

void UXsDot::StartScanning(const FOnDeviceDetected& onDeviceDetected)
{
	OnDeviceDetected = onDeviceDetected;
	XsDotHelper->StartScanning();
}

void UXsDot::StopScanning()
{
	XsDotHelper->StopScanning();
}

void UXsDot::ResetOrientation()
{
	XsDotHelper->ResetOrientation();
}

FVector UXsDot::XsDotVectorToUEVector(const FVector xsVector)
{
	FVector ueVector;
	ueVector.X = xsVector.Y;
	ueVector.Y = xsVector.X;
	ueVector.Z = xsVector.Z;

	return ueVector;
}

FRotator UXsDot::XsDotRotatorToUERotator(const FRotator xsRotator)
{
	return FRotator(xsRotator.Roll, -xsRotator.Yaw , xsRotator.Pitch);
}

bool UXsDot::BindToXsDot(const FString deviceAddress, UXsBindingComponent* target)
{
	for (const auto& device : XsDotHelper->GetDetectedDevices())
	{
		if(XDSTR_TO_UESTR(device.bluetoothAddress()).Equals(deviceAddress))
		{
			XsDotBindingComps.Add(deviceAddress, target);
			return true;
		}
	}

	XDLOG(Error, TEXT("Device with address %s not found"), *deviceAddress);

	return false;
}

void UXsDot::ConnectDevices(const FOnDeviceConnectionResult& onDeviceConnectionResult)
{
	OnDeviceConnectionResult = onDeviceConnectionResult;
	
	UE_LOG(XsDot,Log,TEXT("ConnectDevices function called"));

	for (auto& device : XsDotHelper->GetDetectedDevices())
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
		[&XsDotHelper = this->XsDotHelper
		, &OnDeviceConnectionResult = this->OnDeviceConnectionResult
		, &ThreadCounter = this->ThreadCounter
		, &bIsGameEnd = this->bIsGameEnd
		, &XsDotQuats = this->XsDotQuats
		, _deviceAddress = XDSTR_TO_UESTR(device.bluetoothAddress())]()
		{

			++ThreadCounter;
			bool result = false;
			auto deviceConnector = new FAsyncTask<FAsyncConnectDevices>(*XsDotHelper, _deviceAddress, result);
			deviceConnector->StartBackgroundTask();
			deviceConnector->EnsureCompletion();
			delete deviceConnector;
			--ThreadCounter;

			AsyncTask(ENamedThreads::GameThread, [&, __deviceAddress = _deviceAddress]()
			{
				if (!bIsGameEnd)
				{
					OnDeviceConnectionResult.Execute(__deviceAddress, result);
					XsDotQuats.Add(__deviceAddress, FQuat::Identity);
				}

			});
		});
	}
}

void UXsDot::ConnectDevice(const FString deviceAddress, const FOnDeviceConnectionResult& onDeviceConnectionResult, bool& deviceNotFound)
{
	OnDeviceConnectionResult = onDeviceConnectionResult;
	deviceNotFound = false;

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, 
	[&XsDotHelper = this->XsDotHelper
	, &OnDeviceConnectionResult = this->OnDeviceConnectionResult
	, &ThreadCounter = this->ThreadCounter
	, &bIsGameEnd = this->bIsGameEnd
	, &XsDotQuats = this->XsDotQuats
	, _deviceAddress = deviceAddress]()
	{
		++ThreadCounter;
		bool result = false;
		auto deviceConnector = new FAsyncTask<FAsyncConnectDevices>(*XsDotHelper, _deviceAddress, result);
		deviceConnector->StartBackgroundTask();
		deviceConnector->EnsureCompletion();
		delete deviceConnector;
		--ThreadCounter;

		AsyncTask(ENamedThreads::GameThread, [&, __deviceAddress = _deviceAddress, _result = result]()
		{
			if (!bIsGameEnd)
			{
				OnDeviceConnectionResult.Execute(__deviceAddress, _result);
				XsDotQuats.Add(__deviceAddress, FQuat::Identity);
			}

		});
	});

	
	//deviceNotFound = true;
}

void UXsDot::StartMeasurement(const FString deviceAddress, bool& isSuccess)
{
	isSuccess = XsDotHelper->StartMeasurement(deviceAddress);
}

#pragma endregion UFUNCTIONs


void UXsDot::GetDetectedDeviceName(TArray<FXsPortInfo>& devices)
{
	for (const XsPortInfo& device : XsDotHelper->GetDetectedDevices())
	{
		devices.Add(FXsPortInfo(device));
	}
}

bool UXsDot::GetQuaternionData(const FString& devicebluetoothAddress, FQuat& quat)
{
	FQuat* result = XsDotQuats.Find(devicebluetoothAddress);
	if (result != nullptr)
	{
		quat = *result;
		return true;
	}

	return false;
}

void UXsDot::SetLiveDataOutputRate(const EOutputRate& rate)
{
	XsDotHelper->SetLiveDataOutputRate(rate);
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

	// wait for thread complite
	FMessageDialog::Open(EAppMsgType::Type::Ok, FText::FromString(FString::Printf(TEXT("Waiting for thread safe cleanup..."))));
	bIsGameEnd = true;
	while(ThreadCounter > 0)
	{
		FPlatformProcess::Sleep(0.1f);
	}

	if (XsDotHelper != nullptr)
	{
		XsDotHelper->Cleanup();
		delete XsDotHelper;
	}
}

// Called every frame
void UXsDot::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...

	//Update binded scene components rotation
	for (auto& xsDotQuat : XsDotQuats)
	{
		if (!XsDotHelper->GetQuaternionData(xsDotQuat.Key, xsDotQuat.Value))
		{
			return;
		}

		auto bindingComp = XsDotBindingComps.Find(xsDotQuat.Key);
		if(bindingComp == nullptr)
		{
			continue;
		}
		else if((*bindingComp)->GetBindingEnabled())
		{
			if ((*bindingComp)->GetIsLocal())
			{
				(*bindingComp)->SceneComponent->SetRelativeRotation(xsDotQuat.Value);
			}
			else
			{
				(*bindingComp)->SceneComponent->SetWorldRotation(xsDotQuat.Value);
			}
		}
	}
}

#pragma endregion UE Events


#pragma region IXsDotCallbackListener

void UXsDot::OnAdvertisementFound(const FXsPortInfo& portInfo)
{
	AsyncTask(ENamedThreads::GameThread, [=]()
	{
		OnDeviceDetected.Execute(portInfo.BluetoothAddress);
	});
}

void UXsDot::onButtonClicked(XsDotDevice* device, uint32_t timestamp)
{
	if (device != nullptr)
	{
		AsyncTask(ENamedThreads::GameThread, [=]()
		{
			if (!bIsGameEnd)
			{
				OnButtonClicked.Broadcast(XDSTR_TO_UESTR(device->bluetoothAddress()), static_cast<int>(timestamp));
			}
		});
}
}

void UXsDot::OnError(const XsResultValue result, const FString error)
{
}

#pragma endregion IXsDotCallbackListener

void FAsyncConnectDevices::DoWork()
{
	Result = XsDotHelper.ConnectDot(DeviceAddress);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void UXsBindingComponent::Init(USceneComponent* comp, bool isLocal = false, bool isEnabled = true)
{
	SceneComponent = comp;
	bIsLocal = isLocal;
	bIsEnabled = isEnabled;
}