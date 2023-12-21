// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/LatentActionManager.h"
#include "LatentActions.h"

#include "XsDotCallbackBridge.h"

#include "XsDot.generated.h"


USTRUCT(BlueprintType)
struct MOVELLAXSENSDOT_API FXsBindingInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Xsens Dot")
	USceneComponent* SceneComponent;

	UPROPERTY(BlueprintReadWrite, Category = "Xsens Dot")
	FString TargetXsDeviceBluetoothAddress;

	UPROPERTY(BlueprintReadWrite, Category = "Xsens Dot")
	bool bIsEnabled = false;

	UPROPERTY(BlueprintReadWrite, Category = "Xsens Dot")
	bool bIsLocal;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOVELLAXSENSDOT_API UXsBindingManager : public UActorComponent
{
public:
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	bool Bind(FString tag, const FXsBindingInfo& bindingInfo, const bool isAnonymous);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	bool SetEnable(FString tag, bool isEnable);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	bool SetLocal(FString tag, bool isLocal);

	void Update(const FString& deviceBluetoothAddress, const FQuat& rotation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Xsens Dot")
	void OnRotationUpdated();

private:
	TMap<FString, FXsBindingInfo> BindingInfos;
	TMap<FString, TArray<FXsBindingInfo>> BindingInfosDeviceAddressBased;

	/** if 'BindingInfo' not found return false */
	FXsBindingInfo* GetBindingInfo(const FString& tag);
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MOVELLAXSENSDOT_API UXsDot : public UActorComponent, public IXsDotCallBackListener
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UXsDot();

	// ConnectDevices 함수의 결과를 받기 위한 델리게이트
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnDeviceConnectionResult, FString, deviceAddress, bool, isSuccess);
	FOnDeviceConnectionResult OnDeviceConnectionResult;

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDeviceDetected, FString, deviceAddress);
	FOnDeviceDetected OnDeviceDetected;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnButtonClicked, FString, deviceAddress, int, timestamp);
	UPROPERTY(BlueprintAssignable, Category = "Xsens Dot")
	FOnButtonClicked OnButtonClicked;

	//DECLARE_DELEGATE_

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void GetDetectedDeviceName(TArray<FXsPortInfo>& devices) const;

	/*UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void GetLiveData(const FString& deviceBluetoothAddress, FRotator& rotation, FVector& acceleration, FQuat& quat, bool& valid);*/

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	bool GetQuaternionData(const FString& devicebluetoothAddress, FQuat& quat);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void SetLiveDataOutputRate(const EOutputRate& rate);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void ConnectDevices(const FOnDeviceConnectionResult& onDeviceConnectionResult);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void ConnectDevice(const FString deviceAddress, const FOnDeviceConnectionResult& onDeviceConnectionResult, bool& deviceNotFound);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void StartMeasurement(const FString deviceAddress, bool& isSuccess);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void StartScanning(const FOnDeviceDetected& onDeviceDetected);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void StopScanning();

	/** reset yaw orientation to current device's yaw */
	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void ResetOrientation();

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void AddBindingManager(UXsBindingManager* bindingManager);

	/** return XsDot coordinate system vector to unreal engine coordinate system vector */
	UFUNCTION(BlueprintPure, Category = "Xsens Dot | Utill")
	static FVector XsDotVectorToUEVector(const FVector xsVector);
	UFUNCTION(BlueprintPure, Category = "Xsens Dot | Utill")
	static FRotator XsDotRotatorToUERotator(const FRotator xsRotator);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TArray< UXsBindingManager*> XsBindingManagers;

private:
	XsDotCallbackBridge* XsDotHelper = nullptr;
	TMap<FString, FQuat> XsDotQuats;

	// for check runnging thread
	unsigned int ThreadCounter;
	bool bIsGameEnd = false;

	// Inherited via IXsDotCallBackListener
	void OnAdvertisementFound(const FXsPortInfo& portInfo) override;
	void OnError(const XsResultValue result, const FString error) override;
	void onButtonClicked(XsDotDevice* device, uint32_t timestamp) override;
};

// 병렬 처리를 위한 비동기 작업 클래스
class FAsyncConnectDevices : public FNonAbandonableTask
{
public:
	FAsyncConnectDevices(XsDotCallbackBridge& xsDotHelper, const FString& deviceAddress, bool& result) : Result(result), DeviceAddress(deviceAddress), XsDotHelper(xsDotHelper) {}
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncConnectDevices, STATGROUP_ThreadPoolAsyncTasks); }

	bool& Result;
	const FString& DeviceAddress;
	XsDotCallbackBridge& XsDotHelper;

	void DoWork();
};