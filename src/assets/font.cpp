#include "pch.h"
#include "../assetLoader.h"
#include "../assetLoader_internal.h"

#include "../lib/stb_image.h"
#include "../lib/stb_image_write.h"
#include "../lib/stb_truetype.h"

///*
//* Returns new file path
//*/
//std::string assetLoader::PackFont(std::string Path, char* FileBuffer, std::string Filename, int AssetID, std::ofstream* ofs, bool GeneratePac)
//{
//	std::vector<char_entry> Characters;
//	std::vector<kern_entry> KernValues;
//	font_pack FontPackData;
//	s32 Width, Height, Ascent, Descent, LineGap, AdvanceWidth, LeftSideBearing;
//	stbtt_fontinfo FontInfo;
//
//	stbtt_InitFont(&FontInfo, (unsigned char*)FileBuffer, stbtt_GetFontOffsetForIndex((unsigned char*)FileBuffer, 0));
//	stbtt_GetFontVMetrics(&FontInfo, &Ascent, &Descent, &LineGap);
//
//	f32 PH = (f32)GetAssetSettings().FontSizePixels;
//	f32 ScaleMultiplier = stbtt_ScaleForPixelHeight(&FontInfo, PH);
//	Ascent *= (s32)ScaleMultiplier;
//	Descent *= (s32)ScaleMultiplier;
//	LineGap *= (s32)ScaleMultiplier;
//
//	s32 TextureDim = 512 * (s32)(PH / 40);
//	s32 AtlasDataSize = TextureDim * TextureDim * 1;
//	u8* AtlasBuffer = (u8*)malloc(AtlasDataSize);
//	memset(AtlasBuffer, 0, AtlasDataSize);
//
//	s32 CharCounter = 1;
//	s32 BoxHeight = GetAssetSettings().FontSizePixels;
//	s32 BoxWidth = BoxHeight;
//	s32 CurrentRow = 1;
//	s32 CharsPerRow = TextureDim / BoxWidth;
//
//	FontPackData.Ascent = Ascent;
//	FontPackData.Descent = Descent;
//	FontPackData.LineGap = LineGap;
//	FontPackData.ScaleMultiplier = ScaleMultiplier;
//	FontPackData.CharsPerRow = CharsPerRow;
//	FontPackData.BoxHeight = BoxHeight;
//	FontPackData.BoxWidth = BoxWidth;
//	FontPackData.AtlasDim = TextureDim;
//	FontPackData.AtlasDataSize = AtlasDataSize;
//
//	s32 CharIndex = -1;
//	for (s32 AsciiVal = 32; AsciiVal <= 126; AsciiVal++) // ascii values 32-126
//	{
//		CharIndex++;
//		Characters.push_back(char_entry());
//		s32 xOffset = 0;
//		s32 yOffset = 0;
//
//		u8* CharacterData = stbtt_GetCodepointBitmap(&FontInfo, 0.0f, ScaleMultiplier, AsciiVal, &Width, &Height, &xOffset, &yOffset);
//		stbtt_GetCodepointHMetrics(&FontInfo, AsciiVal, &AdvanceWidth, &LeftSideBearing);
//
//		Characters[CharIndex].AsciiValue = AsciiVal;
//		Characters[CharIndex].Width = Width;
//		Characters[CharIndex].Height = Height;
//		Characters[CharIndex].OffsetX = (xOffset * ScaleMultiplier);
//		Characters[CharIndex].OffsetY = (f32)yOffset;
//		Characters[CharIndex].AdvanceWidth = (AdvanceWidth * ScaleMultiplier);
//		Characters[CharIndex].LeftSideBearing = (LeftSideBearing * ScaleMultiplier);
//		Characters[CharIndex].GlyphDataLength = Width * Height;
//
//		s32 TLX = CharCounter * BoxWidth;
//		s32 TLY = TextureDim * (BoxHeight * CurrentRow);
//
//		Characters[CharIndex].TopLeftOffsetX = TLX;
//		Characters[CharIndex].TopLeftOffsetY = TLY / TextureDim;
//
//		for (s32 y = 0; y < Height; y++)
//		{
//			s32 Position = TLY + (TextureDim * y) + TLX;
//			for (s32 x = 0; x < Width; x++)
//			{
//				AtlasBuffer[Position++] = *CharacterData++;
//			}
//		}
//
//		CharCounter++;
//		if (CharCounter % CharsPerRow == 0)
//		{
//			CurrentRow++;
//			CharCounter = 1;
//		}
//
//		for (s32 KernVal = 32; KernVal <= 126; KernVal++)
//		{
//			float Spacing = (f32)stbtt_GetCodepointKernAdvance(&FontInfo, AsciiVal, KernVal) * ScaleMultiplier;
//
//			if (Spacing != 0)
//			{
//				kern_entry entry;
//
//				entry.AsciiVal1 = AsciiVal;
//				entry.AsciiVal2 = KernVal;
//				entry.Spacing = Spacing;
//
//				KernValues.push_back(entry);
//			}
//		}
//	}
//
//	FontPackData.NumChars = (u32)Characters.size();
//
//	asset_header Header;
//	Header.ID = AssetID;
//	Header.Type = asset_type::Font;
//	Header.DataSize = FontPackData.AtlasDataSize;
//	Header.ExtraSize = (u32)((sizeof(char_entry) * Characters.size()) + (sizeof(kern_entry) * KernValues.size()));
//	Header.NextItem = sizeof(FontPackData) + Header.ExtraSize + FontPackData.AtlasDataSize;
//	strcpy(Header.Filename, Filename.c_str());
//
//	if (ofs != nullptr)
//	{
//		if (GeneratePac)
//		{
//			ofs->write((char*)&Header, sizeof(asset_header));
//			ofs->write((char*)&FontPackData, sizeof(FontPackData));
//			ofs->write((char*)Characters.data(), sizeof(char_entry) * Characters.size());
//			ofs->write((char*)KernValues.data(), sizeof(kern_entry) * KernValues.size());
//			ofs->write((char*)AtlasBuffer, FontPackData.AtlasDataSize);
//		}
//	}
//
//	remove(Path.c_str());
//	std::string NewPath = Path;
//	NewPath.erase(NewPath.length() - 4, 4);
//	NewPath.push_back('.');
//	NewPath += GetAssetSettings().AssetFileExtension;
//	FILE* file;
//	file = fopen(NewPath.c_str(), "wb");
//	if (file)
//	{
//		fwrite((char*)&Header, sizeof(asset_header), 1, file);
//		fwrite((char*)&FontPackData, sizeof(FontPackData), 1, file);
//		fwrite((char*)Characters.data(), sizeof(char_entry), Characters.size(), file);
//		fwrite((char*)KernValues.data(), sizeof(kern_entry), KernValues.size(), file);
//		fwrite((char*)AtlasBuffer, sizeof(char), FontPackData.AtlasDataSize, file);
//		fclose(file);
//	}
//
//	free(AtlasBuffer);
//	return NewPath;
//}
//
//const char* assetLoader::PackFont(const char* Path, int AssetID)
//{
//	WIN32_FIND_DATA FileData;
//	HANDLE hFind = FindFirstFile(Path, &FileData);
//
//	// load font file
//	u32 filesize = (FileData.nFileSizeHigh * ((long)MAXDWORD + 1)) + FileData.nFileSizeLow;
//	std::ifstream ifs(Path, std::ifstream::binary);
//	char* FileBuffer = new char[filesize];
//	ifs.rdbuf()->sgetn(FileBuffer, filesize);
//	ifs.close();
//
//	std::string ret = PackFont(Path, FileBuffer, FileData.cFileName, AssetID);
//	char* retptr = new char[ret.size() + 1];
//	strcpy(retptr, ret.c_str());
//
//	delete[] FileBuffer;
//	return retptr;
//}
//
//void assetLoader::LoadFont(FILE* File, asset_header& Header, std::string Path, void (*Callback)(cFontAsset*))
//{
//	cFontAsset* FontAsset = new cFontAsset;
//	fread((char*)&FontAsset->FontData, sizeof(font_pack), 1, File);
//	char_entry* CharArray = new char_entry[FontAsset->FontData.NumChars * sizeof(char_entry)];
//	kern_entry* KernArray = new kern_entry[Header.ExtraSize - (FontAsset->FontData.NumChars * sizeof(char_entry))];
//	fread((char*)CharArray, sizeof(char_entry), FontAsset->FontData.NumChars, File);
//	fread((char*)KernArray, 1, Header.ExtraSize - (FontAsset->FontData.NumChars * sizeof(char_entry)), File);
//	FontAsset->Characters = CharArray;
//	FontAsset->KernValues = KernArray;
//	FontAsset->AssetID = Header.ID;
//	FontAsset->DataSize = Header.DataSize;
//	FontAsset->Type = asset_type::Font; // font
//	strncpy(FontAsset->Filename, Header.Filename, MAX_PATH);
//	strncpy(FontAsset->Path, Path.c_str(), MAX_PATH);
//	FontAsset->LoadAssetData();
//
//	(*Callback)(FontAsset);
//}
//
//void assetLoader::LoadFont(const char* Path, void (*Callback)(cFontAsset*))
//{
//	FILE* file = fopen(Path, "rb");
//	if (file)
//	{
//		asset_header Header;
//		fread((char*)&Header, 1, sizeof(asset_header), file);
//		LoadFont(file, Header, Path, Callback);
//
//		fclose(file);
//	}
//}