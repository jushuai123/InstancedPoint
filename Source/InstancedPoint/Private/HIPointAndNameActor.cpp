// Fill out your copyright notice in the Description page of Project Settings.


#include "HIPointAndNameActor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/WidgetComponent.h"

// Sets default values
AHIPointAndNameActor::AHIPointAndNameActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HIPoint = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("HIPoint"));
	HIPoint->CastShadow = 0;

	//PointName = CreateDefaultSubobject<UWidgetComponent>(TEXT("PointName"));
}

// Called when the game starts or when spawned
void AHIPointAndNameActor::BeginPlay()
{
	Super::BeginPlay();

	SetBoundsSize();
	
}

// Called every frame
void AHIPointAndNameActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateTransform();
}

float AHIPointAndNameActor::SetBoundsSize()
{
	FVector MeshExtent = HIPoint->GetStaticMesh()->GetBounds().BoxExtent;
	BoundSize = FMath::Max<float>(MeshExtent.X, MeshExtent.Y);
	bSetBoundSize = true;
	return BoundSize = FMath::Max<float>(BoundSize, MeshExtent.Z);
}

void AHIPointAndNameActor::SetCulling(float PatternDis, float NameDis)
{
	PatternCullingDistance = PatternDis;
	NameCullingDistance = NameDis;
}

void AHIPointAndNameActor::SetLockZ(bool Lock)
{
	bLockZ = Lock;
}

void AHIPointAndNameActor::UpdateTransform()
{
	if (HIPoint->GetStaticMesh() && HIPoint->IsVisible())
	{
		if (bSetBoundSize && HIPoint->GetInstanceCount() > 0)
		{
			FVector ControllerLocation = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation();
			AActor* FriController = GetWorld()->GetFirstPlayerController();

			FVector2D ViewSize;
			GetWorld()->GetGameViewport()->GetViewportSize(ViewSize);

			//UWidgetComponent* NewActorComp;

			for (int32 i = 0; i < HIPoint->GetInstanceCount(); i++)
			{
				FTransform InstanceTransform;
				HIPoint->GetInstanceTransform(i, InstanceTransform, true);
				FVector InstanceLocation = InstanceTransform.GetLocation();

				float ScreenDistance = (InstanceLocation - ControllerLocation).Size();

				if (bCulling)
				{
					//判断是否显示图案
					if (ScreenDistance < PatternCullingDistance)
					{
						FVector ControllerUp = FriController->GetActorUpVector();
						FVector ControllerForward = FriController->GetActorForwardVector() * (bLockZ ? FVector(-1.0, -1.0, 0.0) : FVector(-1.0, -1.0, -1.0));
						ControllerForward.Normalize(0.0001);

						FVector2D ScreenLocationA;
						FVector2D ScreenLocationB;
						GetWorld()->GetFirstPlayerController()->ProjectWorldLocationToScreen(InstanceLocation, ScreenLocationA, true);
						GetWorld()->GetFirstPlayerController()->ProjectWorldLocationToScreen(InstanceLocation + (ControllerUp * BoundSize), ScreenLocationB, true);

						FVector NewScale;
						if (ScreenLocationA.X > 0 && ScreenLocationA.X < ViewSize.X && ScreenLocationA.Y > 0 && ScreenLocationA.Y < ViewSize.Y && ScreenLocationB.X > 0 && ScreenLocationB.X < ViewSize.X && ScreenLocationB.Y > 0 && ScreenLocationB.Y < ViewSize.Y)
						{
							NewScale = FVector(ScreenSize / (ScreenLocationA - ScreenLocationB).Size());
						}
						else
						{
							NewScale = InstanceTransform.GetScale3D();
						}
						//FVector NewScale = FVector(ScreenSize / (ScreenLocationA - ScreenLocationB).Size());

						FRotator NewRotator = FRotationMatrix::MakeFromYZ(ControllerForward, (bLockZ ? FVector(0.0, 0.0, 1.0) : ControllerUp)).Rotator();
						FTransform NewTransform = FTransform(NewRotator, InstanceLocation, NewScale);

						if (NewTransform.ContainsNaN())
						{
							UE_LOG(LogTemp, Warning, TEXT("Instance transform ContainsNaN"));
						}
						HIPoint->UpdateInstanceTransform(i, NewTransform, true, true);

						//判断是否显示Name
						if (ScreenDistance < NameCullingDistance)
						{
							if (!ShowNameMap.Contains(i))
							{
								if (NameWidgetClass)
								{
									FString ComponentName = TEXT("UI_") + FString::FromInt(i);
									UWidgetComponent* NewActorComp = NewObject<UWidgetComponent>(this, *ComponentName);
									NewActorComp->SetWidgetSpace(EWidgetSpace::Screen);
									NewActorComp->SetWidgetClass(NameWidgetClass);
									NewActorComp->SetDrawSize(NameDrawSize);
									NewActorComp->SetPivot(NamePivot);
									NewActorComp->SetWorldLocation(InstanceLocation);
									//NameMap.Find(i);
									NewActorComp->RegisterComponent();
									ShowNameMap.Emplace(i, NewActorComp);
								}
							}
						}
						else
						{
							if (ShowNameMap.Contains(i))
							{
								ShowNameMap[i]->DestroyComponent();
								ShowNameMap.Remove(i);
							}
						}
					}
					else
					{
						if (InstanceTransform.GetScale3D() == FVector(0.0, 0.0, 0.0))
						{
							continue;
						}
						//把Scale设置为0来模拟隐藏图案
						FTransform NewTransform = FTransform(FRotator(1.0, 1.0, 1.0), InstanceLocation, FVector(0.001, 0.001, 0.001));
						if (NewTransform.ContainsNaN())
						{
							UE_LOG(LogTemp, Warning, TEXT("Instance transform ContainsNaN"));
						}
						HIPoint->UpdateInstanceTransform(i, NewTransform, true, true);
					}
				}
				else
				{
					FVector ControllerUp = FriController->GetActorUpVector();
					FVector ControllerForward = FriController->GetActorForwardVector() * (bLockZ ? FVector(-1.0, -1.0, 0.0) : FVector(-1.0, -1.0, -1.0));
					ControllerForward.Normalize(0.0001);

					FVector2D ScreenLocationA;
					FVector2D ScreenLocationB;
					GetWorld()->GetFirstPlayerController()->ProjectWorldLocationToScreen(InstanceLocation, ScreenLocationA, true);
					GetWorld()->GetFirstPlayerController()->ProjectWorldLocationToScreen(InstanceLocation + (ControllerUp * BoundSize), ScreenLocationB, true);

					FVector NewScale = FVector(ScreenSize / (ScreenLocationA - ScreenLocationB).Size());
					FRotator NewRotator = FRotationMatrix::MakeFromYZ(ControllerForward, (bLockZ ? FVector(0.0, 0.0, 1.0) : ControllerUp)).Rotator();
					FTransform NewTransform = FTransform(NewRotator, InstanceLocation, NewScale);

					HIPoint->UpdateInstanceTransform(i, NewTransform, true, true);
				}
			}
		}
	}
}