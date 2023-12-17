// Fill out your copyright notice in the Description page of Project Settings.


#include "XsDotObject.h"

// Sets default values
AXsDotObject::AXsDotObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


}

// Called when the game starts or when spawned
void AXsDotObject::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AXsDotObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsXsDotConnected)
	{
		this->SetActorRotation(XsDotQuat);
	}
}

