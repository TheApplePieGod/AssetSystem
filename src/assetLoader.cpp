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

u32 assetLoader::AssetTypeArraySize()
{
	return (u32)AssetTypes.size();
}

asset_type* assetLoader::GetAssetTypeArray()
{
	return AssetTypes.data();
}

void SetAssetFieldsFromHeader(cAsset& Asset, asset_header& Header, std::string Path)
{
	Asset.AssetID = Header.ID;
	Asset.Type = Header.Type;
	Asset.DataSize = Header.RawDataSize;
	strncpy(Asset.Filename, Header.Filename, MAX_PATH);
	strncpy(Asset.Path, Path.c_str(), MAX_PATH);
}

std::string GetLowercaseFileExtension(const char* Filename)
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

s32 assetLoader::GetFileTypeID(const char* Filename)
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

bool assetLoader::LoadAllFileData(const char* Filepath, void*& Out_Data, u32& Out_FileSize)
{
	try
	{
		std::ifstream t(Filepath, std::ifstream::binary | std::ifstream::in);
		t.seekg(0, std::ios::end);
		Out_FileSize = (u32)t.tellg();
		t.seekg(0, std::ios::beg);

		Out_Data = new char[Out_FileSize];

		t.read((char*)Out_Data, Out_FileSize);
		t.close();
		return true;
	}
	catch (const std::exception& e)
	{
		e;
		return false;
	}
}

std::string RemoveFileAndGetNewPath(std::string Path)
{
	remove(Path.c_str());
	Path.erase(Path.length() - 4, 4);
	Path.push_back('.');
	Path += GetAssetSettings().AssetFileExtension;
	return Path;
}

bool CompareFiles(WIN32_FIND_DATA& f1, WIN32_FIND_DATA& f2)
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

std::string InternalPackAsset(std::string FullPath, WIN32_FIND_DATA File, bool GeneratePac, int* ID, std::ofstream* ofs)
{
	std::string NewPath = "";
	s32 TypeID = assetLoader::GetFileTypeID(File.cFileName);
	if (TypeID != -1)
	{
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
				NewPath = RemoveFileAndGetNewPath(FullPath);

				asset_header Header;
				if (ID != nullptr)
				{
					(*ID)++;
					Header.ID = *ID;
				}
				else
					Header.ID = -1; // only for debug ????? still not sure

				Header.Type = TypeID;
				Header.FormatVersion = GetAssetSettings().FormatVersion;
				Header.ExtraDataSize = ExtraDataSize;
				Header.RawDataSize = RawDataSize;
				strcpy(Header.Filename, File.cFileName);

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
				if (ExtraData != nullptr)
					delete[] ExtraData;
				if (RawData != nullptr)
					delete[] RawData;
			}
		}
		else if (GeneratePac) // copy file data into pac
		{
			if (ID != nullptr)
				(*ID)++;

			u32 filesize = (File.nFileSizeHigh * ((long)MAXDWORD + 1)) + File.nFileSizeLow;
			std::ifstream ifs(FullPath.c_str(), std::ifstream::binary);
			char* FileBuffer = new char[filesize];
			ifs.rdbuf()->sgetn(FileBuffer, filesize);
			((asset_header*)FileBuffer)->ID = *ID;
			ofs->write(FileBuffer, filesize);
			delete[] FileBuffer;
		}
	}

	return NewPath.c_str();
}

const char* assetLoader::PackAsset(const char* Path)
{
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile(Path, &data);

	std::string NewPath = InternalPackAsset(Path, data, false, nullptr, nullptr);

	char* retptr = new char[NewPath.size() + 1];
	strcpy(retptr, NewPath.c_str());

	return retptr;
}

void IterateOverDirectory(const char* DirectoryPath, std::ofstream* ofs, bool GeneratePac, int* ID, bool ScanNestedDirectories)
{
	std::string Path = std::string(DirectoryPath);
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile((Path + "*").c_str(), &data);
	std::vector<WIN32_FIND_DATA> FoundFiles;

	do
	{
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0) // sub folder found
		{
			if (ScanNestedDirectories)
				IterateOverDirectory((Path + data.cFileName + "\\").c_str(), ofs, GeneratePac, ID, ScanNestedDirectories);
		}
		else
		{
			std::string Filename = data.cFileName;

			int FilenameSize = (int)Filename.length();
			if (FilenameSize > 3) // make sure it isnt a random file
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
		std::string FullPath = (Path + FoundFiles[i].cFileName);
		InternalPackAsset(FullPath, FoundFiles[i], GeneratePac, ID, ofs);
	}
}

void assetLoader::ScanAssets(const char* DirectoryPath, bool GeneratePac, bool ScanNestedDirectories)
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

		IterateOverDirectory("assets\\", &ofs, GeneratePac, &ID, ScanNestedDirectories);
	}
}

// File pos must be directly after header
cAsset* InitializeNewAsset(asset_header& Header, std::string Path, FILE* File)
{
	asset_type& AssetType = assetLoader::GetAssetTypeFromID(Header.Type);

	char* ExtraData = nullptr;
	if (Header.ExtraDataSize > 0)
	{
		ExtraData = new char[Header.ExtraDataSize];
		fread(ExtraData, sizeof(char), Header.ExtraDataSize, File);
	}

	cAsset Asset;
	SetAssetFieldsFromHeader(Asset, Header, Path);

	cAsset* LoadedAsset = (*AssetType.InitializeData)(&Asset, ExtraData, Header.ExtraDataSize);
	(*AssetType.LoadCallback)(LoadedAsset);

	if (ExtraData != nullptr)
		delete[] ExtraData;

	return LoadedAsset;
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
				fseek(pak, Header.RawDataSize, SEEK_CUR);
			}
			else
				fseek(pak, (Header.ExtraDataSize + Header.RawDataSize), SEEK_CUR);
		}
	}
}

cAsset* assetLoader::InitializeAsset(const char* Path)
{
	FILE* File = fopen(Path, "rb");
	if (File)
	{
		asset_header Header;
		fread((char*)&Header, 1, sizeof(asset_header), File);

		if (Header.Type != -1 && Header.FormatVersion == GetAssetSettings().FormatVersion)
		{
			cAsset* returned = InitializeNewAsset(Header, Path, File);
			fclose(File);
			return returned;
		}
	}

	return nullptr;
}

void assetLoader::InitializeAssetsInDirectory(const char* DirectoryPath, bool ScanNestedDirectories)
{
	std::string Path = std::string(DirectoryPath);
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile((Path + "*").c_str(), &data);
	std::vector<WIN32_FIND_DATA> FoundFiles;

	do
	{
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0) // sub folder found
		{
			if (ScanNestedDirectories)
				InitializeAssetsInDirectory((Path + data.cFileName + "\\").c_str(), ScanNestedDirectories);
		}
		else
		{
			std::string Filename = data.cFileName;

			int FilenameSize = (int)Filename.length();
			if (FilenameSize > 3) // make sure it isnt a random file
			{
				if (GetFileTypeID(data.cFileName) == 0) // asset file
					FoundFiles.push_back(data);
				else
					continue;
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
		std::string FullPath = (Path + FoundFiles[i].cFileName);
		InitializeAsset(FullPath.c_str());
	}
}

void assetLoader::RenameAsset(cAsset* Asset, const char* NewFilename)
{
	std::string NFN = NewFilename;
	NFN += ".";
	std::string EAFFilename = NFN + GetAssetSettings().AssetFileExtension;
	std::string OGFilename = NFN + GetLowercaseFileExtension(Asset->Filename);

	std::string NewPath = Asset->Path;
	NewPath = NewPath.substr(0, NewPath.size() - strlen(Asset->Filename));
	NewPath += EAFFilename;

	std::ifstream t(Asset->Path, std::ifstream::binary | std::ifstream::in);
	t.seekg(0, std::ios::end);
	u32 Filesize = (u32)t.tellg();
	t.seekg(0, std::ios::beg);

	asset_header Header;
	u32 DataSize = Filesize - sizeof(asset_header);
	char* Data = new char[DataSize];
	t.read((char*)&Header, sizeof(asset_header));
	t.read(Data, DataSize);
	t.close();

	strcpy(Header.Filename, OGFilename.c_str());

	FILE* File = fopen(NewPath.c_str(), "wb");
	if (File)
	{
		fwrite(&Header, sizeof(asset_header), 1, File);
		fwrite(Data, sizeof(char), DataSize, File);
		fclose(File);
		remove(Asset->Path);
	}

	strcpy(Asset->Filename, OGFilename.c_str());
	strcpy(Asset->Path, NewPath.c_str());
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