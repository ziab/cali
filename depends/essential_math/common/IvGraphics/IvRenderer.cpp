//===============================================================================
// @ IvRenderer.cpp
// 
// Windowing and display setup routines
// ------------------------------------------------------------------------------
// Copyright (C) 2008-2015 by James M. Van Verth and Lars M. Bishop.
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
//===============================================================================

//-------------------------------------------------------------------------------
//-- Dependencies ---------------------------------------------------------------
//-------------------------------------------------------------------------------

#include <stdlib.h>
#include <assert.h>

#include "IvRenderer.h"
#include "IvMatrix33.h"
#include "IvMatrix44.h"

//-------------------------------------------------------------------------------
//-- Static Members -------------------------------------------------------------
//-------------------------------------------------------------------------------

IvRenderer* IvRenderer::mRenderer = 0;

//-------------------------------------------------------------------------------
//-- Methods --------------------------------------------------------------------
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// @ IvRenderer::IvRenderer()
//-------------------------------------------------------------------------------
// Default constructor
//-------------------------------------------------------------------------------
IvRenderer::IvRenderer() : 
    mIsActive( true ),
	mRenderInViewSpace(false)
{
    mWidth = 0;
    mHeight = 0;

    mFOV = 45.0f;
    mNear = 0.1f;
    mFar = 35.0f;

    mDiffuseColor.Set(1.0f, 1.0f, 1.0f, 1.0f);

    mLightAmbient.Set(1.0f, 1.0f, 1.0f, 1.0f);
    mLightDiffuse.Set(1.0f, 1.0f, 1.0f, 1.0f);
    mLightDirection = IvVector4::xAxis;

}   // End of IvRenderer::IvRenderer()


//-------------------------------------------------------------------------------
// @ IvRenderer::~IvRenderer()
//-------------------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------------------
IvRenderer::~IvRenderer()
{
    
}   // End of IvRenderer::~IvRenderer()

//-------------------------------------------------------------------------------
// @ IvRenderer::Destroy()
//-------------------------------------------------------------------------------
// Shutdown function
//-------------------------------------------------------------------------------
void 
IvRenderer::Destroy()
{
    delete mRenderer;
    mRenderer = 0;
}   // End of IvRenderer::Destroy()


//-------------------------------------------------------------------------------
// @ IvRenderer::Initialize()
//-------------------------------------------------------------------------------
// Initialize display
//-------------------------------------------------------------------------------
bool
IvRenderer::Initialize( unsigned int width, unsigned int height )
{
    return true;
}   // End of IvRenderer::Initialize()
    
//-------------------------------------------------------------------------------
// @ IvRenderer::GetResourceManager()
//-------------------------------------------------------------------------------
// Returns the resource manager
//-------------------------------------------------------------------------------
const IvResourceManager* IvRenderer::GetResourceManager() const
{
    return mResourceManager;
}

//-------------------------------------------------------------------------------
// @ IvRenderer::GetResourceManager()
//-------------------------------------------------------------------------------
// Returns the resource manager
//-------------------------------------------------------------------------------
IvResourceManager* IvRenderer::GetResourceManager()
{
    return mResourceManager;
}

void IvRenderer::SetRenderInViewSpace(bool val)
{
	mRenderInViewSpace = val;
}

const IvVector3& IvRenderer::GetViewPosition()
{
	return mViewPosition;
}

//-------------------------------------------------------------------------------
// @ IvRenderer::GetWorldMatrix()
//-------------------------------------------------------------------------------
// Gets the world matrix for the renderer
//-------------------------------------------------------------------------------
const IvMatrix44& IvRenderer::GetWorldMatrix()
{
    return mWorldMat;
}


inline void IvRenderer::RecalculateMVPMat()
{
	mWVPMat = mProjectionMat*mViewMat*mWorldMat;
}

//-------------------------------------------------------------------------------
// @ IvRenderer::SetWorldMatrix()
//-------------------------------------------------------------------------------
// Sets the world matrix for the renderer
//-------------------------------------------------------------------------------
void IvRenderer::SetWorldMatrix(const IvMatrix44& matrix)
{
	mWorldMat = matrix;
	
	if (mRenderInViewSpace)
	{
		// if we render in camera space then the models position should be calculated
		// against camera position in world's space
		mWorldMat(0, 3) = mWorldMat(0, 3) - mViewPosition.x;
		mWorldMat(1, 3) = mWorldMat(1, 3) - mViewPosition.y;
		mWorldMat(2, 3) = mWorldMat(2, 3) - mViewPosition.z;
	}

	RecalculateMVPMat();

    IvMatrix33 worldMat3x3;
    IvVector3 col0(mWorldMat(0,0), mWorldMat(1,0), mWorldMat(2,0));
    IvVector3 col1(mWorldMat(0,1), mWorldMat(1,1), mWorldMat(2,1));
    IvVector3 col2(mWorldMat(0,2), mWorldMat(1,2), mWorldMat(2,2));
    worldMat3x3.SetColumns(col0, col1, col2);
    mNormalMat.Rotation(Transpose(Inverse(worldMat3x3)));
}

//-------------------------------------------------------------------------------
// @ IvRenderer::GetViewMatrix()
//-------------------------------------------------------------------------------
// Gets the camera matrix for the renderer
//-------------------------------------------------------------------------------
const IvMatrix44& IvRenderer::GetViewMatrix()
{
    return mViewMat;
}

//-------------------------------------------------------------------------------
// @ IvRenderer::SetViewMatrix()
//-------------------------------------------------------------------------------
// Sets the camera matrix for the renderer
//-------------------------------------------------------------------------------
void IvRenderer::SetViewMatrix(const IvMatrix44& matrix)
{
    mViewMat = matrix;
	
	if (mRenderInViewSpace)
	{
		mViewMat(0, 3) = 0.0f;
		mViewMat(1, 3) = 0.0f;
		mViewMat(2, 3) = 0.0f;
	}

	RecalculateMVPMat();
}

void IvRenderer::SetViewMatrixAndViewPosition(const IvMatrix44 & matrix, const IvVector3 & viewPosition)
{
	mViewMat = matrix;

	if (mRenderInViewSpace)
	{
		mViewMat(0, 3) = 0.0f;
		mViewMat(1, 3) = 0.0f;
		mViewMat(2, 3) = 0.0f;
	}

	RecalculateMVPMat();

	mViewPosition = viewPosition;
}

//-------------------------------------------------------------------------------
// @ IvRenderer::GetProjectionMatrix()
//-------------------------------------------------------------------------------
// Gets the projection matrix for the renderer
//-------------------------------------------------------------------------------
const IvMatrix44& IvRenderer::GetProjectionMatrix()
{
    return mProjectionMat;
}

const IvMatrix44& IvRenderer::GetWVPMatrix()
{
	return mWVPMat;
}

//-------------------------------------------------------------------------------
// @ IvRenderer::SetProjectionMatrix()
//-------------------------------------------------------------------------------
// Sets the projection matrix for the renderer
//-------------------------------------------------------------------------------
void IvRenderer::SetProjectionMatrix(const IvMatrix44& matrix)
{
    mProjectionMat = matrix;
	RecalculateMVPMat();
}

//-------------------------------------------------------------------------------
// @ IvRenderer::SetMaterialDiffuse()
//-------------------------------------------------------------------------------
// Set material diffuse color (can be overridden by shader)
//-------------------------------------------------------------------------------
void
IvRenderer::SetDefaultDiffuseColor(float red, float green, float blue, float alpha)
{
    mDiffuseColor.Set(red, green, blue, alpha);
}

//-------------------------------------------------------------------------------
// @ IvRenderer::SetDefaultLightAmbient()
//-------------------------------------------------------------------------------
// Sets ambient color value for base light
//-------------------------------------------------------------------------------
void
IvRenderer::SetDefaultLightAmbient(float red, float green, float blue, float alpha)
{
    mLightAmbient.Set(red, green, blue, alpha);
}

//-------------------------------------------------------------------------------
// @ IvRenderer::SetDefaultLightDiffuse()
//-------------------------------------------------------------------------------
// Sets diffuse color value for base light
//-------------------------------------------------------------------------------
void
IvRenderer::SetDefaultLightDiffuse(float red, float green, float blue, float alpha)
{
    mLightDiffuse.Set(red, green, blue, alpha);
}

//-------------------------------------------------------------------------------
// @ IvRenderer::SetDefaultLightDirection()
//-------------------------------------------------------------------------------
// Sets direction for base light
//-------------------------------------------------------------------------------
void
IvRenderer::SetDefaultLightDirection(const IvVector3& direction)
{
    mLightDirection.Set(direction.x, direction.y, direction.z, 0.0);
}
