#include "pch.h"
#include "../assetLoader.h"
#include "../assetLoader_internal.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION

#include "../lib/stb_image.h"
#include "../lib/stb_image_write.h"
#include "../lib/stb_truetype.h"

u32 assetLoader::Image_GetDataForWriting(char*& Out_Data, u32& Out_RawDataSize, char* FilePath)
{
	png_pack PNGPack;
	unsigned char* PixelData = stbi_load(FilePath, &PNGPack.Width, &PNGPack.Height, &PNGPack.Channels, 4); // load the png
	
	// stb couldn't load image (probably corrupt)
	if (PixelData == nullptr)
		return 0;
	
	PNGPack.Channels = 4;  // makes every loaded texture have 4 channels
	u32 DataLength = PNGPack.Width * PNGPack.Height * PNGPack.Channels;
	u32 TotalSize = sizeof(PNGPack) + DataLength;

	Out_Data = new char[TotalSize];
	memcpy(Out_Data, (char*)&PNGPack, sizeof(PNGPack));
	memcpy(Out_Data + sizeof(PNGPack), (char*)PixelData, DataLength);

	Out_RawDataSize = DataLength;
	delete[] PixelData;
	return TotalSize;
}

void assetLoader::Image_InitializeData(cAsset* AssetDefaults, char* Data, u32 DataSize)
{
	png_pack PNGPack = *((png_pack*)Data);
	cTextureAsset* TexAsset = new cTextureAsset();
	TexAsset->CopyFields(AssetDefaults);

	TexAsset->Width = PNGPack.Width;
	TexAsset->Height = PNGPack.Height;
	TexAsset->Channels = PNGPack.Channels;

	// todo: read directly from Data instead of calling load ?
	TexAsset->LoadAssetData();

	//(*Callback)(TexAsset);
}