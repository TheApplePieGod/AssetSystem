#include "pch/pch.h"
#include "asset.h"

#pragma warning( push )
#pragma warning( disable : 6387)
#pragma warning( disable : 6386)
#pragma warning( disable : 26444)

static const char* AssetTypeStrings[] = { "Invalid", "Asset File", "Texture", "Font", "Save File", "Mesh" };
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

void cAsset::UnloadAsset()
{
	delete[] Data;
	Data = nullptr;
	Loaded = false;
}

/*
* Unloads previous data
* RefreshAsset: Reload texture dimensions from file
*/
void cTextureAsset::LoadAssetData(bool RefreshAsset) //todo: update method of loading / unloading (remove new)
{
	if (Loaded)
		UnloadAsset();

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

void cTextureAsset::UnloadAsset()
{
	SAFE_RELEASE(TextureHandle);
	SAFE_RELEASE(ShaderHandle);
	delete[] Data;
	Data = nullptr;
	TextureHandle = nullptr;
	ShaderHandle = nullptr;
	Loaded = false;
}

/*
* Unloads previous data
* RefreshAsset: todo
*/
void cFontAsset::LoadAssetData(bool RefreshAsset)
{
	if (Loaded)
		UnloadAsset();

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

void cFontAsset::UnloadAsset()
{
	SAFE_RELEASE(AtlasShaderHandle);
	delete[] Data;
	Data = nullptr;
	AtlasShaderHandle = nullptr;
	Loaded = false;
}

void cMeshAsset::LoadAssetData(bool RefreshAsset)
{
	if (Loaded)
		UnloadAsset();

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

	mesh_vertex* Vertices = new mesh_vertex[SearchingHeader.DataSize / sizeof(mesh_vertex)];
	fseek(pak, sizeof(mesh_pack), SEEK_CUR);
	fread(Vertices, SearchingHeader.DataSize, 1, pak);

	Loaded = true;
	Data = Vertices;

	fclose(pak);
}

void cMeshAsset::UnloadAsset()
{
	delete[] Data;
	Data = nullptr;
	Loaded = false;
}

#pragma warning( pop )