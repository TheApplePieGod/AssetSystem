#include "pch.h"
#include "../assetLoader.h"
#include "../assetLoader_internal.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION

#include "../lib/stb_image.h"
#include "../lib/stb_image_write.h"
#include "../lib/stb_truetype.h"

/*
* Returns new file path
*/
std::string assetLoader::PackImage(std::string Path, std::string Filename, int AssetID, std::ofstream* ofs, bool GeneratePac)
{
	asset_header Header;
	png_pack PNGPack;

	unsigned char* PixelData = stbi_load(Path.c_str(), &PNGPack.Width, &PNGPack.Height, &PNGPack.Channels, 4); // load the png

	// stb couldn't load image (probably corrupt)
	if (PixelData == nullptr)
	{
		return "NULL";
	}

	PNGPack.Channels = 4;  // makes every loaded texture have 4 channels
	u32 DataLength = PNGPack.Width * PNGPack.Height * PNGPack.Channels;

	Header.ID = AssetID;
	Header.Type = asset_type::Texture;
	Header.DataSize = DataLength;
	Header.ExtraSize = 0;
	Header.NextItem = sizeof(png_pack) + DataLength;
	strcpy(Header.Filename, Filename.c_str());

	if (ofs != nullptr)
	{
		if (GeneratePac)
		{
			ofs->write((char*)&Header, sizeof(asset_header));
			ofs->write((char*)&PNGPack, sizeof(png_pack));
			ofs->write((char*)PixelData, DataLength);
		}
	}

	remove(Path.c_str());
	std::string NewPath = Path;
	NewPath.erase(NewPath.length() - 4, 4);
	NewPath.push_back('.');
	NewPath += GetAssetSettings().AssetFileExtension;
	FILE* file;
	file = fopen(NewPath.c_str(), "wb");
	if (file)
	{
		fwrite((char*)&Header, sizeof(asset_header), 1, file);
		fwrite((char*)&PNGPack, sizeof(png_pack), 1, file);
		fwrite((char*)PixelData, sizeof(char), DataLength, file);
		fclose(file);
	}

	delete PixelData;
	return NewPath;
}

const char* assetLoader::PackImage(const char* Path, int AssetID)
{
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile(Path, &data);

	std::string ret = assetLoader::PackImage(Path, data.cFileName, AssetID);
	char* retptr = new char[ret.size() + 1];
	strcpy(retptr, ret.c_str());

	return retptr;
}


void assetLoader::LoadImage(FILE* File, asset_header& Header, std::string Path, void (*Callback)(cTextureAsset*))
{
	png_pack reading;
	size_t d = fread((char*)&reading, sizeof(reading), 1, File);
	cTextureAsset* TexAsset = new cTextureAsset;
	TexAsset->AssetID = Header.ID;
	TexAsset->Type = asset_type::Texture; // texture
	TexAsset->DataSize = Header.DataSize;
	TexAsset->Width = reading.Width;
	TexAsset->Height = reading.Height;
	TexAsset->Channels = reading.Channels;
	strncpy(TexAsset->Filename, Header.Filename, MAX_PATH);
	strncpy(TexAsset->Path, Path.c_str(), MAX_PATH);
	TexAsset->LoadAssetData();

	(*Callback)(TexAsset);
}

void assetLoader::LoadImage(const char* Path, void (*Callback)(cTextureAsset*))
{
	FILE* file = fopen(Path, "rb");
	if (file)
	{
		asset_header Header;
		fread((char*)&Header, 1, sizeof(asset_header), file);
		LoadImage(file, Header, Path, Callback);

		fclose(file);
	}
}
