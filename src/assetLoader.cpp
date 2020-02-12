#include "pch.h"
#include "assetLoader.h"
#include "assetLoader_internal.h"

#include "../lib/stb_image_write.h"

#pragma warning( push )
#pragma warning( disable : 6387)
#pragma warning( disable : 6386)
#pragma warning( disable : 26444)

const char* assetLoader::GetLowercaseFileExtension(char* Filename)
{
	std::locale loc;
	int Length = (int)strlen(Filename);

	if (Length < 3)
		return "invalid";

	std::string Checking;
	Checking.push_back(std::tolower(Filename[Length - 3], loc));
	Checking.push_back(std::tolower(Filename[Length - 2], loc));
	Checking.push_back(std::tolower(Filename[Length - 1], loc));

	return Checking.c_str();
}

asset_type assetLoader::GetFileType(char* Filename)
{
	std::string Checking = GetLowercaseFileExtension(Filename);

	if (Checking == "invalid")
		return asset_type::Invalid;

	asset_settings set = GetAssetSettings();

	if (Checking == "png" || Checking == "jpg" || Checking == "tga")
		return asset_type::Texture;
	else if (Checking == "ttf")
		return asset_type::Font;
	else if (Checking == set.AssetFileExtension)
		return asset_type::AssetFile;
	else if (Checking == set.SaveFileExtension)
		return asset_type::SaveFile;
	else if (Checking == "fbx")
		return asset_type::Mesh;
	else
		return asset_type::Invalid;
}

// windows
char* LoadAllFileData(WIN32_FIND_DATA Data, const char* Path)
{
	u32 filesize = (Data.nFileSizeHigh * ((long)MAXDWORD + 1)) + Data.nFileSizeLow;
	std::ifstream ifs(Path, std::ifstream::binary);
	char* FileBuffer = new char[filesize];
	ifs.rdbuf()->sgetn(FileBuffer, filesize);
	ifs.close();

	return FileBuffer;
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
				// load file
				char* FileBuffer = LoadAllFileData(FoundFiles[i], (Path + FoundFiles[i].cFileName).c_str());

				(*ID)++;
				PackFont((Path + data.cFileName), FileBuffer, Filename, *ID, ofs, GeneratePac);
				delete[] FileBuffer;
			} break;

			case Mesh:
			{
				(*ID)++;
				PackMesh((Path + FoundFiles[i].cFileName), Filename, *ID, ofs, GeneratePac);
			} break;

			default:
			{ continue; } break;
		}
	}
}

void assetLoader::ScanAssets(const char* DirectoryPath, bool GeneratePac)
{
	bool Rebuild = true;
	std::string Path = std::string(DirectoryPath);
	std::ofstream ofs;
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile((Path + "*").c_str(), &data); // asset directory
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
			PrevIndex = SearchingHeader.ID;
			int Position = ftell(pak) - sizeof(SearchingHeader);
			_snprintf_s(Buffer, sizeof(Buffer), "%d", Position);
			std::string Path = Buffer;

			switch (SearchingHeader.Type)
			{
				case Texture:
				{ LoadImage(pak, SearchingHeader, Path, Callbacks->ImageCallback); } break;

				case Font:
				{ LoadFont(pak, SearchingHeader, Path, Callbacks->FontCallback); } break;

				case Mesh:
				{ LoadMesh(pak, SearchingHeader, Path, Callbacks->MeshCallback); } break;

				default:
				{} break;
			}

			fseek(pak, SearchingHeader.DataSize, SEEK_CUR); // set pos to next asset
		}
	}
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
			{ LoadImage(file, Header, FullPath, Callbacks->ImageCallback); } break;

			case Font:
			{ LoadFont(file, Header, FullPath, Callbacks->FontCallback); } break;

			case Mesh:
			{ LoadMesh(file, Header, FullPath, Callbacks->MeshCallback); } break;

			default:
			{} break;
		}
		fclose(file);
	}
}

/*
* Export asset in exe directory
* TODO: font export (save original file in asset file?)
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
				//todo (not supported?)
				cFontAsset* Font = (cFontAsset*)Asset;
			} break;

			case Texture:
			{
				cTextureAsset* Tex = (cTextureAsset*)Asset;
				stbi_write_png(Tex->Filename, Tex->Width, Tex->Height, Tex->Channels, Tex->Data, Tex->Width * Tex->Channels);
			} break;

			case Mesh:
			{
				// todo
				cMeshAsset* Mesh = (cMeshAsset*)Asset;
			} break;
		}
	}
}

#pragma warning( pop )