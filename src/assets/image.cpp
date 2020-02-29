#include "pch.h"
#include "../assetLoader.h"
#include "defaultAssetTypes.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION

#include "../lib/stb_image.h"
#include "../lib/stb_image_write.h"
#include "../lib/stb_truetype.h"

bool defaultAssetTypes::Image_GetDataForWriting(char*& Out_ExtraData, char*& Out_RawData, u32& Out_ExtraDataSize, u32& Out_RawDataSize, char* FilePath)
{
	image_data ImageData;
	unsigned char* PixelData = stbi_load(FilePath, &ImageData.Width, &ImageData.Height, &ImageData.Channels, 4); // load the png
	
	// stb couldn't load image (probably corrupt)
	if (PixelData == nullptr)
		return false;
	
	ImageData.Channels = 4;  // makes every loaded texture have 4 channels
	u32 DataLength = ImageData.Width * ImageData.Height * ImageData.Channels;
	u32 ExtraSize = sizeof(ImageData);

	// Set fields
	Out_ExtraData = new char[ExtraSize];
	memcpy(Out_ExtraData, &ImageData, ExtraSize);
	Out_RawData = (char*)PixelData;
	Out_ExtraDataSize = ExtraSize;
	Out_RawDataSize = DataLength;

	return true;
}

cAsset* defaultAssetTypes::Image_InitializeData(cAsset* AssetDefaults, char* ExtraData, u32 ExtraDataSize)
{
	image_data ImageData = *((image_data*)ExtraData);
	cTextureAsset* TexAsset = new cTextureAsset();
	TexAsset->CopyFields(AssetDefaults);

	TexAsset->ImageData = ImageData;

	TexAsset->LoadAssetData();

	return TexAsset;
}