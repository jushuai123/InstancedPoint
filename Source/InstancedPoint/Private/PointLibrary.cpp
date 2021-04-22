// Fill out your copyright notice in the Description page of Project Settings.


#include "PointLibrary.h"
//#include "ImageUtils.h"
//#include <algorithm>
//#include "Misc/FileHelper.h"

//TArray<TArray<float>> UPointLibrary::Heightmap;
//
//void UPointLibrary::ReadHeightmap(FString Path, float MaxAltitude, float MinAltitude, int StepLongitude, int StepLatitude)
//{
//	Heightmap.Empty();
//
//	int PicNumHeight = 6, PicNumWidth = 6; // changed
//
//	UTexture2D* FirstTexture = FImageUtils::ImportFileAsTexture2D(Path + "0_0.png");
//	FTexture2DMipMap* FirstMipMap = &FirstTexture->PlatformData->Mips[0];
//	uint32 StepWidth = FirstMipMap->SizeX / StepLongitude, StepHeight = FirstMipMap->SizeY / StepLatitude;
//
//	for (int a = 0; a < PicNumHeight; a++)
//	{
//		for (size_t j = 0; j < StepLatitude; j++)
//		{
//			TArray<float> Line;
//			Heightmap.Add(Line);
//		}
//		for (int b = 0; b < PicNumWidth; b++)
//		{
//			UTexture2D* Texture = FImageUtils::ImportFileAsTexture2D(Path + FString::FromInt(a) + "_" + FString::FromInt(b) + ".png");
//			FTexture2DMipMap* MipMap = &Texture->PlatformData->Mips[0];
//			FByteBulkData* RawImageData = &MipMap->BulkData;
//			FColor* FormatedImageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));
//
//			for (size_t i = 0; i < StepLongitude; i++)
//			{
//				for (size_t j = 0; j < StepLatitude; j++)
//				{
//					FColor PixelColor = FormatedImageData[std::min(int(j * StepHeight), MipMap->SizeY - 1) * MipMap->SizeX + std::min(int(i * StepWidth), MipMap->SizeX - 1)];
//					Heightmap[a * StepLatitude + j].Add((PixelColor.B * 299.0 + PixelColor.G * 587.0 + PixelColor.R * 114.0) / 1000 / 255 * (MaxAltitude - MinAltitude));
//				}
//			}
//			RawImageData->Unlock();
//		}
//	}
//}
//
//void UPointLibrary::ReturnZ(float Longitude, float Latitude, float& Z)
//{
//	double HeightmapPixelHeight = double(30.9375 - 22.5) / (6 * 4096);
//	double HeightmapPixelWidth = double(115.3125 - 106.875) / (6 * 4096);
//	double HeightmapLeft = 106.875;
//	double HeightmapTop = 30.9375;
//	double TempZ = 0;
//	double CountX = abs(Longitude - HeightmapLeft) / HeightmapPixelWidth;
//	double CountY = abs(Latitude - HeightmapTop) / HeightmapPixelHeight;
//	double DeltaX = CountX - floor(CountX);
//	double DeltaY = CountY - floor(CountY);
//	if (Heightmap.IsValidIndex(ceil(CountY)) && Heightmap[ceil(CountY)].IsValidIndex(ceil(CountX)))
//	{
//		TempZ += Heightmap[floor(CountY)][floor(CountX)] * (1 - DeltaX) * (1 - DeltaY);
//		TempZ += Heightmap[floor(CountY)][ceil(CountX)] * DeltaX * (1 - DeltaY);
//		TempZ += Heightmap[ceil(CountY)][ceil(CountX)] * DeltaX * DeltaY;
//		TempZ += Heightmap[ceil(CountY)][floor(CountX)] * (1 - DeltaX) * DeltaY;
//	}
//	Z = TempZ;
//}

bool UPointLibrary::load_json(FString _file_path, TSharedPtr<FJsonObject>& _json_object)
{
	FString json_string;
	if (FFileHelper::LoadFileToString(json_string, *_file_path))
	{
		TSharedRef<TJsonReader<TCHAR>>json_reader = TJsonReaderFactory<TCHAR>::Create(json_string);
		TSharedPtr<FJsonObject>load_json_object = MakeShareable(new FJsonObject());
		FJsonSerializer::Deserialize(json_reader, load_json_object);
		if (load_json_object)
		{
			_json_object = load_json_object;
			return true;
		}
	}
	return false;
}

bool UPointLibrary::load_area_infomap(FString _file_path, FString _obj_key, TArray<FString>_info_keys,TMap<FString,FString>&InfoMap)
{
	InfoMap.Empty();

	// check file exist
	IFileManager& fm = IFileManager::Get();
	if (!fm.FileExists(*_file_path))return false;

	// load json
	TSharedPtr<FJsonObject>file_obj = MakeShareable(new FJsonObject());
	TSharedPtr<FJsonObject>area_obj = MakeShareable(new FJsonObject());
	load_json(_file_path, file_obj);

	// find info map

	area_obj = file_obj->GetObjectField(_obj_key);
	FString each_value;
	for (FString each_key:_info_keys)
	{
		if (area_obj->GetObjectField("InfoMap")->TryGetStringField(each_key, each_value))
		{
			InfoMap.Add(each_key,each_value);
		}
	}

	return true;
}

TMap<int32, FVector> UPointLibrary::GetPointLocMap(FString Type, FString DataPath)
{
	TMap<int32, FVector> PointLocaMap;

	FArchive* const FileAr = IFileManager::Get().CreateFileReader(*DataPath);
	if (FileAr)
	{
		TSharedPtr<FJsonObject> PointDataObject = MakeShareable(new FJsonObject);
		TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(FileAr);
		FJsonSerializer::Deserialize(Reader, PointDataObject);

		if (PointDataObject)
		{
			int32 PointNum = PointDataObject->GetObjectField(Type)->Values.Num();
			for (int32 PointIndex = 0; PointIndex < PointNum; PointIndex++)
			{
				TSharedPtr<FJsonObject> PointLocObj = MakeShareable(new FJsonObject);
				PointLocObj = PointDataObject->GetObjectField(Type)->GetObjectField(FString::FromInt(PointIndex))->GetObjectField(TEXT("Loc"));
				FVector PointLoc = FVector(PointLocObj->GetNumberField(TEXT("X")), PointLocObj->GetNumberField(TEXT("Y")), PointLocObj->GetNumberField(TEXT("Z")));
				PointLocaMap.Add(PointIndex, PointLoc);
			}
		}

		FileAr->Close();
	}

	return PointLocaMap;
}

TMap<FString, FString> UPointLibrary::GetPointAttribute(FString Type, int32 Index, FString DataPath)
{
	TMap<FString, FString> PointAttMap;

	FArchive* const FileAr = IFileManager::Get().CreateFileReader(*DataPath);
	if (FileAr)
	{
		TSharedPtr<FJsonObject> PointDataObject = MakeShareable(new FJsonObject);
		TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(FileAr);
		FJsonSerializer::Deserialize(Reader, PointDataObject);

		if (PointDataObject)
		{
			TSharedPtr<FJsonObject> PointAtt = MakeShareable(new FJsonObject);
			PointAtt = PointDataObject->GetObjectField(Type)->GetObjectField(FString::FromInt(Index))->GetObjectField(TEXT("Att"));
			TArray<FString> Keys;
			PointAtt->Values.GetKeys(Keys);
			for (FString Key : Keys)
			{
				PointAttMap.Add(Key, PointAtt->GetStringField(Key));
			}
		}

		FileAr->Close();
	}

	return PointAttMap;

}

TMap<FString, FTypePointAtt> UPointLibrary::GetPointAttMap(FString DataPath)
{
	TMap<FString, FTypePointAtt> AllPointAttMap;

	FArchive* const FileAr = IFileManager::Get().CreateFileReader(*DataPath);
	if (FileAr)
	{
		TSharedPtr<FJsonObject> PointDataObject = MakeShareable(new FJsonObject);
		TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(FileAr);
		FJsonSerializer::Deserialize(Reader, PointDataObject);

		if (PointDataObject)
		{
			TArray<FString> Typekeys;
			PointDataObject->Values.GetKeys(Typekeys);
			for (FString TpyeKey : Typekeys)
			{
				TMap<int32, FPointAtt> TypePointAttMap;

				int32 PointNum = PointDataObject->GetObjectField(TpyeKey)->Values.Num();
				for (int32 PointIndex = 0; PointIndex < PointNum; PointIndex++)
				{
					TSharedPtr<FJsonObject> PointAttObj = MakeShareable(new FJsonObject);
					PointAttObj = PointDataObject->GetObjectField(TpyeKey)->GetObjectField(FString::FromInt(PointIndex))->GetObjectField(TEXT("Att"));

					TMap<FString, FString> PointAttMap;
					TArray<FString> PointAttArray;
					PointAttObj->Values.GetKeys(PointAttArray);
					for (FString PointAttKey : PointAttArray)
					{
						PointAttMap.Add(PointAttKey, PointAttObj->GetStringField(PointAttKey));
					}
					FPointAtt PointAtt =  FPointAtt(PointAttMap);

					TypePointAttMap.Add(PointIndex, PointAtt);
				}
				FTypePointAtt TypePointAtt = FTypePointAtt(TypePointAttMap);
				AllPointAttMap.Add(TpyeKey, TypePointAtt);
			}
		}

		FileAr->Close();
	}

	return  AllPointAttMap;
}