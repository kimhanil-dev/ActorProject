// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "XsDotCallbackBridge.h"

#include "XsDot.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MOVELLAXSENSDOT_API UXsDot : public UActorComponent, public IXsDotCallBackListener
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UXsDot();

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void StartScanning();

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void StopScanning();

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void ConnectDevices();

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void GetDetectedDeviceName(TArray<FXsPortInfo>& devices);

	UFUNCTION(BlueprintCallable, Category = "Xsens Dot")
	void GetLiveData(const FString deviceBluetoothAddress, FVector& rotation, FVector& acceleration, bool& valid);
	
	/*DECLARE_DELEGATE_OneParam(FOnDeviceDetected, const FXsPortInfo&)
	UPROPERTY(BlueprintAssignable, Category = "Xsens Dot")
	FOnDeviceDetected OnDeviceDetected;*/

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	TUniquePtr<XsDotCallbackBridge> XsDotHelper;

	// Inherited via IXsDotCallBackListener
	void OnAdvertisementFound(const FXsPortInfo& portInfo) override;
	void OnError(const XsResultValue result, const FString error) override;
};
