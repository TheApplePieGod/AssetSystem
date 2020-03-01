#pragma once

//TODO: update asset when filename / path changes

#include "asset.h"

namespace assetLoader
{
	/*
	* Scan directory and convert assets to asset format
	* GeneratePac: optionally generate the pack file
	*/
	void ScanAssets(const char* DirectoryPath, bool GeneratePac, bool ScanNestedDirectories);

	// Initialize/load asset files
	void InitializeAssetsInDirectory(const char* DirectoryPath, bool ScanNestedDirectories);
	// Pack file is assumed to be in the exe directory
	void InitializeAssetsFromPack();

	/*
	* Dynamically convert file into asset format on drive; returns new full path of the converted file
	* Path is full path of the file
	* AssetID MUST be unique from other assets
	* Does not generate/modify pack file; it must be recreated
	* Returned path is newed, must be deleted after use
	*/
	//const char* PackImage(const char* Path, int AssetID); // todo: PackAsset

	/*
	* Individually load assets from disk
	* Path is full path of the file
	* only supported in debug mode
	* (useful for dragging/dropping new assets)
	*/
	//void LoadImage(const char* Path, void (*Callback)(cTextureAsset*)); // todo: LoadAsset

	// typeid 0 is always the type of the assetfile defined in asset_settings
	void AddAssetType(asset_type NewType);
	asset_type& GetAssetTypeFromID(s32 TypeID);
	u32 AssetTypeArraySize();
	asset_type* GetAssetTypeArray();

	// Returns id of filetype (if supported) from any filename, otherwise returns -1
	s32 GetFileTypeID(char* Filename);

	// Exports loaded asset to exe directory (todo)
	//void ExportAsset(cAsset* Asset);

#ifdef ASSET_DIRECTX11
	// Call after LoadAssetData, for use with default types (font, image)
	void RegisterDXTexture(cAsset* Asset, bool GenerateMIPs, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
#endif
}