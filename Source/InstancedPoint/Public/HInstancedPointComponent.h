// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "HInstancedPointComponent.generated.h"

/**
 * 
 */
UCLASS()
class INSTANCEDPOINT_API UHInstancedPointComponent : public UHierarchicalInstancedStaticMeshComponent
{
	GENERATED_UCLASS_BODY()
	
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FEOnCullingName, FString, InType, int32, Index, FVector, Location, bool, Culling);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelectPatternCulling);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFilterOffName, FString, InType);

public:
	UFUNCTION(BlueprintPure, Category = "InstancedPoint")
		FString GetType() { return Type; }

	UFUNCTION(BlueprintCallable, Category = "InstancedPoint")
		void SetType(FString InType) { Type = InType; }

	UFUNCTION(BlueprintCallable, Category = "InstancedPoint")
		float SetBoundsSize();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	UFUNCTION(BlueprintCallable, Category = "InstancedPoint")
		void UpdateTransform();

	UFUNCTION(BlueprintCallable, Category = "InstancedPoint")
		void SetLockZ(bool Lock);

	void UpdateName(float ScrDis, int32 InstIndex, FVector InstanceLocation);

	void UpdateType(int32 InstIndex, FVector InstanceLocation, AActor* iNFriController, FVector2D InViewSize, FTransform InInstanceTransform);

	UFUNCTION(BlueprintCallable, Category = "InstancedPoint")
		void SetCulling(float PatternDis, float NameDis);

	UFUNCTION(BlueprintCallable, Category = "InstancedPoint")
		void SelectInstance(int32 Index) { SelectedInstanceIndex = Index; }

	UFUNCTION(BlueprintCallable, Category = "InstancedPoint")
		void UnselectInstance() { SelectedInstanceIndex = -1; }

	UFUNCTION(BlueprintCallable, Category = "InstancedPoint")
		void FilterOffname();

	FTransform GetMinTransform(FVector Loc);

	FVector GetMinScale3D();
public:
		FString Type;

		float BoundSize = 0.0;

	UPROPERTY(EditAnywhere, Category = "InstancedPoint")
		int32 ScreenSize = 10;

	UPROPERTY(EditAnywhere, Category = "InstancedPoint")
		bool bLockZ = false;

	bool bSetBoundSize = false;

	UPROPERTY(EditAnywhere, Category = "InstancedPoint")
		float PatternCullingDistance = 100000;

	UPROPERTY(EditAnywhere, Category = "InstancedPoint")
		float NameCullingDistance = 1000;

	double dPatternCullingDistance;
	double dNameCullingDistance;

	UPROPERTY(EditAnywhere, Category = "InstancedPoint")
		bool bCulling = true;

	UPROPERTY(EditAnywhere, Category = "InstancedPoint")
		TMap<int32, FVector> ShowNameMap;

	UPROPERTY(EditAnywhere, Category = "InstancedPoint")
		int32 SelectedInstanceIndex = -1;

	UPROPERTY(BlueprintAssignable)
		FEOnCullingName EOnCullingName;

	UPROPERTY(BlueprintAssignable)
		FOnSelectPatternCulling OnSelectPatternCulling;
	UPROPERTY(BlueprintAssignable)
		FOnFilterOffName OnFilterOffName;
};
