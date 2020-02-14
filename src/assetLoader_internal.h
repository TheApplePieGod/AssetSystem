#pragma once

// define functions not needed to be exposed in the api
namespace assetLoader
{
	u32 Image_GetDataForWriting(char*& Out_Data, u32& Out_RawDataSize, char* FilePath);
	void Image_InitializeData(cAsset* AssetDefaults, char* Data, u32 DataSize);
}