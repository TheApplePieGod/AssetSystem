#include "pch/pch.h"
#include "asset.h"

static const char* AssetTypeStrings[] = { "Invalid", "Asset File", "Texture", "Font", "Save File" };
static asset_settings AssetSettings;

const char* GetStringFromAssetType(asset_type Type)
{
	return AssetTypeStrings[Type];
}

asset_settings GetAssetSettings()
{
	return AssetSettings;
}

void SetAssetSettings(asset_settings Settings)
{
	AssetSettings = Settings;
}

void /*ASSETSYSTEM_API*/ cAsset::UnloadAsset()
{
	delete[] Data;
	Data = nullptr;
	Loaded = false;
}

void /*ASSETSYSTEM_API*/ cTextureAsset::LoadAssetData(bool RefreshAsset) //todo: update method of loading / unloading (remove new)
{
	FILE* pak = nullptr;
	if (AssetSettings.LoadFromPack)
	{
		pak = fopen(AssetSettings.PackFileName, "rb");
		fseek(pak, atoi(Path), SEEK_SET);
	}
	else
		pak = fopen(Path, "rb");

	asset_header SearchingHeader;
	fread((char*)&SearchingHeader, sizeof(SearchingHeader), 1, pak);

	char* Pixels = new char[SearchingHeader.DataSize];

	if (RefreshAsset)
	{
		png_pack png;
		fread((char*)&png, sizeof(png), 1, pak);
		Width = png.Width;
		Height = png.Height;
		Channels = png.Channels;
		DataSize = SearchingHeader.DataSize;
	}
	else
		fseek(pak, sizeof(png_pack), SEEK_CUR);

	fread(Pixels, SearchingHeader.DataSize, 1, pak);

	Loaded = true;
	Data = Pixels;

	fclose(pak);
}

void /*ASSETSYSTEM_API*/ cTextureAsset::UnloadAsset()
{
	SAFE_RELEASE(TextureHandle);
	SAFE_RELEASE(AssociatedShaderHandle);
	delete[] Data;
	Data = nullptr;
	Loaded = false;
}

void /*ASSETSYSTEM_API*/ cFontAsset::LoadAssetData(bool RefreshAsset)
{
	FILE* pak = nullptr;
	if (AssetSettings.LoadFromPack)
	{
		pak = fopen(AssetSettings.PackFileName, "rb");
		fseek(pak, atoi(Path), SEEK_SET);
	}
	else
		pak = fopen(Path, "rb");

	asset_header SearchingHeader;
	fread((char*)&SearchingHeader, sizeof(SearchingHeader), 1, pak);

	char* Pixels = new char[SearchingHeader.DataSize];
	fseek(pak, sizeof(font_pack) + SearchingHeader.ExtraSize, SEEK_CUR);
	fread(Pixels, SearchingHeader.DataSize, 1, pak);

	Loaded = true;
	Data = Pixels;

	fclose(pak);
}

void /*ASSETSYSTEM_API*/ cFontAsset::UnloadAsset()
{
	SAFE_RELEASE(AtlasShaderHandle);
	delete[] Data;
	Data = nullptr;
	Loaded = false;
}

char_entry /*ASSETSYSTEM_API*/ cFontAsset::FindCharEntryByAscii(u32 AsciiVal)
{
	for (u32 i = 0; i < NumChars; i++)
	{
		if (Characters[i].AsciiValue == AsciiVal)
			return Characters[i];
	}
	return char_entry();
}