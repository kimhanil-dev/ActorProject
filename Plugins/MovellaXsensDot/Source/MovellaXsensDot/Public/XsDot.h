// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/LatentActionManager.h"
#include "LatentActions.h"

#include "XsDotCallbackBridge.h"

#include "XsDot.generated.h"

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
	void GetDetectedDeviceName(TArray<FXsPortInfo>& devices);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void GetLiveData(const FString& deviceBluetoothAddress, FRotator& rotation, FVector& acceleration, FQuat& quat, bool& valid);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void SetLiveDataOutputRate(const EOutputRate& rate);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void ConnectDevices(const FOnDeviceConnectionResult& onDeviceConnectionResult);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void ConnectDevice(const FString deviceAddress, const FOnDeviceConnectionResult& onDeviceConnectionResult, bool& deviceNotFound);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void StartScanning(const FOnDeviceDetected& onDeviceDetected);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void StopScanning();

	/** reset yaw orientation to current device's yaw */
	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void ResetOrientation();

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
private:
	XsDotCallbackBridge* XsDotHelper = nullptr;

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
	FAsyncConnectDevices(XsDotCallbackBridge& xsDotHelper, XsPortInfo& xsPortInfo, bool& result) : Result(result), XsPortInfo(xsPortInfo), XsDotHelper(xsDotHelper) {}
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncConnectDevices, STATGROUP_ThreadPoolAsyncTasks); }

	bool& Result;
	XsPortInfo& XsPortInfo;
	XsDotCallbackBridge& XsDotHelper;

	void DoWork();
};