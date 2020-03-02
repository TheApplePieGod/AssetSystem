#include "pch.h"
#include "../assetLoader.h"
#include "../assets/assetTypes.h"

using namespace assetTypes;

#ifdef ASSET_DIRECTX11
// assumes asset type 1 is image and 2 is font
void assetLoader::RegisterDXTexture(cAsset* Asset, bool GenerateMIPs, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext)
{
	Assert(Asset->Data != nullptr && Asset->Loaded == true);

	D3D11_SUBRESOURCE_DATA TextureData = {};
	D3D11_TEXTURE2D_DESC desc;

	switch (Asset->Type)
	{
		default:
		{ Assert(1 == 2); } break;

		case 1: // image
		{
			cTextureAsset* Tex = (cTextureAsset*)Asset;
			TextureData.SysMemPitch = Tex->ImageData.Channels * Tex->ImageData.Width;
			desc.Width = Tex->ImageData.Width;
			desc.Height = Tex->ImageData.Height;
			switch (Tex->ImageData.Channels)
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

		case 2: // font
		{
			cFontAsset* Font = (cFontAsset*)Asset;
			TextureData.SysMemPitch = Font->FontData.AtlasDim * 1; // assumes one channel
			desc.Width = Font->FontData.AtlasDim;
			desc.Height = Font->FontData.AtlasDim;
			desc.Format = DXGI_FORMAT_A8_UNORM; // should always be 1
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

		case 1: // image
		{
			cTextureAsset* Tex = (cTextureAsset*)Asset;
			Tex->ShaderHandle = shader;
			Tex->TextureHandle = pTexture;
		} break;

		case 2: // font
		{
			cFontAsset* Font = (cFontAsset*)Asset;
			Font->AtlasShaderHandle = shader;
		} break;
	}
}
#endif