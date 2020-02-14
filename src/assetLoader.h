#pragma once

//TODO: update asset when filename changes

#include "asset.h"

typedef void (*t_ImageCallback)(cTextureAsset*);
typedef void (*t_FontCallback)(cFontAsset*);
typedef void (*t_MeshCallback)(cMeshAsset*);

namespace assetLoader
{
	struct asset_load_callbacks
	{
		t_ImageCallback ImageCallback;
		t_FontCallback FontCallback;
		t_MeshCallback MeshCallback;
	};

	/*
	* Scan directory and convert assets to asset format
	* GeneratePac: optionally generate the pack file
	*/
	void ScanAssets(const char* DirectoryPath, bool GeneratePac);

	// Initialize/load asset files
	void InitializeAssetsInDirectory(const char* DirectoryPath, asset_load_callbacks* Callbacks);
	// Pack file is assumed to be in the exe directory
	void InitializeAssetsFromPac(asset_load_callbacks* Callbacks);

	/*
	* Dynamically convert file into asset format on drive; returns new full path of the converted file
	* Path is full path of the file
	* AssetID MUST be unique from other assets
	* Does not generate/modify pack file; it must be recreated
	* Returned path is newed, must be deleted after use
	*/
	//const char* PackImage(const char* Path, int AssetID);

	/*
	* Individually load assets from disk
	* Path is full path of the file
	* only supported in debug mode
	* (useful for dragging/dropping new assets)
	*/
	//void LoadImage(const char* Path, void (*Callback)(cTextureAsset*));

	// typeid 0 is always the type of the assetfile defined in asset_settings
	void AddAssetType(asset_type NewType);
	asset_type& GetAssetTypeFromID(s32 TypeID);

	// Returns id of filetype (if supported) from any filename, otherwise returns -1
	s32 GetFileTypeID(char* Filename);

	// Exports loaded asset to exe directory
	void ExportAsset(cAsset* Asset);

#ifdef ASSET_DIRECTX11
	// Call after LoadAssetData if asset has a texture (font, texture)
	void RegisterDXTexture(cAsset* Asset, bool GenerateMIPs, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
#endif
}