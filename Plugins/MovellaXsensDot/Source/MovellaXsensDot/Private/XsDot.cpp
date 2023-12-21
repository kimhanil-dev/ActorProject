// Fill out your copyright notice in the Description page of Project Settings.


#include "XsDot.h"
#include "XsDefines.h"
#include "XsDotCallbackBridge.h"

#include "movelladot_pc_sdk.h"

#include "Misc/MessageDialog.h"

#include "GeometryCollection/GeometryCollectionComponent.h"

// Sets default values for this component's properties
UXsDot::UXsDot()
{
	UGeometryCollectionComponent component;
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

void UXsDot::AddBindingManager(UXsBindingManager* bindingManager)
{
	XsBindingManagers.Push(bindingManager);
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


void UXsDot::GetDetectedDeviceName(TArray<FXsPortInfo>& devices) const
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
	while (ThreadCounter > 0)
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
	for (auto[key, value] : XsDotQuats)
	{
		if (!XsDotHelper->GetQuaternionData(key, value))
		{
			return;
		}

		for (auto& bindingManager : XsBindingManagers)
		{
			bindingManager->Update(key, value);
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


bool UXsBindingManager::Bind(FString tag, const FXsBindingInfo& bindingInfo, const bool isAnonymous)
{
	const FString& bluetoothdAddress = bindingInfo.TargetXsDeviceBluetoothAddress;

	if (!isAnonymous)
	{
		BindingInfos.Add(tag, bindingInfo);
	}

	if (BindingInfosDeviceAddressBased.Contains(bluetoothdAddress))
	{
		BindingInfosDeviceAddressBased[bluetoothdAddress].Push(bindingInfo);
	}
	else
	{
		BindingInfosDeviceAddressBased.Add(bluetoothdAddress).Push(bindingInfo);
	}

	return true;
}

bool UXsBindingManager::SetEnable(FString tag,bool isEnable)
{
	FXsBindingInfo* bindingInfo = GetBindingInfo(tag);
	if (bindingInfo != nullptr)
	{
		bindingInfo->bIsEnabled = true;
		return true;
	}
	else
	{
		return false;
	}
}

bool UXsBindingManager::SetLocal(FString tag, bool isLocal)
{
	FXsBindingInfo* bindingInfo = GetBindingInfo(tag);
	if (bindingInfo != nullptr)
	{
		bindingInfo->bIsLocal = true;
		return true;
	}
	else
	{
		return false;
	}
}

void UXsBindingManager::Update(const FString& deviceBluetoothAddress, const FQuat& rotation)
{
	if (BindingInfosDeviceAddressBased.Contains(deviceBluetoothAddress))
	{
		auto& target = BindingInfosDeviceAddressBased[deviceBluetoothAddress];
		for (auto& bindingInfo : target)
		{
			if (bindingInfo.bIsEnabled)
			{
				if (bindingInfo.bIsLocal)
				{
					bindingInfo.SceneComponent->SetRelativeRotation(rotation);
				}
				else
				{
					bindingInfo.SceneComponent->SetWorldRotation(rotation);
				}
			}
		}
	}
}

FXsBindingInfo* UXsBindingManager::GetBindingInfo(const FString& tag)
{
	if (!BindingInfos.Contains(tag))
	{
		return nullptr;
	}

	return &BindingInfos[tag];
}
