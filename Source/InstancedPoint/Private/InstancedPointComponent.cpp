// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedPointComponent.h"


UInstancedPointComponent::UInstancedPointComponent(const FObjectInitializer& PCIP)
	:Super(PCIP)
{
	PrimaryComponentTick.bCanEverTick = true;
}

float UInstancedPointComponent::SetBoundsSize()
{
	FVector MeshExtent = GetStaticMesh()->GetBounds().BoxExtent;
	BoundSize = FMath::Max<float>(MeshExtent.X, MeshExtent.Y);
	bSetBoundSize = true;
	return BoundSize = FMath::Max<float>(BoundSize, MeshExtent.Z);
}

void UInstancedPointComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateTransform();
	//UE_LOG(LogTemp, Warning, TEXT("IPC"));

}

void UInstancedPointComponent::UpdateTransform()
{
	if(GetStaticMesh())
	if (bSetBoundSize && GetStaticMesh() && GetInstanceCount() > 0)
	{
		for (int32 i = 0; i < GetInstanceCount(); i++)
		{
			FTransform InstanceTransform;
			GetInstanceTransform(i, InstanceTransform, true);
			FVector InstanceLocation = InstanceTransform.GetLocation();

			AActor* FriController= GetWorld()->GetFirstPlayerController();
			FVector ControllerUp = FriController->GetActorUpVector();
			FVector ControllerForward = FriController->GetActorForwardVector() * FVector(-1.0, -1.0, 0.0);
			ControllerForward.Normalize(0.0001);

			FVector2D ScreenLocationA;
			FVector2D ScreenLocationB;
			GetWorld()->GetFirstPlayerController()->ProjectWorldLocationToScreen(InstanceLocation, ScreenLocationA, true);
			GetWorld()->GetFirstPlayerController()->ProjectWorldLocationToScreen(InstanceLocation + (ControllerUp * BoundSize), ScreenLocationB, true);
			
			FVector NewScale =FVector(ScreenSize / (ScreenLocationA - ScreenLocationB).Size());
			FRotator NewRotator = FRotationMatrix::MakeFromYZ(ControllerForward, FVector(0.0, 0.0, 1.0)).Rotator();
			FTransform NewTransform = FTransform(NewRotator, InstanceLocation, NewScale);

			UpdateInstanceTransform(i, NewTransform, true, true);
		}
	}
}