#pragma once
#include "shared.h"

const char* GetStringFromAssetType(asset_type Type);

asset_settings GetAssetSettings();
void SetAssetSettings(asset_settings Settings);

#ifdef ASSET_FORWARD_DECLARATIONS
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
#endif

struct /*ASSETSYSTEM_API*/ cAsset
{
	s32 AssetID; // deprecated
	b32 Active;
	bool Loaded;
	asset_type Type;
	void* Data = nullptr;
	u32 DataSize;
	char Filename[MAX_PATH];
	char Path[MAX_PATH]; // if reading from pac this will be the position inside, otherwise it will be the path of the file

	virtual void LoadAssetData(bool RefreshAsset = false) = 0; // refresh asset is basically like reinitializing it so use if new data is possibly different
	virtual void UnloadAsset();
};

struct /*ASSETSYSTEM_API*/ cTextureAsset : public cAsset
{
	s32 Width;
	s32 Height;
	s32 Channels;
	ID3D11Texture2D* TextureHandle;
	ID3D11ShaderResourceView* AssociatedShaderHandle;

	void LoadAssetData(bool RefreshAsset = false) override;
	void UnloadAsset() override;
};

struct /*ASSETSYSTEM_API*/ cFontAsset : public cAsset
{
	font_pack FontData;
	ID3D11ShaderResourceView* AtlasShaderHandle;

	u32 NumChars;
	u32 NumKernVals;
	char_entry* Characters;
	kern_entry* KernValues;

	void LoadAssetData(bool RefreshAsset = false) override;
	void UnloadAsset() override;
	char_entry FindCharEntryByAscii(u32 AsciiVal);
};