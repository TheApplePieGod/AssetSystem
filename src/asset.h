#pragma once

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef int32_t b32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double d64;
typedef intptr_t intptr;
typedef uintptr_t uintptr;

#define SAFE_RELEASE(Pointer) if(Pointer) {Pointer->Release();}
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;} // Dereference a NULL pointer
#define MAX_FILE_EXTENSIONS 5

struct asset_header
{
	s32 ID;
	s32 Type;
	u32 FormatVersion;
	u32 ExtraDataSize; // size of extra stored data related to the asset
	u32 RawDataSize; // size of raw asset data
	char Filename[MAX_PATH];
};

struct asset_settings
{
	// load asset files in debug mode vs from pac file
	bool LoadFromPack = false;

	char PackFileName[20] = "assets.pac";

	// MUST only be 3 chars
	char AssetFileExtension[4] = "eaf";

	// change when big changes are made to asset structure
	u32 FormatVersion = 1;

	// size in pixels of each character in the generated font atlas 
	s32 FontSizePixels = 40;
};

asset_settings& GetAssetSettings();
void SetAssetSettings(asset_settings Settings);

struct cAsset
{
	s32 AssetID; 	// DEPRECATED (todo: move out of asset.h, keep in header for pac use)
	b32 Active = true;
	bool Loaded = false;
	s32 Type;
	void* Data = nullptr;
	u32 DataSize;
	char Filename[MAX_PATH];
	char Path[MAX_PATH]; // if reading from pac this will be the position inside, otherwise it will be the path of the file

	void CopyFields(cAsset* Asset)
	{
		AssetID = Asset->AssetID;
		Active = Asset->Active;
		Loaded = Asset->Loaded;
		Type = Asset->Type;
		Data = Asset->Data;
		DataSize = Asset->DataSize;
		strcpy(Filename, Asset->Filename);
		strcpy(Path, Asset->Path);
	}

	virtual void LoadAssetData(); // refresh asset is basically like reinitializing it so use if new data is possibly different
	virtual void UnloadAsset();
};

typedef bool(*t_GetDataForWriting)(char*&, char*&, u32&, u32&, char*); // must return total data size. char* is returned data and u32& is the size of the raw data
typedef cAsset* (*t_InitializeData)(cAsset*, char*, u32); // cAsset*: base asset values; reference to local var passed in so final asset must be allocated, char*: data to be processed (same data as extradata exported in GetDataForWriting), u32: size of data. Return pointer to new asset
typedef void (*t_LoadCallback)(cAsset*); // called after asset is initialized for the first time. useful for bookkeeping, etc

struct asset_type
{
	s32 TypeID;
	char TypeName[20];
	char FileExtensions[4][MAX_FILE_EXTENSIONS]; // Support for same asset type with multiple file extensions

	t_GetDataForWriting GetDataForWriting;
	t_InitializeData InitializeData;
	t_LoadCallback LoadCallback;
};