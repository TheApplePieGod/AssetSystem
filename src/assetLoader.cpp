#include "pch/pch.h"
#include <fstream>
#include <algorithm>
#include <shlwapi.h>
#include <locale>

#include "assetLoader.h"

#pragma warning( push )
#pragma warning( disable : 6387)
#pragma warning( disable : 6386)
#pragma warning( disable : 26444)

// define functions not needed to be exposed in the api
namespace assetLoader
{
	void IterateOverDirectory(const char* DirectoryPath, std::ofstream* ofs, FILE* pak, bool Rebuild, bool GeneratePac, int* ID);

	// if ofs is nullptr it skips pac write (internal)
	std::string PackImage(std::string Path, std::string Filename, int AssetID, std::ofstream* ofs = nullptr, bool GeneratePac = false);
	std::string PackFont(std::string Path, char* FileBuffer, std::string Filename, int AssetID, std::ofstream* ofs = nullptr, bool GeneratePac = false);

	bool CompareFiles(WIN32_FIND_DATA f1, WIN32_FIND_DATA f2);

	// internal
	void LoadImage(FILE* File, asset_header& Header, std::string Path, void (*Callback)(cTextureAsset*));
	void LoadFont(FILE* File, asset_header& Header, std::string Path, void (*Callback)(cFontAsset*));
}

/*
* Header* has a size of 1
* Returns new file path
*/
std::string assetLoader::PackImage(std::string Path, std::string Filename, int AssetID, std::ofstream* ofs, bool GeneratePac)
{
	asset_header Header;
	png_pack PNGPack;

	unsigned char* PixelData = stbi_load(Path.c_str(), &PNGPack.Width, &PNGPack.Height, &PNGPack.Channels, 4); // load the png
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

/*
* Header* has a size of 1
* Returns new file path
*/
std::string assetLoader::PackFont(std::string Path, char* FileBuffer, std::string Filename, int AssetID, std::ofstream* ofs, bool GeneratePac)
{
	std::vector<char_entry> Characters;
	std::vector<kern_entry> KernValues;
	font_pack FontPackData;
	s32 Width, Height, Ascent, Descent, LineGap, AdvanceWidth, LeftSideBearing;
	stbtt_fontinfo FontInfo;

	stbtt_InitFont(&FontInfo, (unsigned char*)FileBuffer, stbtt_GetFontOffsetForIndex((unsigned char*)FileBuffer, 0));
	stbtt_GetFontVMetrics(&FontInfo, &Ascent, &Descent, &LineGap);

	f32 PH = (f32)GetAssetSettings().FontSizePixels;
	f32 ScaleMultiplier = stbtt_ScaleForPixelHeight(&FontInfo, PH);
	Ascent *= (s32)ScaleMultiplier;
	Descent *= (s32)ScaleMultiplier;
	LineGap *= (s32)ScaleMultiplier;

	s32 TextureDim = 512 * (s32)(PH / 40);
	s32 AtlasDataSize = TextureDim * TextureDim * 1;
	u8* AtlasBuffer = (u8*)malloc(AtlasDataSize);
	memset(AtlasBuffer, 0, AtlasDataSize);

	s32 CharCounter = 1;
	s32 BoxHeight = GetAssetSettings().FontSizePixels;
	s32 BoxWidth = BoxHeight;
	s32 CurrentRow = 1;
	s32 CharsPerRow = TextureDim / BoxWidth;

	FontPackData.Ascent = Ascent;
	FontPackData.Descent = Descent;
	FontPackData.LineGap = LineGap;
	FontPackData.ScaleMultiplier = ScaleMultiplier;
	FontPackData.CharsPerRow = CharsPerRow;
	FontPackData.BoxHeight = BoxHeight;
	FontPackData.BoxWidth = BoxWidth;
	FontPackData.AtlasDim = TextureDim;
	FontPackData.AtlasDataSize = AtlasDataSize;

	s32 CharIndex = -1;
	for (s32 AsciiVal = 32; AsciiVal <= 126; AsciiVal++) // ascii values 32-126
	{
		CharIndex++;
		Characters.push_back(char_entry());
		s32 xOffset = 0;
		s32 yOffset = 0;

		u8* CharacterData = stbtt_GetCodepointBitmap(&FontInfo, 0.0f, ScaleMultiplier, AsciiVal, &Width, &Height, &xOffset, &yOffset);
		stbtt_GetCodepointHMetrics(&FontInfo, AsciiVal, &AdvanceWidth, &LeftSideBearing);

		Characters[CharIndex].AsciiValue = AsciiVal;
		Characters[CharIndex].Width = Width;
		Characters[CharIndex].Height = Height;
		Characters[CharIndex].OffsetX = (xOffset * ScaleMultiplier);
		Characters[CharIndex].OffsetY = (f32)yOffset;
		Characters[CharIndex].AdvanceWidth = (AdvanceWidth * ScaleMultiplier);
		Characters[CharIndex].LeftSideBearing = (LeftSideBearing * ScaleMultiplier);
		Characters[CharIndex].GlyphDataLength = Width * Height;

		s32 TLX = CharCounter * BoxWidth;
		s32 TLY = TextureDim * (BoxHeight * CurrentRow);

		Characters[CharIndex].TopLeftOffsetX = TLX;
		Characters[CharIndex].TopLeftOffsetY = TLY / TextureDim;

		for (s32 y = 0; y < Height; y++)
		{
			s32 Position = TLY + (TextureDim * y) + TLX;
			for (s32 x = 0; x < Width; x++)
			{
				AtlasBuffer[Position++] = *CharacterData++;
			}
		}

		CharCounter++;
		if (CharCounter % CharsPerRow == 0)
		{
			CurrentRow++;
			CharCounter = 1;
		}

		for (s32 KernVal = 32; KernVal <= 126; KernVal++)
		{
			float Spacing = (f32)stbtt_GetCodepointKernAdvance(&FontInfo, AsciiVal, KernVal) * ScaleMultiplier;

			if (Spacing != 0)
			{
				kern_entry entry;

				entry.AsciiVal1 = AsciiVal;
				entry.AsciiVal2 = KernVal;
				entry.Spacing = Spacing;

				KernValues.push_back(entry);
			}
		}
	}

	FontPackData.NumChars = (u32)Characters.size();

	asset_header Header;
	Header.ID = AssetID;
	Header.Type = asset_type::Font;
	Header.DataSize = FontPackData.AtlasDataSize;
	Header.ExtraSize = (u32)((sizeof(char_entry) * Characters.size()) + (sizeof(kern_entry) * KernValues.size()));
	Header.NextItem = sizeof(FontPackData) + Header.ExtraSize + FontPackData.AtlasDataSize;
	strcpy(Header.Filename, Filename.c_str());

	if (ofs != nullptr)
	{
		if (GeneratePac)
		{
			ofs->write((char*)&Header, sizeof(asset_header));
			ofs->write((char*)&FontPackData, sizeof(FontPackData));
			ofs->write((char*)Characters.data(), sizeof(char_entry) * Characters.size());
			ofs->write((char*)KernValues.data(), sizeof(kern_entry) * KernValues.size());
			ofs->write((char*)AtlasBuffer, FontPackData.AtlasDataSize);
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
		fwrite((char*)&FontPackData, sizeof(FontPackData), 1, file);
		fwrite((char*)Characters.data(), sizeof(char_entry), Characters.size(), file);
		fwrite((char*)KernValues.data(), sizeof(kern_entry), KernValues.size(), file);
		fwrite((char*)AtlasBuffer, sizeof(char), FontPackData.AtlasDataSize, file);
		fclose(file);
	}

	free(AtlasBuffer);
	return NewPath;
}

const char* assetLoader::PackFont(const char* Path, int AssetID)
{
	WIN32_FIND_DATA FileData;
	HANDLE hFind = FindFirstFile(Path, &FileData);

	// load font file
	u32 filesize = (FileData.nFileSizeHigh * ((long)MAXDWORD + 1)) + FileData.nFileSizeLow;
	std::ifstream ifs(Path, std::ifstream::binary);
	char* FileBuffer = new char[filesize];
	ifs.rdbuf()->sgetn(FileBuffer, filesize);
	ifs.close();

	std::string ret = PackFont(Path, FileBuffer, FileData.cFileName, AssetID);
	char* retptr = new char[ret.size() + 1];
	strcpy(retptr, ret.c_str());

	delete[] FileBuffer;
	return retptr;
}

asset_type assetLoader::GetFileType(char* Filename)
{
	std::locale loc;
	int Length = (int)strlen(Filename);

	if (Length < 3)
		return asset_type::Invalid;

	std::string Checking;
	Checking.push_back(std::tolower(Filename[Length - 3], loc));
	Checking.push_back(std::tolower(Filename[Length - 2], loc));
	Checking.push_back(std::tolower(Filename[Length - 1], loc));

	asset_settings set = GetAssetSettings();

	if (Checking == "png" || Checking == "jpg")
		return asset_type::Texture;
	else if (Checking == "ttf")
		return asset_type::Font;
	else if (Checking == set.AssetFileExtension)
		return asset_type::AssetFile;
	else if (Checking == set.SaveFileExtension)
		return asset_type::SaveFile;
	else
		return asset_type::Invalid;
}

void assetLoader::IterateOverDirectory(const char* DirectoryPath, std::ofstream* ofs, FILE* pak, bool Rebuild, bool GeneratePac, int* ID)
{
	std::string Path = std::string(DirectoryPath);
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile((Path + "*").c_str(), &data);
	std::vector<WIN32_FIND_DATA> FoundFiles;

	do
	{
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0)
			IterateOverDirectory((Path + data.cFileName + "\\").c_str(), ofs, pak, Rebuild, GeneratePac, ID);

		std::string Filename = data.cFileName;

		int FilenameSize = (int)Filename.length();
		if (FilenameSize > 3) // make sure it isnt a random file
		{
			FoundFiles.push_back(data);
		}
	} while (FindNextFile(hFind, &data));
	{
		FindClose(hFind);
	}

	// sort by filename because otherwise tiles arent sorted numerically
	std::sort(FoundFiles.begin(), FoundFiles.end(), CompareFiles);

	for (u32 i = 0; i < FoundFiles.size(); i++)
	{
		std::string Filename = FoundFiles[i].cFileName;
		switch (GetFileType(FoundFiles[i].cFileName))
		{
		case AssetFile:
		{
			if (GeneratePac)
			{
				u32 filesize = (FoundFiles[i].nFileSizeHigh * ((long)MAXDWORD + 1)) + FoundFiles[i].nFileSizeLow;
				std::ifstream ifs(Path + FoundFiles[i].cFileName, std::ifstream::binary);
				char* FileBuffer = new char[filesize];
				ifs.rdbuf()->sgetn(FileBuffer, filesize);
				ofs->write(FileBuffer, filesize);
				delete[] FileBuffer;
			}
		} break;

		case Texture:
		{
			(*ID)++;
			PackImage((Path + FoundFiles[i].cFileName), Filename, *ID, ofs, GeneratePac);
		} break;

		case Font:
		{
			// load font file
			u32 filesize = (FoundFiles[i].nFileSizeHigh * ((long)MAXDWORD + 1)) + FoundFiles[i].nFileSizeLow;
			std::ifstream ifs(Path + FoundFiles[i].cFileName, std::ifstream::binary);
			char* FileBuffer = new char[filesize];
			ifs.rdbuf()->sgetn(FileBuffer, filesize);
			ifs.close();

			(*ID)++;
			PackFont((Path + data.cFileName), FileBuffer, Filename, *ID, ofs, GeneratePac);
			delete[] FileBuffer;
		} break;

		default:
		{ continue; } break;
		}
	}
}

void assetLoader::ScanAssets(bool Rebuild, bool GeneratePac)
{
	std::ofstream ofs;
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile("assets\\*", &data); // asset directory
	int ID = 0;

	if (hFind != INVALID_HANDLE_VALUE)
	{
		FILE* pak = nullptr;

		if (GeneratePac)
		{
			pak = fopen(GetAssetSettings().PackFileName, "rb");
			ofs.open("Temp.pac", std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
			// force rebuild if no pak file is present
			if (!pak)
				Rebuild = true;
		}

		IterateOverDirectory("assets\\", &ofs, pak, Rebuild, GeneratePac, &ID);

		if (GeneratePac)
		{
			ofs.close();

			if (pak)
				fclose(pak);

			ofs.open(GetAssetSettings().PackFileName, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
			WIN32_FIND_DATA tempdata;
			std::ifstream ifs("Temp.pac", std::ifstream::binary);
			FindFirstFile("Temp.pac", &tempdata);
			u32 size = (tempdata.nFileSizeHigh * ((long)MAXDWORD + 1)) + tempdata.nFileSizeLow;
			char* FinalBuffer = new char[size];
			ifs.rdbuf()->sgetn(FinalBuffer, size);

			ofs.write(FinalBuffer, size);

			ofs.close();
			ifs.close();
			remove("Temp.pac");
		}
	}
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

void assetLoader::LoadFont(FILE* File, asset_header& Header, std::string Path, void (*Callback)(cFontAsset*))
{
	cFontAsset* FontAsset = new cFontAsset;
	fread((char*)&FontAsset->FontData, sizeof(font_pack), 1, File);
	char_entry* CharArray = new char_entry[FontAsset->FontData.NumChars * sizeof(char_entry)];
	kern_entry* KernArray = new kern_entry[Header.ExtraSize - (FontAsset->FontData.NumChars * sizeof(char_entry))];
	fread((char*)CharArray, sizeof(char_entry), FontAsset->FontData.NumChars, File);
	fread((char*)KernArray, 1, Header.ExtraSize - (FontAsset->FontData.NumChars * sizeof(char_entry)), File);
	FontAsset->Characters = CharArray;
	FontAsset->KernValues = KernArray;
	FontAsset->AssetID = Header.ID;
	FontAsset->DataSize = Header.DataSize;
	FontAsset->Type = asset_type::Font; // font
	strncpy(FontAsset->Filename, Header.Filename, MAX_PATH);
	strncpy(FontAsset->Path, Path.c_str(), MAX_PATH);
	FontAsset->LoadAssetData();

	(*Callback)(FontAsset);
}

void assetLoader::LoadFont(const char* Path, void (*Callback)(cFontAsset*))
{
	FILE* file = fopen(Path, "rb");
	if (file)
	{
		asset_header Header;
		fread((char*)&Header, 1, sizeof(asset_header), file);
		LoadFont(file, Header, Path, Callback);

		fclose(file);
	}
}

void assetLoader::InitializeAssetsFromPac(asset_load_callbacks* Callbacks) // assets should never change, so new is OK (later add int array of ids to load)
{
	FILE* pak = fopen(GetAssetSettings().PackFileName, "rb");
	char Buffer[20];
	s32 PrevIndex = -1;
	asset_header SearchingHeader;
	while (true) // iterate through assets (optimize?)
	{
		fread((char*)&SearchingHeader, 1, sizeof(SearchingHeader), pak);
		if (PrevIndex == SearchingHeader.ID) // when all assets have been read
		{
			fclose(pak);
			return;
		}
		else
		{
			PrevIndex = SearchingHeader.ID; // TODO: stop using new
			int Position = ftell(pak) - sizeof(SearchingHeader);
			_snprintf_s(Buffer, sizeof(Buffer), "%d", Position);
			std::string Path = Buffer;

			if (SearchingHeader.Type == asset_type::Texture) // texture asset found
				LoadImage(pak, SearchingHeader, Path, Callbacks->ImageCallback);

			else if (SearchingHeader.Type == asset_type::Font) // font asset found
				LoadFont(pak, SearchingHeader, Path, Callbacks->FontCallback);

			fseek(pak, SearchingHeader.DataSize, SEEK_CUR); // set pos to next asset
		}
	}
}

bool assetLoader::CompareFiles(WIN32_FIND_DATA f1, WIN32_FIND_DATA f2)
{
	wchar_t wtext[MAX_PATH];
	mbstowcs(wtext, f1.cFileName, MAX_PATH + 1);//Plus null
	wchar_t wtext2[MAX_PATH];
	mbstowcs(wtext2, f2.cFileName, MAX_PATH + 1);//Plus null

	int Comp = StrCmpLogicalW(wtext, wtext2);

	if (Comp == 0)
	{
		Assert(1 == 2);
		return false;
	}
	else if (Comp == 1)
		return false;
	else
		return true;
}

void assetLoader::InitializeAssetsInDirectory(const char* DirectoryPath, asset_load_callbacks* Callbacks)
{
	std::string Path = std::string(DirectoryPath);
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile((Path + "*").c_str(), &data);
	std::vector<WIN32_FIND_DATA> FoundFiles;

	do
	{
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0)
			InitializeAssetsInDirectory((Path + data.cFileName + "\\").c_str(), Callbacks);

		std::string Filename = data.cFileName;

		int FilenameSize = (int)Filename.length();
		if (FilenameSize > 3) // make sure it isnt a random file
		{
			switch (GetFileType(data.cFileName))
			{
			case AssetFile:
			{
				FoundFiles.push_back(data);
			} break;

			default:
			{ continue; } break;
			}
		}
	} while (FindNextFile(hFind, &data));
	{
		FindClose(hFind);
	}

	// sort by filename because otherwise tiles arent sorted numerically
	std::sort(FoundFiles.begin(), FoundFiles.end(), CompareFiles);

	for (u32 i = 0; i < FoundFiles.size(); i++)
	{
		FILE* file = fopen((Path + FoundFiles[i].cFileName).c_str(), "rb");
		asset_header Header;
		fread((char*)&Header, 1, sizeof(asset_header), file);
		std::string FullPath = Path + FoundFiles[i].cFileName;

		switch (Header.Type)
		{
			case Texture:
			{
				LoadImage(file, Header, FullPath, Callbacks->ImageCallback);
			} break;

			case Font:
			{
				LoadFont(file, Header, FullPath, Callbacks->FontCallback);
			} break;

			default:
			{} break;
		}
		fclose(file);
	}
}

/*
* Export asset in exe directory
* TODO: font export
*/
void assetLoader::ExportAsset(cAsset* Asset)
{
	if (Asset->Loaded)
	{
		switch (Asset->Type)
		{
			default:
			{} break;

			case Font:
			{
				//todo
				cFontAsset* Font = (cFontAsset*)Asset;
			} break;

			case Texture:
			{
				cTextureAsset* Tex = (cTextureAsset*)Asset;
				stbi_write_png(Tex->Filename, Tex->Width, Tex->Height, Tex->Channels, Tex->Data, Tex->Width * Tex->Channels);
			} break;
		}
	}
}

#ifdef ASSET_DIRECTX11
void assetLoader::RegisterDXTexture(cAsset* Asset, bool GenerateMIPs, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext)
{
	Assert(Asset->Data != nullptr && Asset->Loaded == true);

	D3D11_SUBRESOURCE_DATA TextureData = {};
	D3D11_TEXTURE2D_DESC desc;

	switch (Asset->Type)
	{
		default:
		{ Assert(1 == 2); } break;

		case Font:
		{
			cFontAsset* Font = (cFontAsset*)Asset;
			TextureData.SysMemPitch = Font->FontData.AtlasDim * 1; // assumes one channel
			desc.Width = Font->FontData.AtlasDim;
			desc.Height = Font->FontData.AtlasDim;
			desc.Format = DXGI_FORMAT_A8_UNORM; // should always be 1
		} break;

		case Texture:
		{
			cTextureAsset* Tex = (cTextureAsset*)Asset;
			TextureData.SysMemPitch = Tex->Channels * Tex->Width;
			desc.Width = Tex->Width;
			desc.Height = Tex->Height;
			switch (Tex->Channels)
			{
				case 0:
				case 2:
				{
					Assert(1 == 2); // cant have 0 or 2 channel texture
				}
				case 1:
				{
					desc.Format = DXGI_FORMAT_A8_UNORM;
					break;
				}
				case 3:
				case 4:
				{
					desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
					break;
				}
			}
		} break;
	}

	desc.MiscFlags = GenerateMIPs ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
	desc.MipLevels = GenerateMIPs ? 0 : 1;
	desc.ArraySize = 1;

	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT; // Only the GPU has access to read this resource
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0; // No CPU access is required

	ID3D11Texture2D* pTexture = NULL;
	HRESULT hr = Device->CreateTexture2D(&desc, NULL, &pTexture);
	if (FAILED(hr))
		Assert(1 == 2);

	DeviceContext->UpdateSubresource(pTexture, 0, NULL, Asset->Data, TextureData.SysMemPitch, 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = GenerateMIPs ? 0 : 1;

	ID3D11ShaderResourceView* shader = NULL;
	hr = Device->CreateShaderResourceView((ID3D11Resource*)pTexture, NULL, &shader);
	if (FAILED(hr))
		Assert(1 == 2);

	if (GenerateMIPs)
		DeviceContext->GenerateMips(shader);

	switch (Asset->Type)
	{
		default:
		{ Assert(1 == 2); } break;

		case Font:
		{
			cFontAsset* Font = (cFontAsset*)Asset;
			Font->AtlasShaderHandle = shader;
		} break;

		case Texture:
		{
			cTextureAsset* Tex = (cTextureAsset*)Asset;
			Tex->ShaderHandle = shader;
			Tex->TextureHandle = pTexture;
		} break;
	}
}
#endif

#pragma warning( pop )