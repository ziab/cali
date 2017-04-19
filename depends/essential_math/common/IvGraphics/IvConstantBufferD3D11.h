#pragma once
//===============================================================================
// @ IvResourceManager.h
// 
// Base class for managing creation and deletion of buffers, shaders and textures
// ------------------------------------------------------------------------------
// Copyright (C) 2008-2015  James M. Van Verth and Lars M. Bishop.
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//===============================================================================

#ifndef __IvConstantBufferD3D11__h__
#define __IvConstantBufferD3D11__h__

//-------------------------------------------------------------------------------
//-- Dependencies ---------------------------------------------------------------
//-------------------------------------------------------------------------------

#include <d3d11.h>
#include "IvConstantBuffer.h"

//-------------------------------------------------------------------------------
//-- Typedefs, Structs ----------------------------------------------------------
//-------------------------------------------------------------------------------

class IvResourceManagerD3D11;

//-------------------------------------------------------------------------------
//-- Classes --------------------------------------------------------------------

class IvConstantBufferD3D11 : public IvConstantBuffer
{
public:
	virtual char* Get() final;
	bool Update(ID3D11DeviceContext* context);
	void Set(ID3D11DeviceContext* context, UINT index) const;

	IvConstantBufferD3D11();
	virtual ~IvConstantBufferD3D11() final;

protected:
	friend class IvResourceManagerD3D11;

	bool Create(ID3D11Device* device, size_t size);
	void Free();

private:
	// copy operations
	IvConstantBufferD3D11(const IvConstantBufferD3D11& other);
	IvConstantBufferD3D11& operator=(const IvConstantBufferD3D11& other);

	ID3D11Buffer* mD3D11Buffer;
	char* mSysBuffer;
	UINT mSysBufferSize;
	bool mDirty;
};

#endif