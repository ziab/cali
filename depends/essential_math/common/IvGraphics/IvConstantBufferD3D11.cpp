#include "IvConstantBufferD3D11.h"

char * IvConstantBufferD3D11::Get()
{
	mDirty = true;
	return mSysBuffer;
}

bool IvConstantBufferD3D11::Update(ID3D11DeviceContext * context)
{
	if (!mDirty) return true;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = context->Map(mD3D11Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Copy the data
	memcpy(mappedResource.pData, mSysBuffer, mSysBufferSize);

	// Unlock the constant buffer.
	context->Unmap(mD3D11Buffer, 0);

	mDirty = false;
	return true;
}

void IvConstantBufferD3D11::Set(ID3D11DeviceContext * context, UINT index) const
{
	context->VSSetConstantBuffers(index, 1, &mD3D11Buffer);
	context->PSSetConstantBuffers(index, 1, &mD3D11Buffer);
}

IvConstantBufferD3D11::IvConstantBufferD3D11() :
	mD3D11Buffer(nullptr),
	mSysBuffer(nullptr),
	mSysBufferSize(0),
	mDirty(false)
{
}

IvConstantBufferD3D11::~IvConstantBufferD3D11()
{
	Free();
}

bool IvConstantBufferD3D11::Create(ID3D11Device * device, size_t size)
{
	// extend to multiple of 16
	if (size % 16 != 0)
	{
		size = (size / 16 + 1) * 16;
	}

	mSysBufferSize = static_cast<UINT>(size);

	// ensure that the buffer is 16-byte aligned
	mSysBuffer = (char*)_aligned_malloc(mSysBufferSize, 16);

	ZeroMemory(mSysBuffer, mSysBufferSize);

	// Fill in a buffer description.
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = mSysBufferSize;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = mSysBuffer;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	// Create the buffer.
	auto hr = device->CreateBuffer(&cbDesc, &InitData, &mD3D11Buffer);

	return !FAILED(hr);
}

void IvConstantBufferD3D11::Free()
{
	mD3D11Buffer->Release();
	mD3D11Buffer = nullptr;

	_aligned_free(mSysBuffer);
	mSysBuffer = nullptr;

	mSysBufferSize = 0;
}
