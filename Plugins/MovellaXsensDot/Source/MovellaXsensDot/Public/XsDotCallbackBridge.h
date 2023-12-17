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
		, BluetoothAddress(*XDSTR_TO_UESTR(portInfo.bluetoothAddress())){}
	FXsPortInfo(const FXsPortInfo& xsPortInfo)
		:PortName(xsPortInfo.PortName)
		, DeviceID(xsPortInfo.DeviceID)
		, BluetoothAddress(xsPortInfo.BluetoothAddress){}


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Xsens")
	FString PortName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Xsens")
	FString DeviceID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Xsens")
	FString BluetoothAddress;
};

UENUM(BlueprintType)
enum class EOutputRate : uint8
{
	OR_NONE = 0 UMETA(DisplayName = "None"),

	OR_1HZ = 1 UMETA(DisplayName = "OutputRate 1Hz"),
	OR_4HZ = 4 UMETA(DisplayName = "OutputRate 4Hz"),
	OR_10HZ = 10 UMETA(DisplayName = "OutputRate 10Hz"),
	OR_12HZ = 12 UMETA(DisplayName = "OutputRate 12Hz"),
	OR_15HZ = 15 UMETA(DisplayName = "OutputRate 15Hz"),
	OR_20HZ = 20 UMETA(DisplayName = "OutputRate 20Hz"),
	OR_30HZ = 30 UMETA(DisplayName = "OutputRate 30Hz"),
	OR_60HZ = 60 UMETA(DisplayName = "OutputRate 60Hz"),
	OR_120HZ = 120 UMETA(DisplayName = "OutputRate 120Hz")
};

class IXsDotCallBackListener
{
public:
	virtual void OnAdvertisementFound(const FXsPortInfo& portInfo) = 0;
	virtual void OnError(const XsResultValue result, const FString error) = 0;
	virtual void onButtonClicked(XsDotDevice* device, uint32_t timestamp) = 0;

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

	bool ConnectDot(const FString& deviceAddress);
	bool ConnectDot(XsPortInfo portInfo);
	//todo : connection�� �������� ��쿡 ��� �˸� ������?
	bool ConnectDots(const XsPortInfoArray& portInfos);
	//todo : connection�� �������� ��쿡 ��� �˸� ������?
	bool ConnectAllDots();

	XsPortInfoArray GetDetectedDevices();
	TArray<const XsDotDevice*> GetConnectedDevices();


	// !!! ���� !!!
	// Packet���� ���� �о���� �Լ����� Packet�� �а� ���� ���ο��� Packet�� �����մϴ�.
	bool GetLiveData(const FString& deviceBluetoothAddress, FRotator& outRotation, FVector& outAcc, FQuat& quat);
	bool GetQuaternionData(const FString deviceBluetoothAddress, FQuat& outQuat);

	void SetLiveDataOutputRate(const EOutputRate& rate = EOutputRate::OR_30HZ);

	void ResetOrientation();

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
	void onButtonClicked(XsDotDevice* device, uint32_t timestamp) override;
	//void onProgressUpdated(XsDotUsbDevice* device, int current, int total, const XsString* identifier) override;
	//void onRecordedDataAvailable(XsDotUsbDevice* device, const XsDataPacket* packet) override;
	//void onRecordedDataAvailable(XsDotDevice* device, const XsDataPacket* packet) override;
	//void onRecordedDataDone(XsDotUsbDevice* device) override;
	//void onRecordedDataDone(XsDotDevice* device) override;

	//mutable xsens::Mutex mutex;

};