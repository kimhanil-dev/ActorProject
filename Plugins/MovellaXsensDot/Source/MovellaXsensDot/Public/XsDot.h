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

	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnDeviceConnectionTryFinished, FString, deviceAddress, bool, isSuccess);
	FOnDeviceConnectionTryFinished OnDeviceConnectionTryFinished;

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDeviceDetected, FString, deviceAddress);
	FOnDeviceDetected OnDeviceDetected;

	//DECLARE_DELEGATE_

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void GetDetectedDeviceName(TArray<FXsPortInfo>& devices);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void GetLiveData(const FString deviceBluetoothAddress, FVector& rotation, FVector& acceleration, bool& valid);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void SetLiveDataOutputRate(const EOutputRate& rate);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void ConnectDevices();

	
	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void StartScanning();


	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void StopScanning();



protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	XsDotCallbackBridge* XsDotHelper = nullptr;

	// Inherited via IXsDotCallBackListener
	void OnAdvertisementFound(const FXsPortInfo& portInfo) override;
	void OnError(const XsResultValue result, const FString error) override;
};

// 병렬 처리를 위한 비동기 작업 클래스
class FAsyncConnectDevices : public FNonAbandonableTask
{
public:
	FAsyncConnectDevices(XsDotCallbackBridge& xsDotHelper, XsPortInfo& xsPortInfo) : XsDotHelper(xsDotHelper), XsPortInfo(xsPortInfo) {}
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncConnectDevices, STATGROUP_ThreadPoolAsyncTasks); }

	XsPortInfo& XsPortInfo;
	XsDotCallbackBridge& XsDotHelper;

	void DoWork();
};