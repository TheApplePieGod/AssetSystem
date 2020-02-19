#include "pch.h"
#include "assetLoader.h"

#include "../lib/stb_image_write.h"

#pragma warning( push )
#pragma warning( disable : 6387)
#pragma warning( disable : 6386)
#pragma warning( disable : 26444)

extern std::vector<asset_type> AssetTypes;

void assetLoader::AddAssetType(asset_type NewType)
{
	// verify id is unique and non zero
	Assert(NewType.TypeID != 0);
	for (asset_type& type : AssetTypes)
	{
		Assert(type.TypeID != NewType.TypeID);
	}

	AssetTypes.push_back(NewType);
}

asset_type& assetLoader::GetAssetTypeFromID(s32 TypeID)
{
	for (asset_type& type : AssetTypes)
	{
		if (type.TypeID == TypeID)
			return type;
	}

	throw std::out_of_range("TypeID does not exist");
}

void SetAssetFieldsFromHeader(cAsset& Asset, asset_header& Header, std::string Path)
{
	Asset.AssetID = Header.ID;
	Asset.Type = Header.Type;
	Asset.DataSize = Header.RawDataSize;
	strncpy(Asset.Filename, Header.Filename, MAX_PATH);
	strncpy(Asset.Path, Path.c_str(), MAX_PATH);
}

std::string GetLowercaseFileExtension(char* Filename)
{
	std::locale loc;
	int Length = (int)strlen(Filename);

	if (Length < 3)
		return "invalid";

	std::string Checking;
	Checking.push_back(std::tolower(Filename[Length - 3], loc));
	Checking.push_back(std::tolower(Filename[Length - 2], loc));
	Checking.push_back(std::tolower(Filename[Length - 1], loc));

	return Checking;
}

s32 assetLoader::GetFileTypeID(char* Filename)
{
	std::string Checking = GetLowercaseFileExtension(Filename);

	if (Checking == "invalid")
		return -1;

	asset_settings set = GetAssetSettings();

	if (Checking == set.AssetFileExtension) // asset file found
		return 0;

	for (asset_type& type : AssetTypes)
	{
		for (u32 i = 0; i < MAX_FILE_EXTENSIONS; i++)
		{
			if (type.FileExtensions[i] == Checking)
				return type.TypeID;
		}
	}

	return -1;
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

std::string RemoveFileAndGetNewPath(std::string Path)
{
	remove(Path.c_str());
	Path.erase(Path.length() - 4, 4);
	Path.push_back('.');
	Path += GetAssetSettings().AssetFileExtension;
	return Path;
}

bool CompareFiles(WIN32_FIND_DATA f1, WIN32_FIND_DATA f2)
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

void IterateOverDirectory(const char* DirectoryPath, std::ofstream* ofs, bool GeneratePac, int* ID)
{
	std::string Path = std::string(DirectoryPath);
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile((Path + "*").c_str(), &data);
	std::vector<WIN32_FIND_DATA> FoundFiles;

	do
	{
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0)
			IterateOverDirectory((Path + data.cFileName + "\\").c_str(), ofs, GeneratePac, ID);

		std::string Filename = data.cFileName;

		int FilenameSize = (int)Filename.length();
		if (FilenameSize > 3) // make sure it isnt a random file
			FoundFiles.push_back(data);

	} while (FindNextFile(hFind, &data));
	{
		FindClose(hFind);
	}

	// sort by filename because otherwise tiles arent sorted numerically
	std::sort(FoundFiles.begin(), FoundFiles.end(), CompareFiles);

	for (u32 i = 0; i < FoundFiles.size(); i++)
	{
		std::string Filename = FoundFiles[i].cFileName;

		s32 TypeID = assetLoader::GetFileTypeID(FoundFiles[i].cFileName);
		if (TypeID != -1)
		{
			std::string FullPath = (Path + FoundFiles[i].cFileName);

			if (TypeID != 0) // 0 = type of asset file
			{
				asset_type& AssetType = assetLoader::GetAssetTypeFromID(TypeID);
				char* ExtraData = nullptr;
				char* RawData = nullptr;
				u32 ExtraDataSize = 0;
				u32 RawDataSize = 0;
				bool DeleteData = (*AssetType.GetDataForWriting)(ExtraData, RawData, ExtraDataSize, RawDataSize, (char*)FullPath.c_str());

				if (ExtraDataSize + RawDataSize > 0)
				{
					std::string NewPath = RemoveFileAndGetNewPath(FullPath);

					(*ID)++;
					asset_header Header;
					Header.ID = *ID;
					Header.Type = TypeID;
					Header.FormatVersion = GetAssetSettings().FormatVersion;
					Header.ExtraDataSize = ExtraDataSize;
					Header.RawDataSize = RawDataSize;
					strcpy(Header.Filename, FoundFiles[i].cFileName);

					FILE* File;
					File = fopen(NewPath.c_str(), "wb");
					if (File)
					{
						fwrite((char*)&Header, sizeof(Header), 1, File);
						fwrite(ExtraData, sizeof(char), ExtraDataSize, File);
						fwrite(RawData, sizeof(char), RawDataSize, File);
						fclose(File);
					}

					if (GeneratePac)
					{
						ofs->write((char*)&Header, sizeof(Header));
						ofs->write(ExtraData, ExtraDataSize);
						ofs->write(RawData, RawDataSize);
					}
				}

				if (DeleteData)
				{
					delete[] ExtraData;
					delete[] RawData;
				}
			}
			else if (GeneratePac) // copy file data into pac
			{
				u32 filesize = (FoundFiles[i].nFileSizeHigh * ((long)MAXDWORD + 1)) + FoundFiles[i].nFileSizeLow;
				std::ifstream ifs(FullPath.c_str(), std::ifstream::binary);
				char* FileBuffer = new char[filesize];
				ifs.rdbuf()->sgetn(FileBuffer, filesize);
				ofs->write(FileBuffer, filesize);
				delete[] FileBuffer;
			}
		}
	}
}

void assetLoader::ScanAssets(const char* DirectoryPath, bool GeneratePac)
{
	std::string Path = std::string(DirectoryPath);
	std::ofstream ofs;
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile((Path + "*").c_str(), &data); // asset directory
	int ID = 0;

	if (hFind != INVALID_HANDLE_VALUE)
	{
		if (GeneratePac)
			ofs.open(GetAssetSettings().PackFileName, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);

		IterateOverDirectory("assets\\", &ofs, GeneratePac, &ID);
	}
}

// File pos must be directly after header
void InitializeNewAsset(asset_header& Header, std::string Path, FILE* File)
{
	asset_type& AssetType = assetLoader::GetAssetTypeFromID(Header.Type);

	char* ExtraData = new char[Header.ExtraDataSize];
	cAsset Asset;
	SetAssetFieldsFromHeader(Asset, Header, Path);

	fread(ExtraData, sizeof(char), Header.ExtraDataSize, File);

	cAsset* LoadedAsset = (*AssetType.InitializeData)(&Asset, ExtraData, Header.ExtraDataSize);
	(*AssetType.LoadCallback)(LoadedAsset);
	delete[] ExtraData;
}

void assetLoader::InitializeAssetsFromPack() // assets should never change, so new is OK (later add int array of ids to load)
{
	FILE* pak = fopen(GetAssetSettings().PackFileName, "rb");
	char Buffer[20];
	s32 PrevIndex = -1;
	asset_header Header;
	while (true) // iterate through assets (optimize?)
	{
		fread((char*)&Header, 1, sizeof(Header), pak);
		if (PrevIndex == Header.ID) // when all assets have been read
		{
			fclose(pak);
			return;
		}
		else
		{
			PrevIndex = Header.ID;

			if (Header.Type != -1 && Header.FormatVersion == GetAssetSettings().FormatVersion)
			{
				int Position = ftell(pak) - sizeof(Header);
				_snprintf_s(Buffer, sizeof(Buffer), "%d", Position);
				std::string PositionInPack = Buffer;

				InitializeNewAsset(Header, PositionInPack, pak);
			}
			else
				fseek(pak, (Header.ExtraDataSize + Header.RawDataSize), SEEK_CUR);
		}
	}
}

void assetLoader::InitializeAssetsInDirectory(const char* DirectoryPath)
{
	std::string Path = std::string(DirectoryPath);
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile((Path + "*").c_str(), &data);
	std::vector<WIN32_FIND_DATA> FoundFiles;

	do
	{
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0)
			InitializeAssetsInDirectory((Path + data.cFileName + "\\").c_str());

		std::string Filename = data.cFileName;

		int FilenameSize = (int)Filename.length();
		if (FilenameSize > 3) // make sure it isnt a random file
		{
			if (GetFileTypeID(data.cFileName) == 0) // asset file
				FoundFiles.push_back(data);
			else
				continue;
		}
	} while (FindNextFile(hFind, &data));
	{
		FindClose(hFind);
	}

	// sort by filename because otherwise tiles arent sorted numerically
	std::sort(FoundFiles.begin(), FoundFiles.end(), CompareFiles);

	for (u32 i = 0; i < FoundFiles.size(); i++)
	{
		FILE* File = fopen((Path + FoundFiles[i].cFileName).c_str(), "rb");
		if (File)
		{
			asset_header Header;
			fread((char*)&Header, 1, sizeof(asset_header), File);
			std::string FullPath = Path + FoundFiles[i].cFileName;

			if (Header.Type != -1 && Header.FormatVersion == GetAssetSettings().FormatVersion)
			{
				InitializeNewAsset(Header, FullPath, File);
			}
			fclose(File);
		}
	}
}

/*
* Export asset in exe directory
* TODO: font export (save original file in asset file?)
*/
//void assetLoader::ExportAsset(cAsset* Asset)
//{
//	if (Asset->Loaded)
//	{
//		switch (Asset->Type)
//		{
//			default:
//			{} break;
//
//			case Font:
//			{
//				//todo (not supported?)
//				cFontAsset* Font = (cFontAsset*)Asset;
//			} break;
//
//			case Texture:
//			{
//				cTextureAsset* Tex = (cTextureAsset*)Asset;
//				stbi_write_png(Tex->Filename, Tex->Width, Tex->Height, Tex->Channels, Tex->Data, Tex->Width * Tex->Channels);
//			} break;
//
//			case Mesh:
//			{
//				// todo
//				cMeshAsset* Mesh = (cMeshAsset*)Asset;
//			} break;
//		}
//	}
//}

#pragma warning( pop )