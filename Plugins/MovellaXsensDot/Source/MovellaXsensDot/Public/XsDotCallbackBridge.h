#pragma once

#include "CoreMinimal.h"

#include <list>
#include <map>

#include "movelladot_pc_sdk.h"
#include "xscommon/xsens_mutex.h"

#include "XsDefines.h"

#include "XsDotCallbackBridge.generated.h"

USTRUCT(BlueprintType)
struct FXsPortInfo
{
	GENERATED_USTRUCT_BODY()

	FXsPortInfo(){}
	FXsPortInfo(const XsPortInfo& portInfo)
		:PortName(*XDSTR_TO_UESTR(portInfo.portName()))
		, DeviceID(*XDOBJ_TO_UESTR(portInfo.deviceId()))
		, BluetoothAddress(*XDSTR_TO_UESTR(portInfo.bluetoothAddress()))
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Xsens")
	FString PortName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Xsens")
	FString DeviceID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Xsens")
	FString BluetoothAddress;
};

class IXsDotCallBackListener
{
public:
	virtual void OnAdvertisementFound(const FXsPortInfo& portInfo) = 0;
	virtual void OnError(const XsResultValue result, const FString error) = 0;
	// �� ���� ������ ���Źް� ������ �������̽��� �߰��� ������ ��
};

class XsDotCallbackBridge : public XsDotCallback
{
public:
	XsDotCallbackBridge();
	~XsDotCallbackBridge();

	// �ݹ� ���� ���
	// �������̽��� ��ӹ��� Ŭ������ ����ϸ� �ش� Ŭ������ �Լ��� ȣ���
	void RegisterListener(IXsDotCallBackListener* listener);

	bool Initialize();

	// bluethooth device detection
	// call StopScanning() function, you want to stop scanning 
	bool StartScanning();
	void StopScanning();
	// usb device detection
	XsPortInfoArray DetectUsbDevices();

	bool ConnectDot(XsPortInfo& portInfo);
	//todo : connection�� �������� ��쿡 ��� �˸� ������?
	void ConnectDots(XsPortInfoArray& portInfos);
	//todo : connection�� �������� ��쿡 ��� �˸� ������?
	void ConnectAllDots();

	XsPortInfoArray GetDetectedDevices();
	TArray<const XsDotDevice*> GetConnectedDevices();

	// packet���� �����͸� �����ϴ� �Լ���
	// �����Ͱ� ���ų� ���⿡ �����ϸ� false�� ������
	bool GetLiveData(const FString& deviceBluetoothAddress, FVector& outRotation, FVector& outAcc);

	void Cleanup();

protected:
	
	XsDotConnectionManager* mConnectionManager;
	XsPortInfoArray DetectedDevices;
	TArray<XsDotDevice*> mConnectedDevices;
	TArray<XsDotUsbDevice*> mConnectedUsbDevices;
	TArray<IXsDotCallBackListener*> mListeners;
	std::map<FString,std::list<XsDataPacket>> mPackets;
	mutable xsens::Mutex mMutex;

	void onAdvertisementFound(const XsPortInfo* portInfo) override;
	void onError(XsResultValue result, const XsString* error) override;
	//void onBatteryUpdated(XsDotDevice* device, int batteryLevel, int chargingStatus) override;
	void onLiveDataAvailable(XsDotDevice* device, const XsDataPacket* packet) override;
	//void onProgressUpdated(XsDotDevice* device, int current, int total, const XsString* identifier) override;
	//void onDeviceUpdateDone(const XsPortInfo* portInfo, XsDotFirmwareUpdateResult result) override;
	//void onRecordingStopped(XsDotDevice* device) override;
	//void onDeviceStateChanged(XsDotDevice* device, XsDeviceState newState, XsDeviceState oldState) override;
	//void onButtonClicked(XsDotDevice* device, uint32_t timestamp) override;
	//void onProgressUpdated(XsDotUsbDevice* device, int current, int total, const XsString* identifier) override;
	//void onRecordedDataAvailable(XsDotUsbDevice* device, const XsDataPacket* packet) override;
	//void onRecordedDataAvailable(XsDotDevice* device, const XsDataPacket* packet) override;
	//void onRecordedDataDone(XsDotUsbDevice* device) override;
	//void onRecordedDataDone(XsDotDevice* device) override;

	//mutable xsens::Mutex mutex;

};