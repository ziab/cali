#include "IvRenderTextureD3D11.h"
#include "D3D11\IvResourceManagerD3D11.h"
#include "D3D11\IvRendererD3D11.h"

#include <d3d11.h>

void IvRenderTextureD3D11::Destroy()
{
	IvTextureD3D11::Destroy();

	if (mRenderTargetView)
	{
		mRenderTargetView->Release();
		mRenderTargetView = nullptr;
	}
}

bool IvRenderTextureD3D11::Resize(size_t width, size_t height, IvTextureFormat format, IvResourceManager& resman)
{
	Destroy();

	SetAddressingU(kClampTexAddr);
	SetAddressingV(kClampTexAddr);
	SetMagFiltering(kBilerpTexMagFilter);
	SetMinFiltering(kBilerpTexMinFilter);

	IvTexture::mWidth = (unsigned int) width;
	IvTexture::mHeight = (unsigned int) height;
	IvTexture::mFormat = format;
	mUsage = kDefaultUsage;

	unsigned int texelSize = sInternalTextureFormatSize[mFormat];

	mLevelCount = 1;

	// allocate level for BeginLoadData/EndLoadData

	mLevels = new Level[mLevelCount];

	mLevels[0].mData = nullptr;
	mLevels[0].mWidth = mWidth;
	mLevels[0].mHeight = mHeight;
	mLevels[0].mSize = mWidth*mHeight*texelSize;

	auto& resmand3d11 = static_cast<IvResourceManagerD3D11&>(resman);
	auto device = resmand3d11.GetDevice();

	D3D11_TEXTURE2D_DESC textureDesc;
	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	// Setup the render target texture description.
	textureDesc.Width = (UINT) width;
	textureDesc.Height = (UINT) height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = D3DTextureFormatMapping[format];
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create the render target texture.
	HRESULT result = device->CreateTexture2D(&textureDesc, NULL, (ID3D11Texture2D**)&mTexturePtr);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the description of the render target view.
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	result = device->CreateRenderTargetView(mTexturePtr, &renderTargetViewDesc, &mRenderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the description of the shader resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	result = device->CreateShaderResourceView(mTexturePtr, &shaderResourceViewDesc, &mShaderResourceView);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool IvRenderTextureD3D11::Resize(size_t width, size_t height, size_t depth, IvTextureFormat format, IvResourceManager& resman)
{
	Destroy();

	SetAddressingU(kClampTexAddr);
	SetAddressingV(kClampTexAddr);
	SetMagFiltering(kBilerpTexMagFilter);
	SetMinFiltering(kBilerpTexMinFilter);

	IvTexture::mWidth = (unsigned int)width;
	IvTexture::mHeight = (unsigned int)height;
	IvTexture::mDepth = (unsigned int)depth;
	IvTexture::mFormat = format;
	mUsage = kDefaultUsage;

	unsigned int texelSize = sInternalTextureFormatSize[mFormat];

	mLevelCount = 1;

	// allocate level for BeginLoadData/EndLoadData

	mLevels = new Level[mLevelCount];

	mLevels[0].mData = nullptr;
	mLevels[0].mWidth = mWidth;
	mLevels[0].mHeight = mHeight;
	mLevels[0].mSize = mWidth * mHeight * mDepth * texelSize;

	auto& resmand3d11 = static_cast<IvResourceManagerD3D11&>(resman);
	auto device = resmand3d11.GetDevice();

	D3D11_TEXTURE3D_DESC textureDesc;
	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	// Setup the render target texture description.
	textureDesc.Width = (UINT)width;
	textureDesc.Height = (UINT)height;
	textureDesc.Depth = (UINT)depth;
	textureDesc.MipLevels = 0;
	textureDesc.Format = D3DTextureFormatMapping[format];
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create the render target texture.
	HRESULT result = device->CreateTexture3D(&textureDesc, NULL, (ID3D11Texture3D**)&mTexturePtr);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the description of the render target view.
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
	renderTargetViewDesc.Texture3D.MipSlice = 0;
	renderTargetViewDesc.Texture3D.FirstWSlice = 0;
	renderTargetViewDesc.Texture3D.WSize = (UINT)(-1);

	// Create the render target view.
	result = device->CreateRenderTargetView(mTexturePtr, &renderTargetViewDesc, &mRenderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the description of the shader resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	shaderResourceViewDesc.Texture3D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture3D.MipLevels = 1;

	// Create the shader resource view.
	result = device->CreateShaderResourceView(mTexturePtr, &shaderResourceViewDesc, &mShaderResourceView);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void IvRenderTextureD3D11::Set3DSlice(size_t sliceNum)
{
	// Check if texture is actuall 3d
	if (mWidth == 0) return;

	mRenderTargetView->Release();
	mRenderTargetView = nullptr;

	// Setup the description of the render target view.
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = D3DTextureFormatMapping[mFormat];
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
	renderTargetViewDesc.Texture3D.MipSlice = 0;
	renderTargetViewDesc.Texture3D.FirstWSlice = (UINT)sliceNum;
	renderTargetViewDesc.Texture3D.WSize = 1;

	auto& renderer = *IvRenderer::mRenderer;
	auto& resman = *renderer.GetResourceManager();
	auto& d3d11renderer = static_cast<IvRendererD3D11&>(renderer);
	auto* device = d3d11renderer.GetDevice();

	// Create the render target view.
	HRESULT result = device->CreateRenderTargetView(mTexturePtr, &renderTargetViewDesc, &mRenderTargetView);
	if (FAILED(result))
	{
		// do nothing
	}
}
