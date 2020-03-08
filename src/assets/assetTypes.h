#pragma once

namespace assetTypes
{
	// image
	struct image_data
	{
		s32 Width;
		s32 Height;
		s32 Channels;
	};

	bool Image_GetDataForWriting(char*& Out_ExtraData, char*& Out_RawData, u32& Out_ExtraDataSize, u32& Out_RawDataSize, char* FilePath);
	cAsset* Image_InitializeData(cAsset* AssetDefaults, char* ExtraData, u32 ExtraDataSize);

	static asset_type ImageType = { 1, "Image", {"png", "jpg", "tga"}, Image_GetDataForWriting, Image_InitializeData }; // Set callback separately


	// font
	struct font_data // assumes one channel texture
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

	union v2
	{
		struct
		{
			f32 x, y;
		};
		struct
		{
			f32 u, v;
		};
		f32 E[2];
	};

	inline v2
	operator-(v2 A, v2 B)
	{
		v2 Result;

		Result.x = A.x - B.x;
		Result.y = A.y - B.y;

		return(Result);
	}

	union v3
	{
		struct
		{
			f32 x, y, z;
		};
		struct
		{
			f32 u, v, w;
		};
		struct
		{
			f32 r, g, b;
		};
		f32 E[3];
	};

	inline v3
	operator-(v3 A, v3 B)
	{
		v3 Result;

		Result.x = A.x - B.x;
		Result.y = A.y - B.y;
		Result.z = A.z - B.z;

		return(Result);
	}

	inline v3
	operator*(v3 A, f32 B)
	{
		v3 Result;

		Result.x = A.x * B;
		Result.y = A.x * B;
		Result.z = A.x * B;

		return(Result);
	}

	bool Font_GetDataForWriting(char*& Out_ExtraData, char*& Out_RawData, u32& Out_ExtraDataSize, u32& Out_RawDataSize, char* FilePath);
	cAsset* Font_InitializeData(cAsset* AssetDefaults, char* ExtraData, u32 ExtraDataSize);

	static asset_type FontType = { 2, "Font", {"ttf"}, Font_GetDataForWriting, Font_InitializeData }; // Set callback separately

	// mesh
	// Structure used when storing vertices from imported mesh
	struct vertex
	{
		// Position in x/y plane
		v3 position;

		// UV coordinates
		v2 uv;

		// Normals
		v3 normal;

		// Tangents
		v3 tangent;

		// Bitangent
		v3 bitangent;
	};

	struct mesh_data
	{
		u32 NumVertices;
		u32 NumIndices;
	};

	bool Mesh_GetDataForWriting(char*& Out_ExtraData, char*& Out_RawData, u32& Out_ExtraDataSize, u32& Out_RawDataSize, char* FilePath);
	cAsset* Mesh_InitializeData(cAsset* AssetDefaults, char* ExtraData, u32 ExtraDataSize);

	static asset_type MeshType = { 3, "Mesh", {"fbx"}, Mesh_GetDataForWriting, Mesh_InitializeData }; // Set callback separately
};

struct cTextureAsset : public cAsset // todo: change to cImageAsset
{
	assetTypes::image_data ImageData;

#ifdef ASSET_DIRECTX11
	ID3D11Texture2D* TextureHandle = nullptr;
	ID3D11ShaderResourceView* ShaderHandle = nullptr;
#endif

	// Register texture with rendering sdk by calling assetLoader::Register(your_sdk)Texture
	void UnloadAsset() override;
};

struct cFontAsset : public cAsset
{
	assetTypes::font_data FontData;

#ifdef ASSET_DIRECTX11
	ID3D11ShaderResourceView* AtlasShaderHandle = nullptr;
#endif

	u32 NumChars;
	u32 NumKernVals;
	assetTypes::char_entry* Characters;
	assetTypes::kern_entry* KernValues;

	// Register atlas tex with rendering sdk by calling assetLoader::Register(your_sdk)Texture
	void UnloadAsset() override;

	inline assetTypes::char_entry FindCharEntryByAscii(u32 AsciiVal)
	{
		for (u32 i = 0; i < NumChars; i++)
		{
			if (Characters[i].AsciiValue == AsciiVal)
				return Characters[i];
		}
		return assetTypes::char_entry();
	}
};

struct cMeshAsset : public cAsset
{
	assetTypes::mesh_data MeshData;
};