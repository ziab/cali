#pragma once
#include "IvTexture.h"

class IvRenderTexture : virtual public IvTexture
{
public:
	virtual bool Resize(size_t width, size_t height, IvTextureFormat format, class IvResourceManager& renderer) = 0;
	virtual bool Resize(size_t width, size_t height, size_t depth, IvTextureFormat format, IvResourceManager & resman) = 0;
	virtual void Set3DSlice(size_t sliceNum) = 0;

	IvRenderTexture() {};
	virtual ~IvRenderTexture() {};
};
