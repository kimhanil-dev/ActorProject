// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "XsDotObject.generated.h"

UCLASS()
class MOVELLAXSENSDOT_API AXsDotObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AXsDotObject();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Xsens Dot")
	bool bIsXsDotConnected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Xsens Dot")
	FQuat XsDotQuat;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
