// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "InstancedPointComponent.generated.h"

/**
 * 
 */
UCLASS()
class INSTANCEDPOINT_API UInstancedPointComponent : public UInstancedStaticMeshComponent
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "InstancedPoint")
	float SetBoundsSize();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction);

	UFUNCTION(BlueprintCallable, Category = "InstancedPoint")
	void UpdateTransform();

//	//切换远中近的事件
//	DECLARE_EVENT(UInstancedPointComponent, FChangeDistanceType)
//	FChangeDistanceType& DisChanged() { return ChangeDistanceType;}
//private:
//	FChangeDistanceType ChangeDistanceType;

public:
	UPROPERTY(VisibleAnywhere, Category = "InstancedPoint")
		float BoundSize = 0.0;

	UPROPERTY(EditAnywhere, Category = "InstancedPoint")
		int32 ScreenSize = 10;

	bool bSetBoundSize = false;


};
