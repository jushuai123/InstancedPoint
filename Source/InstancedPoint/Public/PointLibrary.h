// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Json.h"
#include "Dom/JsonObject.h"
#include "PointLibrary.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FPointAtt
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PointAttribute")
		TMap<FString, FString> PointAtt;

	FPointAtt()
	{
		PointAtt = {};
	}

	FPointAtt(TMap<FString, FString> InPointAtt)
	{
		PointAtt = InPointAtt;
	}

	
};

USTRUCT(BlueprintType)
struct FTypePointAtt
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PointAttribute")
		TMap<int32, FPointAtt> TypePointAtt;

	FTypePointAtt()
	{
		TypePointAtt = {};
	}

	FTypePointAtt(TMap<int32, FPointAtt> InTypePointAtt)
	{
		TypePointAtt = InTypePointAtt;
	}

	
};

UCLASS()
class INSTANCEDPOINT_API UPointLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//static TArray<TArray<float>> Heightmap;

	//UFUNCTION(BlueprintCallable, meta = (DisplayName = "ReadHeightmap", Keywords = "Read Heightmap And Store"), Category = "StreetMesh")
	//	static void ReadHeightmap(FString Path, float MaxAltitude = 2563, float MinAltitude = -149, int StepLongitude = 4096, int StepLatitude = 4096); // changed

	//UFUNCTION(BlueprintCallable, meta = (DisplayName = "ReturnZ", Keywords = "Return Z From Heightmap"), Category = "StreetMesh")
	//	static void ReturnZ(float Longitude, float Latitude, float& Z);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LoadAreaInfoMap", Keywords = "Load Area Info Map"), Category = "PointLib")
		static bool load_area_infomap(FString _file_path,FString _obj_key,TArray<FString>_info_keys,TMap<FString,FString>&InfoMap);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetPointAttribute", Keywords = "Get Point Attribute"), Category = "PointLib")
		static TMap<FString, FString> GetPointAttribute(FString Type, int32 Index, FString DataPath);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetPointLoc", Keywords = "get Point Location"), Category = "PointLib")
		static TMap<int32, FVector> GetPointLocMap(FString Type, FString DataPath);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetPointAttMap", Keywords = "Get Point Attribute Map"), Category = "PointLib")
		static TMap<FString, FTypePointAtt> GetPointAttMap(FString DataPath);

private:
		static bool load_json(FString _file_path,TSharedPtr<FJsonObject>&_json_object);
};
