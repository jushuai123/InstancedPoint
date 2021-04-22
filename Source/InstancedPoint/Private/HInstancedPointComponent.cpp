// Fill out your copyright notice in the Description page of Project Settings.


#include "HInstancedPointComponent.h"
#include "Components/InstancedStaticMeshComponent.h"

UHInstancedPointComponent::UHInstancedPointComponent(const FObjectInitializer& PCIP)
	:Super(PCIP)
{
	PrimaryComponentTick.bCanEverTick = true;
}

float UHInstancedPointComponent::SetBoundsSize()
{
	FVector MeshExtent = GetStaticMesh()->GetBounds().BoxExtent;
	BoundSize = FMath::Max<float>(MeshExtent.X, MeshExtent.Y);
	bSetBoundSize = true;
	return BoundSize = FMath::Max<float>(BoundSize, MeshExtent.Z);
}

void UHInstancedPointComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateTransform();
	//UE_LOG(LogTemp, Warning, TEXT("IPC"));

}

void UHInstancedPointComponent::UpdateTransform()
{
	if (GetStaticMesh() && IsVisible())
	{
		if (bSetBoundSize && GetInstanceCount() > 0)
		{
			FVector ControllerLocation = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation();
			AActor* FriController = GetWorld()->GetFirstPlayerController();

			FVector2D ViewSize;
			GetWorld()->GetGameViewport()->GetViewportSize(ViewSize);

			for (int32 i = 0; i < GetInstanceCount(); i++)
			{
				FTransform InstanceTransform;
				GetInstanceTransform(i, InstanceTransform, true);
				FVector InstanceLocation = InstanceTransform.GetLocation();

				float ScreenDistance = (InstanceLocation - ControllerLocation).Size();

				if (i == SelectedInstanceIndex)
				{
					//FTransform NewTransform = FTransform(FRotator(0.01, 0.01, 0.01), InstanceLocation, FVector(0.01, 0.01, 0.01));
					FTransform NewTransform = GetMinTransform(InstanceLocation);
					if (NewTransform.ContainsNaN())
					{
						UE_LOG(LogTemp, Warning, TEXT("Instance transform ContainsNaN"));
					}
					UpdateInstanceTransform(i, NewTransform, true, true);

					UpdateName(ScreenDistance, i, InstanceLocation);
					
					if (ScreenDistance < PatternCullingDistance)
					{
						//SelectPatternCulling = false;
					}
					else
					{
						//SelectPatternCulling = true;
						OnSelectPatternCulling.Broadcast();
					}

					continue;
				}

				if (bCulling)
				{
					//判断是否显示图案
					if (ScreenDistance < PatternCullingDistance)
					{
						UpdateType(i, InstanceLocation, FriController, ViewSize, InstanceTransform);

						//判断是否显示Name
						UpdateName(ScreenDistance, i, InstanceLocation);
					}
					else
					{
						if (InstanceTransform.GetScale3D() == GetMinScale3D())
						{
							continue;
						}
						//把Scale设置为0来模拟隐藏图案
						FTransform NewTransform = GetMinTransform(InstanceLocation);
						if (NewTransform.ContainsNaN())
						{
							UE_LOG(LogTemp, Warning, TEXT("Instance transform ContainsNaN"));
						}
						UpdateInstanceTransform(i, NewTransform, true, true);
					}
				}
				else
				{
					UpdateType(i, InstanceLocation, FriController, ViewSize, InstanceTransform);
				}
			}
		}
	}
}

void UHInstancedPointComponent::SetCulling(float PatternDis, float NameDis)
{
	PatternCullingDistance = PatternDis;
	NameCullingDistance = NameDis;
}

void UHInstancedPointComponent::SetLockZ(bool Lock)
{
	bLockZ = Lock;
}

void UHInstancedPointComponent::UpdateName(float ScrDis, int32 InstIndex, FVector InstanceLocation)
{
	FString TypeName = Type;
	if (ScrDis < NameCullingDistance)
	{
		if (!ShowNameMap.Contains(InstIndex))
		{
			ShowNameMap.Add(InstIndex, InstanceLocation);
			EOnCullingName.Broadcast(TypeName, InstIndex, InstanceLocation, false);
		}
	}
	else
	{
		if (ShowNameMap.Contains(InstIndex))
		{
			ShowNameMap.Remove(InstIndex);
			EOnCullingName.Broadcast(TypeName, InstIndex, InstanceLocation, true);
		}
	}
}

void UHInstancedPointComponent::UpdateType(int32 InstIndex, FVector InstanceLocation, AActor* iNFriController, FVector2D InViewSize, FTransform InInstanceTransform)
{
	FVector ControllerUp = iNFriController->GetActorUpVector();
	FVector ControllerForward = iNFriController->GetActorForwardVector() * (bLockZ ? FVector(-1.0, -1.0, 0.0) : FVector(-1.0, -1.0, -1.0));
	//FVector ControllerForward = FriController->GetActorForwardVector() * FVector(-1.0, -1.0, -1.0);
	ControllerForward.Normalize(0.01);

	FVector2D ScreenLocationA;
	FVector2D ScreenLocationB;
	GetWorld()->GetFirstPlayerController()->ProjectWorldLocationToScreen(InstanceLocation, ScreenLocationA, true);
	GetWorld()->GetFirstPlayerController()->ProjectWorldLocationToScreen(InstanceLocation + (ControllerUp * BoundSize), ScreenLocationB, true);

	FVector NewScale;
	if (ScreenLocationA.X > 0 && ScreenLocationA.X < InViewSize.X && ScreenLocationA.Y > 0 && ScreenLocationA.Y < InViewSize.Y && ScreenLocationB.X > 0 && ScreenLocationB.X < InViewSize.X && ScreenLocationB.Y > 0 && ScreenLocationB.Y < InViewSize.Y)
	{
		float ScreenLen = (ScreenLocationA - ScreenLocationB).Size();
		if (ScreenLen == 0)
		{
			ScreenLen = 0.01;
		}
		NewScale = FVector(ScreenSize / (ScreenLocationA - ScreenLocationB).Size());
	}
	else
	{
		//NewScale = InInstanceTransform.GetScale3D();
		NewScale = GetMinScale3D();
	}
	//FVector NewScale = FVector(ScreenSize / (ScreenLocationA - ScreenLocationB).Size());
	FRotator NewRotator = FRotationMatrix::MakeFromYZ(ControllerForward, (bLockZ ? FVector(0.0, 0.0, 1.0) : ControllerUp)).Rotator();
	//FRotator NewRotator = FRotationMatrix::MakeFromYZ(ControllerForward, ControllerUp).Rotator();
	FTransform NewTransform = FTransform(NewRotator, InstanceLocation, NewScale);

	UpdateInstanceTransform(InstIndex, NewTransform, true, true);
}

void UHInstancedPointComponent::FilterOffname()
{
	OnFilterOffName.Broadcast(Type);
	ShowNameMap.Empty();
}

FTransform UHInstancedPointComponent::GetMinTransform(FVector Loc)
{
	return FTransform(FRotator(0.01, 0.01, 0.01), Loc, GetMinScale3D());
}

FVector UHInstancedPointComponent::GetMinScale3D()
{
	return FVector(0.01, 0.01, 0.01);
}