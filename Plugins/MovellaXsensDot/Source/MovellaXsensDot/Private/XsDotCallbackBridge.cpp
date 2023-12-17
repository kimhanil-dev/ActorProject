#include "XsDotCallbackBridge.h"

XsDotCallbackBridge::XsDotCallbackBridge()
{

}

XsDotCallbackBridge::~XsDotCallbackBridge()
{
}

void XsDotCallbackBridge::RegisterListener(IXsDotCallBackListener* listener)
{
	mListeners.Add(listener);
}

bool XsDotCallbackBridge::Initialize()
{
	// 1. Create a XsDotConnectionManager instance.
	mConnectionManager = XsDotConnectionManager::construct();
	if (mConnectionManager == nullptr)
	{
		XDLOG(Error, TEXT("XsDotConnectionManager construct failed"));
		return false;
	}

	// 2. Register callback 
	mConnectionManager->addXsDotCallbackHandler(this);
	return true;
}

bool XsDotCallbackBridge::StartScanning()
{
	UE_LOG(XsDot,Log, TEXT("Scanning Start..."))

	if(!mConnectionManager->enableDeviceDetection())
	{
		XDLOG(Error, TEXT("Scanning Start failed : %s"), *XDSTR_TO_UESTR(mConnectionManager->lastResultText()));
		return false;
	}

	return true;
}

void XsDotCallbackBridge::StopScanning()
{
	UE_LOG(XsDot,Log, TEXT("Scanning Stop..."));
	mConnectionManager->disableDeviceDetection();
}

XsPortInfoArray XsDotCallbackBridge::DetectUsbDevices()
{
	return mConnectionManager->detectUsbDevices();
}

bool XsDotCallbackBridge::ConnectDot(XsPortInfo portInfo)
{
	if(mConnectionManager == nullptr)
		return false;

	if (portInfo.isBluetooth())
	{
		UE_LOG(XsDot, Log, TEXT("Opening DOT with address: %s"), *XDSTR_TO_UESTR(portInfo.bluetoothAddress()));
		if (!mConnectionManager->openPort(portInfo))
		{
			XDLOG(Log, TEXT("Connection to device failed : %s"), *XDSTR_TO_UESTR(mConnectionManager->lastResultText()));
			return false;
		}

		XsDotDevice* newDevice = mConnectionManager->device(portInfo.deviceId());
		if (newDevice == nullptr)
			return false;

		mConnectedDevices.Add(newDevice);
		UE_LOG(XsDot, Log, TEXT("Found a device with tag: %s @ address: "), *XDSTR_TO_UESTR(newDevice->deviceTagName()), *XDSTR_TO_UESTR(newDevice->bluetoothAddress()));

		auto filterProfiles = newDevice->getAvailableFilterProfiles();
		UE_LOG(XsDot, Log, TEXT("available filter profiles: %d"), filterProfiles.size());
		for (auto& f : filterProfiles)
			UE_LOG(XsDot, Log, TEXT("%s\n"), *FString(f.label()));

		UE_LOG(XsDot, Log, TEXT("Current profile: %s"), *FString(newDevice->onboardFilterProfile().label()));

		if (newDevice->setOnboardFilterProfile(XsString("General")))
		{
			UE_LOG(XsDot, Log, TEXT("Successfully set profile to General"));
		}
		else
		{
			XDLOG(Error, TEXT("Setting filter profile failed!"));
		}

	/*	UE_LOG(XsDot, Log, TEXT("Setting quaternion CSV output"));
		newDevice->setLogOptions(XsLogOptions::Quaternion);

		XsString logFileName = XsString("logfile_") << newDevice->bluetoothAddress().replacedAll(":", "-") << ".csv";
		UE_LOG(XsDot, Log, TEXT("Enable logging to: %s"), *XDSTR_TO_UESTR(logFileName));

		if (!newDevice->enableLogging(logFileName))
			XDLOG(Error, TEXT("Failed to enable logging. Reason: %s"),*XDSTR_TO_UESTR(newDevice->lastResultText()));*/
		
		UE_LOG(XsDot, Log, TEXT("Putting device into measurement mode."))
		if (!newDevice->startMeasurement(XsPayloadMode::ExtendedQuaternion))
		{
			XDLOG(Error, TEXT("Could not put device into measurement mode. Reason: %s"), *XDSTR_TO_UESTR(newDevice->lastResultText()));
		}
	}
	else // usb
	{
		UE_LOG(XsDot,Log, TEXT("Opening DOT with ID: %s @ port: %s, baudrate: %d"), *XDOBJ_TO_UESTR(portInfo.deviceId()), *XDSTR_TO_UESTR(portInfo.portName()), portInfo.baudrate());
		if (!mConnectionManager->openPort(portInfo))
		{
			XDLOG(Log, TEXT("Connection to device failed : %s"), mConnectionManager->lastResultText().c_str());
			return false;
		}

		XsDotUsbDevice* newDevice = mConnectionManager->usbDevice(portInfo.deviceId());
		if (newDevice == nullptr)
			return false;

		mConnectedUsbDevices.Add(newDevice);
		UE_LOG(XsDot,Log, TEXT("Device: %s, With ID: %s opend."), *XDSTR_TO_UESTR(newDevice->productCode()), *XDOBJ_TO_UESTR(newDevice->deviceId()));
	}

	return true;
}

bool XsDotCallbackBridge::ConnectDots(const XsPortInfoArray& portInfos)
{
	bool result = true;
	for (const XsPortInfo& portInfo : portInfos)
	{
		if(!ConnectDot(portInfo))
			result = false;
	}

	return result;
}

bool XsDotCallbackBridge::ConnectAllDots()
{
	return ConnectDots(GetDetectedDevices());
}

XsPortInfoArray XsDotCallbackBridge::GetDetectedDevices()
{
	xsens::Lock locky(&mMutex);
	return DetectedDevices;
}

TArray<const XsDotDevice*> XsDotCallbackBridge::GetConnectedDevices()
{
	TArray<const XsDotDevice*> connectedDevices;
	for (XsDotDevice* device : mConnectedDevices)
	{
		connectedDevices.Add(device);
	}

	return connectedDevices;
}

bool XsDotCallbackBridge::GetLiveData(const FString& deviceBluetoothAddress, FRotator& outRotation, FVector& outAcc, FQuat& quat)
{
	xsens::Lock locky(&mMutex);

	auto& packets = mPackets[deviceBluetoothAddress];
	if(packets.size() == 0)
		return false;
	
	XsDataPacket packetData = packets.front();
	packets.pop_front();
	
	if(packetData.containsOrientation())
	{
		XsEuler euler = packetData.orientationEuler();
		XsQuaternion xsQuat = packetData.orientationQuaternion(XsDataIdentifier::XDI_Quaternion);
		quat = FQuat(xsQuat.x(), xsQuat.y(), xsQuat.z(), xsQuat.w());
		outRotation = FRotator(euler.pitch(), euler.yaw(), euler.roll());
	}

	if (packetData.containsVelocity())
	{
		XsVector velocity = packetData.velocity(XsDataIdentifier::XDI_AngularVelocityGroup);
		outAcc = FVector3d(velocity[0], velocity[1], velocity[2]);
	}

	return true;
}

bool XsDotCallbackBridge::GetQuaternionData(const FString& deviceBluetoothAddress, FQuat& outQuat)
{
	xsens::Lock locky(&mMutex);

	auto& packets = mPackets[deviceBluetoothAddress];
	if (packets.size() == 0)
		return false;

	XsDataPacket packetData = packets.front();
	packets.pop_front();

	// 개발 모드가 아니라면 GetQuaternion은 항상 성공해야 합니다.
#if  defined(UE_BUILD_DEBUG) | defined(UE_EDITOR) | defined(UE_BUILD_DEVELOPMENT)
	if (!packetData.containsOrientation())
		XDLOG(Error, TEXT("packetData does not contain orientation data"));
#endif

	XsQuaternion xsQuat = packetData.orientationQuaternion(XsDataIdentifier::XDI_Quaternion);
	outQuat = FQuat(xsQuat.x(), xsQuat.y(), xsQuat.z(), xsQuat.w());

	return true;
}

void XsDotCallbackBridge::SetLiveDataOutputRate(const EOutputRate& rate)
{
	for (auto* device : mConnectedDevices)
	{
		device->setOutputRate(static_cast<uint8>(rate));
	}
}

void XsDotCallbackBridge::ResetOrientation()
{
	for (auto* device : mConnectedDevices)
	{
		device->resetOrientation(XsResetMethod::XRM_Heading);
	}
}

void XsDotCallbackBridge::onAdvertisementFound(const XsPortInfo* portInfo)
{
	xsens::Lock locky(&mMutex);
	UE_LOG(XsDot, Log, TEXT("Device Detected : %s"), *XDSTR_TO_UESTR(portInfo->bluetoothAddress()));

	DetectedDevices.push_back(*portInfo);
	for(IXsDotCallBackListener* listener : mListeners)
	{
		listener->OnAdvertisementFound(FXsPortInfo(*portInfo));
	}
}

void XsDotCallbackBridge::onError(XsResultValue result, const XsString* error)
{
	UE_LOG(XsDot,Error, TEXT("Error Code : %d. Error : %s"), result, *XDSTR_TO_UESTR((*error)));
	for (IXsDotCallBackListener* listener : mListeners)
	{
		listener->OnError(result,*XDSTR_TO_UESTR((*error)));
	}
}

void XsDotCallbackBridge::onLiveDataAvailable(XsDotDevice* device, const XsDataPacket* packet)
{
	xsens::Lock locky(&mMutex);
	if (packet != nullptr)
	{
		auto& packetDatas = mPackets[*XDSTR_TO_UESTR(device->bluetoothAddress())];
		if (packetDatas.size() >= 30)
		{
			packetDatas.pop_front();
		}
		mPackets[*XDSTR_TO_UESTR(device->bluetoothAddress())].push_back(*packet);
	}
	else
	{
		UE_LOG(XsDot, Error, TEXT("Live data error(Device : %s)'s packet is null"));
	}
}

void XsDotCallbackBridge::onButtonClicked(XsDotDevice* device, uint32_t timestamp)
{
	for (IXsDotCallBackListener* listener : mListeners)
	{
		listener->onButtonClicked(device, timestamp);
	}
}

void XsDotCallbackBridge::Cleanup()
{
	if (!mConnectionManager)
		return;

	UE_LOG(XsDot,Log, TEXT("Closing ports..."));
	mConnectionManager->close();

	UE_LOG(XsDot,Log, TEXT("Freeing XsDotConnectionManager object..."));
	mConnectionManager->destruct();
	mConnectionManager = nullptr;

	UE_LOG(XsDot,Log, TEXT("Successful exit."));
}