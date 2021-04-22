// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HIPointAndNameActor.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UWidgetComponent;

UCLASS()
class INSTANCEDPOINT_API AHIPointAndNameActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHIPointAndNameActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


public:

	UPROPERTY(Category = "InstancedPoint", VisibleAnywhere, BlueprintReadOnly)
		UHierarchicalInstancedStaticMeshComponent* HIPoint;

public:
	UFUNCTION(BlueprintCallable, Category = "InstancedPoint")
		float SetBoundsSize();

	UFUNCTION(BlueprintCallable, Category = "InstancedPoint")
		void UpdateTransform();

	UFUNCTION(BlueprintCallable, Category = "InstancedPoint")
		void SetLockZ(bool Lock);

	UFUNCTION(BlueprintCallable, Category = "InstancedPoint")
		void SetCulling(float PatternDis, float NameDis);

public:
	UPROPERTY(VisibleAnywhere, Category = "InstancedPoint")
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

	UPROPERTY(EditAnywhere, Category = "InstancedPoint")
		bool bCulling = true;

	//UPROPERTY(EditAnywhere, Category = "InstancedPoint")
		TMap<int32, UWidgetComponent*> ShowNameMap;

	UPROPERTY(EditAnywhere, Category = "InstancedPoint")
		TMap<int32, FString> NameMap;

	UPROPERTY(EditAnywhere, Category = "InstancedPoint")
		TSubclassOf<UUserWidget> NameWidgetClass;

	UPROPERTY(EditAnywhere, Category = "InstancedPoint")
		FVector2D NameDrawSize = FVector2D(256, 32);

	UPROPERTY(EditAnywhere, Category = "InstancedPoint")
		FVector2D NamePivot = FVector2D(0.5, 2.5);
};
