#pragma once

#include "asset.h"

typedef void (*t_ImageCallback)(cTextureAsset*);
typedef void (*t_FontCallback)(cFontAsset*);

#ifdef ASSET_FORWARD_DECLARATIONS
struct WIN32_FIND_DATA;
namespace std { class string; class ofstream; }
#endif

namespace assetLoader
{
	struct asset_load_callbacks
	{
		t_ImageCallback ImageCallback;
		t_FontCallback FontCallback;
	};

	void IterateOverDirectory(const char* DirectoryPath, std::ofstream* ofs, FILE* pak, bool Rebuild, bool GeneratePac, int* ID);
	void ScanAssets(bool Rebuild, bool GeneratePac);
	void InitializeAssetsInDirectory(const char* DirectoryPath, asset_load_callbacks* Callbacks);
	void InitializeAssetsFromPac(asset_load_callbacks* Callbacks);

	// if ofs is nullptr it skips pac write
	std::string PackImage(std::string Path, std::string Filename, int AssetID, std::ofstream* ofs = nullptr, bool GeneratePac = false);
	std::string PackFont(std::string Path, char* FileBuffer, std::string Filename, int AssetID, std::ofstream* ofs = nullptr, bool GeneratePac = false);

	void LoadImage(FILE* File, asset_header& Header, std::string Path, void (*Callback)(cTextureAsset*));
	void LoadImage(std::string Path, void (*Callback)(cTextureAsset*));

	void LoadFont(FILE* File, asset_header& Header, std::string Path, void (*Callback)(cFontAsset*));
	asset_type GetFileType(char* Filename);

	bool CompareFiles(WIN32_FIND_DATA f1, WIN32_FIND_DATA f2);
}