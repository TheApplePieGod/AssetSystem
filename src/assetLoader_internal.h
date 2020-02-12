#pragma once

// define functions not needed to be exposed in the api
namespace assetLoader
{
	void IterateOverDirectory(const char* DirectoryPath, std::ofstream* ofs, FILE* pak, bool Rebuild, bool GeneratePac, int* ID);

	// if ofs is nullptr it skips pac write (internal)
	std::string PackImage(std::string Path, std::string Filename, int AssetID, std::ofstream* ofs = nullptr, bool GeneratePac = false);
	std::string PackFont(std::string Path, char* FileBuffer, std::string Filename, int AssetID, std::ofstream* ofs = nullptr, bool GeneratePac = false);
	std::string PackMesh(std::string Path, std::string Filename, int AssetID, std::ofstream* ofs = nullptr, bool GeneratePac = false);

	bool CompareFiles(WIN32_FIND_DATA f1, WIN32_FIND_DATA f2);

	// internal
	void LoadImage(FILE* File, asset_header& Header, std::string Path, void (*Callback)(cTextureAsset*));
	void LoadFont(FILE* File, asset_header& Header, std::string Path, void (*Callback)(cFontAsset*));
	void LoadMesh(FILE* File, asset_header& Header, std::string Path, void (*Callback)(cMeshAsset*));
}