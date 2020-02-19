#include "pch.h"
#include "asset.h"
#include "assets/defaultAssetTypes.h"

#pragma warning( push )
#pragma warning( disable : 6387)
#pragma warning( disable : 6386)
#pragma warning( disable : 26444)

using namespace defaultAssetTypes;

asset_settings AssetSettings;
std::vector<asset_type> AssetTypes;

asset_settings& GetAssetSettings()
{
	return AssetSettings;
}

void SetAssetSettings(asset_settings Settings)
{
	AssetSettings = Settings;
}

void cAsset::LoadAssetData()
{
	if (Loaded)
		UnloadAsset();

	FILE* File = nullptr;
	if (AssetSettings.LoadFromPack)
	{
		File = fopen(AssetSettings.PackFileName, "rb");
		fseek(File, atoi(Path), SEEK_SET);
	}
	else
		File = fopen(Path, "rb");

	asset_header Header;
	fread((char*)&Header, sizeof(Header), 1, File);

	char* LoadedData = new char[Header.RawDataSize];

	fseek(File, Header.ExtraDataSize, SEEK_CUR); // skip extra data

	fread(LoadedData, Header.RawDataSize, 1, File);

	Loaded = true;
	Data = LoadedData; // data can be casted to any type later on

	fclose(File);
}

void cAsset::UnloadAsset()
{
	delete[] Data;
	Data = nullptr;
	Loaded = false;
}

void cTextureAsset::UnloadAsset()
{
	cAsset::UnloadAsset();
	SAFE_RELEASE(TextureHandle);
	SAFE_RELEASE(ShaderHandle);
	TextureHandle = nullptr;
	ShaderHandle = nullptr;
}


void cFontAsset::UnloadAsset()
{
	cAsset::UnloadAsset();
	SAFE_RELEASE(AtlasShaderHandle);
	AtlasShaderHandle = nullptr;
}

#pragma warning( pop )