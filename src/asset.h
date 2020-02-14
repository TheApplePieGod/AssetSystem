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

struct png_pack // change to texture_pack
{
	s32 Width;
	s32 Height;
	s32 Channels;
};

struct kern_entry
{
	u32 AsciiVal1;
	u32 AsciiVal2;
	float Spacing;
};

struct char_entry // assumes one channel texture
{
	u32 AsciiValue;
	u32 Width;
	u32 Height;
	f32 OffsetX;
	f32 OffsetY;
	f32 AdvanceWidth;
	f32 LeftSideBearing;
	u32 GlyphDataLength;
	s32 TopLeftOffsetX;
	s32 TopLeftOffsetY;
};

struct font_pack // assumes one channel texture
{
	s32 Ascent;
	s32 Descent;
	s32 LineGap;
	f32 ScaleMultiplier;
	s32 CharsPerRow;
	s32 BoxHeight;
	s32 BoxWidth;
	s32 AtlasDim;
	s32 AtlasDataSize;
	u32 NumChars;
};

struct mesh_pack
{
	u32 NumVertices;
};

struct asset_header
{
	s32 ID;
	s32 Type;
	u32 TotalSize; // total size of extra data + raw data size, next asset starts after this
	u32 RawDataSize; // tells the size of raw asset data
	char Filename[MAX_PATH];

	inline bool operator==(asset_header A)
	{
		if (A.ID == ID &&
			A.Type == Type &&
			A.RawDataSize == RawDataSize &&
			A.TotalSize == TotalSize)
			return true;
		else
			return false;
	}
};

struct asset_settings
{
	// load asset files in debug mode vs from pac file
	bool LoadFromPack = false;

	char PackFileName[20] = "assets.pac";

	// MUST only be 3 chars
	char AssetFileExtension[4] = "eaf";

	// size in pixels of each character in the generated font atlas 
	int FontSizePixels = 40;
};

// Structure used when storing vertices from imported mesh
struct mesh_vertex
{
	// Position in x/y plane
	f32 x, y, z;

	// UV coordinates
	f32 u, v;

	// Normals
	f32 nx, ny, nz;

	// Tangents
	f32 tx, ty, tz;
};

asset_settings GetAssetSettings();
void SetAssetSettings(asset_settings Settings);

struct cAsset
{
	s32 AssetID; 	// internal use
	b32 Active;
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

typedef u32(*t_GetDataForWriting)(char*&, u32&, char*); // must return total data size. char* is returned data and u32& is the size of the raw data
typedef void(*t_InitializeData)(cAsset*, char*, u32); // cAsset*: base values for the asset, char*: data to be processed (same data exported in GetDataForWriting), u32: size of data

struct asset_type
{
	s32 TypeID;
	char TypeName[20];
	char FileExtension[4];

	t_GetDataForWriting GetDataForWriting;
	t_InitializeData InitializeData;
};

struct cTextureAsset : public cAsset
{
	s32 Width;
	s32 Height;
	s32 Channels;

#ifdef ASSET_DIRECTX11
	ID3D11Texture2D* TextureHandle = nullptr;
	ID3D11ShaderResourceView* ShaderHandle = nullptr;
#endif

	// Register texture with rendering sdk by calling assetLoader::Register(your_sdk)Texture
	void UnloadAsset() override;
};

struct cFontAsset : public cAsset
{
	font_pack FontData;

#ifdef ASSET_DIRECTX11
	ID3D11ShaderResourceView* AtlasShaderHandle = nullptr;
#endif

	u32 NumChars;
	u32 NumKernVals;
	char_entry* Characters;
	kern_entry* KernValues;

	// Register atlas tex with rendering sdk by calling assetLoader::Register(your_sdk)Texture
	void UnloadAsset() override;

	inline char_entry FindCharEntryByAscii(u32 AsciiVal)
	{
		for (u32 i = 0; i < NumChars; i++)
		{
			if (Characters[i].AsciiValue == AsciiVal)
				return Characters[i];
		}
		return char_entry();
	}
};

struct cMeshAsset : public cAsset
{
	u32 VertexCount;
};