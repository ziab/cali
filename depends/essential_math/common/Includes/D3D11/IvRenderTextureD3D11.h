#pragma once
#include "IvRenderTexture.h"
#include "IvRenderer.h"
#include "D3D11\IvTextureD3D11.h"

#pragma warning( push )
#pragma warning( disable : 4250 ) // C4250 - 'class1' : inherits 'class2::member' via dominance

class IvRenderTextureD3D11 : public virtual IvTextureD3D11, public IvRenderTexture
{
	struct ID3D11RenderTargetView* mRenderTargetView;

public:
	virtual void Destroy() override final;

	// IvRenderTexture
	virtual bool Resize(size_t width, size_t height, IvTextureFormat format, IvResourceManager& resman) override final;
	virtual bool Resize(size_t width, size_t height, size_t depth, IvTextureFormat format, IvResourceManager & resman) override final;
	virtual void Set3DSlice(size_t sliceNum) override final;

	ID3D11RenderTargetView* GetTargetView() { return mRenderTargetView; }
	ID3D11RenderTargetView** GetPtrToTargetView() { return &mRenderTargetView; }
	ID3D11Texture2D* GetTexture() { return static_cast<ID3D11Texture2D*>(mTexturePtr); }

	IvRenderTextureD3D11() :
		IvTextureD3D11(),
		IvRenderTexture(),
		mRenderTargetView(nullptr)
	{
	}

	virtual ~IvRenderTextureD3D11() { Destroy(); };
};

#pragma warning( pop ) // C4250